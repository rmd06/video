#include "ffmpeg_video_server.h"

// This code is written based on the example code from
// avcodec_sample.0.5.0.c and based on ffplay.c

ffmpeg_video_server::ffmpeg_video_server(const char *filename)
  : m_pFormatCtx(NULL)
  , d_mode(SINGLE)
{
    _status = false;

    // Initialize libavcodec and have it load all codecs.
    printf("dbg: Registering ffmped stuff\n");
    avcodec_register_all();
    avdevice_register_all();
    avfilter_register_all();
    av_register_all();

    // Allocate the context to use
    printf("dbg: Allocating context\n");
    m_pFormatCtx = avformat_alloc_context();
    if (m_pFormatCtx == NULL) {
            fprintf(stderr,"ffmpeg_video_server::ffmpeg_video_server(): Cannot allocate context\n");
            return;
    }

    // Open the video file.
    printf("dbg: Opening file\n");
    AVInputFormat *iformat = NULL;
    AVDictionary *format_opts = NULL;
    if (avformat_open_input(&m_pFormatCtx, filename, iformat, &format_opts)!=0) {
            fprintf(stderr,"ffmpeg_video_server::ffmpeg_video_server(): Cannot open file %s\n", filename);
            return;
    }

    // Retrieve stream information
    if(av_find_stream_info(m_pFormatCtx)<0) {
        fprintf(stderr,"ffmpeg_video_server::ffmpeg_video_server(): Cannot find stream information\n");
        return;
    }

    // Find the first video stream
    int i;
    m_videoStream=-1;
    for(i=0; i<m_pFormatCtx->nb_streams; i++) {
        if(m_pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
            m_videoStream=i;
            break;
        }
    }
    if(m_videoStream==-1) {
        fprintf(stderr,"ffmpeg_video_server::ffmpeg_video_server(): Cannot find video stream\n");
        return;
    }

    // Get a pointer to the codec context for the video stream
    m_pCodecCtx=m_pFormatCtx->streams[m_videoStream]->codec;

    // Find the decoder for the video stream
    AVCodec         *pCodec;
    pCodec=avcodec_find_decoder(m_pCodecCtx->codec_id);
    if (pCodec==NULL) {
        fprintf(stderr,"ffmpeg_video_server::ffmpeg_video_server(): Cannot find CODEC\n");
        return;
    }

    // Open codec
    if (avcodec_open(m_pCodecCtx, pCodec)<0) {
        fprintf(stderr,"ffmpeg_video_server::ffmpeg_video_server(): Cannot open CODEC\n");
        return;
    }

    // Hack to correct wrong frame rates that seem to be generated by some codecs
    if (m_pCodecCtx->time_base.num>1000 && m_pCodecCtx->time_base.den==1) {
        m_pCodecCtx->time_base.den=1000;
    }

    // Allocate video frame
    m_pFrame=avcodec_alloc_frame();
    if (m_pFrame==NULL) {
        fprintf(stderr,"ffmpeg_video_server::ffmpeg_video_server(): Out of memory allocating video frame\n");
        return;
    }

    // Allocate an AVFrame structure
    m_pFrameRGB=avcodec_alloc_frame();
    if (m_pFrameRGB==NULL) {
        fprintf(stderr,"ffmpeg_video_server::ffmpeg_video_server(): Out of memory allocating RGB video frame\n");
        return;
    }

    // Determine required buffer size and allocate buffer
    size_t numBytes;
    numBytes=avpicture_get_size(PIX_FMT_RGB24, m_pCodecCtx->width,
        m_pCodecCtx->height);
    m_buffer=new uint8_t(numBytes);

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    avpicture_fill((AVPicture *)m_pFrameRGB, m_buffer, PIX_FMT_RGB24,
        m_pCodecCtx->width, m_pCodecCtx->height);

    _num_columns = m_pCodecCtx->width;
    _num_rows = m_pCodecCtx->height;
    _minX = _minY = 0;
    _maxX = _num_columns-1;
    _maxY = _num_rows-1;
    _binning = 1;

    _status = true;
}

ffmpeg_video_server::~ffmpeg_video_server()
{
    if (m_buffer) {
        delete [] m_buffer;
        m_buffer = NULL;
    }
    if (m_pFrameRGB) {
        av_free(m_pFrameRGB);
        m_pFrameRGB = NULL;
    }
    if (m_pFrame) {
        av_free(m_pFrame);
        m_pFrame = NULL;
    }

    avcodec_close(m_pCodecCtx);
    av_close_input_file(m_pFormatCtx);

    // Delete the format context.
    if (m_pFormatCtx) {
            avformat_free_context(m_pFormatCtx);
            m_pFormatCtx = NULL;
    }
}

bool ffmpeg_video_server::write_to_opengl_texture(GLuint tex_id) {
	// XXX *** stub
	return true;
}

bool ffmpeg_video_server::get_pixel_from_memory(unsigned int X, unsigned int Y, vrpn_uint8 &val, int RGB) const {
	val = (vrpn_uint8)0; // XXX
	return true;
}

