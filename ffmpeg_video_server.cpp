#include "ffmpeg_video_server.h"

ffmpeg_video_server::ffmpeg_video_server(const char *filename)
  : m_pFormatCtx(NULL)
{
	_status = false;
	// XXX Initialize ffmpeg here.

	// Initialize libavcodec and have it load all codecs.
	avcodec_register_all();

	// Allocate the context to use
	m_pFormatCtx = avformat_alloc_context();
	if (m_pFormatCtx == NULL) {
		fprintf(stderr,"ffmpeg_video_server::ffmpeg_video_server(): Cannot allocate context\n");
		return;
	}

	// Open the video file.
	AVInputFormat *iformat;
        if (avformat_open_input(&m_pFormatCtx, filename, iformat, NULL)!=0) {
		fprintf(stderr,"ffmpeg_video_server::ffmpeg_video_server(): Cannot open file %s\n", filename);
		return;
	}

	printf("XXX finish implementing FFMPEG!\n");

	_num_columns = 0;	// XXX
	_num_rows = 0;		// XXX
	_minX = _minY = 0;
	_maxX = _num_columns-1;
	_maxY = _num_rows-1;
	_binning = 1;

	_status = false;	// XXX
}

ffmpeg_video_server::~ffmpeg_video_server() {
	// XXX Tear down everything

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

bool ffmpeg_video_server::read_image_to_memory(unsigned int minX, unsigned int maxX, unsigned int minY, unsigned int maxY, double exposure_time_millisecs) {
	vrpn_gettimeofday(&m_timestamp, NULL);
	// XXX
	return false;
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
	// XXX
}

/** Pause the video file at the current location. */
void  ffmpeg_video_server::pause(void)
{
	// XXX
}

/** Read a single frame of video, then pause. */
void  ffmpeg_video_server::single_step(void)
{
	// XXX
}

/** Rewind the videofile to the beginning and take one step. */
void  ffmpeg_video_server::rewind(void)
{
	// XXX
}

void  ffmpeg_video_server::close_device(void)
{
	// XXX
}