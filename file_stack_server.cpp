// XXX To be able to read 16-bit images, we need to use a version of ImageMagick
// that is compiled with -DQuantumDepth=16.  Then, we need to #define QuantumLeap
// before includeing magick/api.h.  How to make this compatible with Nano and
// other NSRG applications?

#include "file_stack_server.h"
#include "file_list.h"

#define QuantumLeap
#include <magick/api.h>

#include <list>
using namespace std;

#ifdef	_WIN32
  const char READSTRING[] = "rb";
#else
  const char READSTRING[] = "r";
#endif

// This is to ensure that we only call MagickIncarnate once.
bool file_stack_server::ds_majickInitialized = false;

file_stack_server::file_stack_server(const char *filename, const char *magickfilesdir) :
d_buffer(NULL),
d_mode(SINGLE),
d_xFileSize(0),
d_yFileSize(0)
{
  // In case we fail somewhere along the way
  _status = false;

  // If we've not called MagickIncarnate(), do so now
  if (!ds_majickInitialized) {
      ds_majickInitialized = true;
#ifdef	_WIN32
      InitializeMagick(magickfilesdir);
#endif
  }

  // Get a list of the files in the same directory as the file we
  // just opened.  If it is empty, then we've got a problem and should
  // bail.
  if (!file_list(filename, d_fileNames)) {
    return;
  }

  // Open the first file and find out how big it is.
  // This routine has some side effects: It allocates the buffer and it fills in
  // the d_xFileSize and d_yFileSize data members.
  if (!read_image_from_file(*d_fileNames.begin())) {
    fprintf(stderr,"file_stack_server::file_stack_server(): Can't read %s\n", d_fileNames.begin()->c_str());
    return;
  }

  // Everything opened okay.
  _minX = _minY = 0;
  _maxX = d_xFileSize-1;
  _maxY = d_yFileSize-1;
  _num_columns = d_xFileSize;
  _num_rows = d_yFileSize;
  _binning = 1;
  d_whichFile = d_fileNames.begin();
  _status = true;
}

file_stack_server::~file_stack_server(void)
{
  // Free the space taken by the in-memory image (if allocated)
  if (d_buffer) {
    delete [] d_buffer;
  }
}

void  file_stack_server::play()
{
  d_mode = PLAY;
}

void  file_stack_server::pause()
{
  d_mode = PAUSE;
}

void  file_stack_server::rewind()
{
  // Seek back to the first file
  d_whichFile = d_fileNames.begin();

  // Read one frame when we start
  d_mode = SINGLE;
}

void  file_stack_server::single_step()
{
  d_mode = SINGLE;
}

  // This routine has some side effects: It allocates the buffer and it fills in
  // the d_xFileSize and d_yFileSize data members.