bool ffmpeg_video_server::get_pixel_from_memory(unsigned int X, unsigned int Y, vrpn_uint16 &val, int RGB) const {
	val = (vrpn_uint16)0; // XXX
	return true;
}

bool ffmpeg_video_server::read_image_to_memory(unsigned int minX, unsigned int maxX, unsigned int minY, unsigned int maxY, double exposure_time_millisecs)
{
    vrpn_gettimeofday(&m_timestamp, NULL);

    // If we're paused, then return without an image and try not to eat the whole CPU
    if (d_mode == PAUSE) {
      vrpn_SleepMsecs(10);
      return false;
    }

    // If we're doing single-frame, then set the mode to pause for next time so that we
    // won't keep trying to read frames.
    if (d_mode == SINGLE) {
      d_mode = PAUSE;
    }

    // Read and decode a frame from the video file.  If there are no
    // complete video frames in the file, then frameFinished will be
    // 0 at the end of the while loop.
    AVPacket        packet;
    int             frameFinished = 0;
    av_init_packet(&packet);
    while(!frameFinished && (av_read_frame(m_pFormatCtx, &packet)>=0)) {
        // Is this a packet from the video stream?
        if(packet.stream_index==m_videoStream) {
            // Decode video frame
            avcodec_decode_video2(m_pCodecCtx, m_pFrame, &frameFinished,
                &packet);

            // Did we get a full video frame?
            if(frameFinished) {
                static struct SwsContext *img_convert_ctx;

                // Convert the image into RGB format that we need
                // XXX Consider initializing this conversion format in the
                // constructor to reduce work here.
                if(img_convert_ctx == NULL) {
                    int w = m_pCodecCtx->width;
                    int h = m_pCodecCtx->height;

                    img_convert_ctx = sws_getContext(w, h,
                                                    m_pCodecCtx->pix_fmt,
                                                    w, h, PIX_FMT_RGB24, SWS_BICUBIC,
                                                    NULL, NULL, NULL);
                    if(img_convert_ctx == NULL) {
                        fprintf(stderr, "ffmpeg_video_server::read_image_to_memory(): Cannot initialize the conversion context!\n");
                        return false;
                    }
                }
                int ret = sws_scale(img_convert_ctx, m_pFrame->data, m_pFrame->linesize, 0,
                                  m_pCodecCtx->height, m_pFrameRGB->data, m_pFrameRGB->linesize);
            }
        }

        // Free the packet that was allocated by av_read_frame
        av_free_packet(&packet);
    }

    // If we've gone past the end of the video, then set the mode to pause
    // and return false to say that we have no frame.  If we get here without
    // a finished frame, then we must be at the end.
    if (!frameFinished) {
        d_mode = PAUSE;
        return false;
    }

    // The image is now loaded and properly formatted in the m_pFrame, ready
    // to be copied from m_pFrame->data.
    return true;
}

bool ffmpeg_video_server::send_vrpn_image(vrpn_Imager_Server* svr, vrpn_Connection* svrcon, double g_exposure, int svrchan, int num_chans) {
	// Make sure we have a valid, open device
	if (!_status) { return false; };
/* XXX
	// Send the current frame over to the client in chunks as big as possible (limited by vrpn_IMAGER_MAX_REGION).
	int nRowsPerRegion=vrpn_IMAGER_MAX_REGIONu8/_num_columns;
	//printf("nRowsPerRegion = %i\n", nRowsPerRegion);
	//printf("(_num_columns, _num_rows) = (%i, %i)\n", _num_columns, _num_rows);
	
	svr->send_begin_frame(0, _num_columns-1, 0, _num_rows-1, 0, 0, &m_timestamp);
	unsigned y;
	for(y=0; y<_num_rows; y+=nRowsPerRegion) {
		svr->send_region_using_base_pointer(svrchan,0,_num_columns-1,y,min(_num_rows,y+nRowsPerRegion)-1,
			(vrpn_uint8*)m_cam->GetImagePointer(), 1, _num_columns, _num_rows, true, 0, 0, 0, &m_timestamp);
		svr->mainloop();
	}
	svr->send_end_frame(0, _num_columns-1, 0, _num_rows-1, 0, 0, &m_timestamp);
	svr->mainloop();
*/

	// Mainloop the server connection (once per server mainloop, not once per object).
	svrcon->mainloop();

	return true;
}

/** Begin playing the video file from the current location. */
void  ffmpeg_video_server::play(void)
{
    d_mode = PLAY;
}

/** Pause the video file at the current location. */
void  ffmpeg_video_server::pause(void)
{
    d_mode = PAUSE;
}

/** Read a single frame of video, then pause. */
void  ffmpeg_video_server::single_step(void)
{
    d_mode = SINGLE;
}

/** Rewind the videofile to the beginning and take one step. */
void  ffmpeg_video_server::rewind(void)
{
    // XXX

    // Read one frame when we start
    d_mode = SINGLE;
}

void  ffmpeg_video_server::close_device(void)
{
	// XXX
}