bool  file_stack_server::read_image_from_file(const string filename)
{
  ExceptionInfo	  exception;
  Image		  *image;
  ImageInfo       *image_info;
  PixelPacket	  *pixels;
  ViewInfo	  *vinfo;

  //Initialize the image info structure and read an image.
  GetExceptionInfo(&exception);
  image_info=CloneImageInfo((ImageInfo *) NULL);
  (void) strcpy(image_info->filename,filename.c_str());
  image=ReadImage(image_info,&exception);
  if (image == (Image *) NULL) {
      // print out something to let us know we are missing the 
      // delegates.mgk or whatever if that is the problem instead of just
      // saying the file can't be loaded later
      fprintf(stderr, "nmb_ImgMagic: %s: %s\n",
             exception.reason,exception.description);
      //MagickError(exception.severity,exception.reason,exception.description);
      // Get here if we can't decipher the file, let caller handle it. 
      return false;
  }

  // Ensure that we don't get images of different sizes.  If this image is the
  // wrong size, then return false.  If we've never read any images, then the
  // sizes will be 0.
  if (d_xFileSize == 0) {
    d_xFileSize = image->columns;
    d_yFileSize = image->rows;
  } else if ( (d_xFileSize != image->columns) || (d_yFileSize != image->rows) ) {
    fprintf(stderr,"file_stack_server::read_image_from_file(): Image size differs\n");
    return false;
  }

  // Allocate space to read a frame from the file, if it has not yet been
  // allocated
  if (d_buffer == NULL) {
    if ( (d_buffer = new vrpn_uint16[d_xFileSize * d_yFileSize * 3]) == NULL) {
      fprintf(stderr,"file_stack_server::read_image_from_file: Out of memory\n");
      _status = false;
      d_mode = PAUSE;
      return false;
    }
  }

  // This is the method for reading pixels that compiles and works, 
  // as opposed to GetImagePixels or GetOnePixel, which wouldn't compile. 
  vinfo = OpenCacheView(image);
  pixels = GetCacheView(vinfo, 0,0,image->columns,image->rows);
  if(!pixels) {
      fprintf(stderr, "file_stack_server::read_image_from_file: unable to get pixel cache.\n"); 
      return false;
  }

  // Copy the pixels from the image into the buffer. Flip the image over in Y
  // to match the orientation we expect.
  unsigned x, y, flip_y;
  for (y = 0; y < d_yFileSize; y++) {
    flip_y = (d_yFileSize - 1) - y;
    for (x = 0; x < d_xFileSize; x++) {
      d_buffer[ (x + flip_y * d_xFileSize) * 3 + 0 ] = pixels[x + image->columns*y].red;
      d_buffer[ (x + flip_y * d_xFileSize) * 3 + 1 ] = pixels[x + image->columns*y].green;
      d_buffer[ (x + flip_y * d_xFileSize) * 3 + 2 ] = pixels[x + image->columns*y].blue;
    }
  }

  CloseCacheView(vinfo);
  DestroyImageInfo(image_info);
  DestroyImage(image);
  return true;
}

bool  file_stack_server::read_image_to_memory(unsigned minX, unsigned maxX,
							unsigned minY, unsigned maxY,
							double exposure_time_millisecs)
{
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

  // If we've gone past the end of the list, then set the mode to pause and return
  if (d_whichFile == d_fileNames.end()) {
    d_mode = PAUSE;
    return false;
  }

  // Try to read the current file.  If we fail in the read, set the mode to paused so we don't keep trying.
  // If we succeed, then set up to read the next file.
  if (!read_image_from_file(*d_whichFile)) {
    fprintf(stderr,"file_stack_server::read_image_to_memory(): Could not read file\n");
    return false;
  }
  d_whichFile++;

  // Okay, we got a new frame!
  return true;
}

/// Get pixels out of the memory buffer
bool  file_stack_server::get_pixel_from_memory(unsigned X, unsigned Y, vrpn_uint8 &val, int color) const
{
  // Make sure we are within the range of allowed pixels
  if ( (X < _minX) || (Y < _minY) || (X > _maxX) || (Y > _maxY) ) {
    return false;
  }

  // Fill in the pixel value, assuming pixels vary in X fastest in the file.
  val = (vrpn_uint8)(d_buffer[ (X + Y * d_xFileSize) * 3 + color ] >> 8);

  return true;
}

bool  file_stack_server::get_pixel_from_memory(unsigned X, unsigned Y, vrpn_uint16 &val, int color) const
{
  // Make sure we are within the range of allowed pixels
  if ( (X < _minX) || (Y < _minY) || (X > _maxX) || (Y > _maxY) ) {
    return false;
  }

  // Fill in the pixel value, assuming pixels vary in X fastest in the file.
  val = d_buffer[ (X + Y * d_xFileSize) * 3 + color ];
  return true;
}

/// Store the memory image to a PPM file.
bool  file_stack_server::write_memory_to_ppm_file(const char *filename, int gain, bool sixteen_bits) const
{
  //XXX;
  fprintf(stderr,"edt_pulnix_raw_file_server::write_memory_to_ppm_file(): Not yet implemented\n");
  return false;

  return true;
}

/// Send whole image over a vrpn connection
bool  file_stack_server::send_vrpn_image(vrpn_TempImager_Server* svr,vrpn_Synchronized_Connection* svrcon,double g_exposure,int svrchan)
{
  ///XXX;
  fprintf(stderr,"edt_pulnix_raw_file_server::send_vrpn_image(): Not yet implemented\n");
  return false;

  return true;
}