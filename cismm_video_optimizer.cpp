//XXX Add an optional second tracker to do an orientation match in addition
// to a shift match.

//---------------------------------------------------------------------------
// This section contains configuration settings for the Video Spot Tracker.
// It is used to make it possible to compile and link the code when one or
// more of the camera- or file- driver libraries are unavailable. First comes
// a list of definitions.  These should all be defined when building the
// program for distribution.  Following that comes a list of paths and other
// things that depend on these definitions.  They may need to be changed
// as well, depending on where the libraries were installed.

#define	VST_USE_ROPER
//#define USE_METAMORPH	    // Metamorph reader not completed.
const bool g_show_video = true;

#ifdef	VST_USE_ROPER
#pragma comment(lib,"D:\\Program Files\\Roper Scientific\\PVCAM\\pvcam32.lib")
#endif

// END configuration section.
//---------------------------------------------------------------------------

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tcl.h>
#include <tk.h>
#include "Tcl_Linkvar.h"
#ifdef	VST_USE_ROPER
#include "roper_server.h"
#endif
#include "directx_camera_server.h"
#include "directx_videofile_server.h"
#include "diaginc_server.h"
#include "edt_server.h"
#include "SEM_camera_server.h"
#include "file_stack_server.h"
#include "image_wrapper.h"
#include "spot_tracker.h"
#ifdef	_WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glut.h>
#include <quat.h>
#include <vrpn_Types.h>
// This pragma tells the compiler not to tell us about truncated debugging info
// due to name expansion within the string, list, and vector classes.
#pragma warning( disable : 4786 )
#include <list>
using namespace std;

//#define	DEBUG

static void cleanup();
static void dirtyexit();

#ifndef	M_PI
const double M_PI = 2*asin(1.0);
#endif

//--------------------------------------------------------------------------
// Version string for this program
const char *Version_string = "01.01";

//--------------------------------------------------------------------------
// Global constants

const int KERNEL_DISC = 0;	  //< These must match the values used in cismm_video_optimizer.tcl.
const int KERNEL_CONE = 1;
const int KERNEL_SYMMETRIC = 2;

//--------------------------------------------------------------------------
// Some classes needed for use in the rest of the program.

class Spot_Information {
public:
  Spot_Information(spot_tracker *tracker) { d_tracker = tracker; d_index = d_static_index++; }

  spot_tracker *tracker(void) const { return d_tracker; }
  unsigned index(void) const { return d_index; }

protected:
  spot_tracker		*d_tracker;	    //< The tracker we're keeping information for
  unsigned		d_index;	    //< The index for this instance
  static unsigned	d_static_index;     //< The index to use for the next one (never to be re-used).
};
unsigned  Spot_Information::d_static_index = 0;	  //< Start the first instance of a Spot_Information index at zero.

class Controllable_Video {
public:
  /// Start the stored video playing.
  virtual void play(void) = 0;

  /// Pause the stored video
  virtual void pause(void) = 0;

  /// Rewind the stored video to the beginning (also pauses).
  virtual void rewind(void) = 0;

  /// Single-step the stored video for one frame.
  virtual void single_step() = 0;
};

class Directx_Controllable_Video : public Controllable_Video , public directx_videofile_server {
public:
  Directx_Controllable_Video(const char *filename) : directx_videofile_server(filename) {};
  virtual ~Directx_Controllable_Video() {};
  void play(void) { directx_videofile_server::play(); }
  void pause(void) { directx_videofile_server::pause(); }
  void rewind(void) { pause(); directx_videofile_server::rewind(); }
  void single_step(void) { directx_videofile_server::single_step(); }
};

class Pulnix_Controllable_Video : public Controllable_Video, public edt_pulnix_raw_file_server {
public:
  Pulnix_Controllable_Video(const char *filename) : edt_pulnix_raw_file_server(filename) {};
  virtual ~Pulnix_Controllable_Video() {};
  void play(void) { edt_pulnix_raw_file_server::play(); }
  void pause(void) { edt_pulnix_raw_file_server::pause(); }
  void rewind(void) { pause(); edt_pulnix_raw_file_server::rewind(); }
  void single_step(void) { edt_pulnix_raw_file_server::single_step(); }
};

class FileStack_Controllable_Video : public Controllable_Video, public file_stack_server {
public:
  FileStack_Controllable_Video(const char *filename) : file_stack_server(filename, "C:/nsrg/external/pc_win32/bin/ImageMagick-5.5.7-Q16/MAGIC_DIR_PATH") {};
  virtual ~FileStack_Controllable_Video() {};
  void play(void) { file_stack_server::play(); }
  void pause(void) { file_stack_server::pause(); }
  void rewind(void) { pause(); file_stack_server::rewind(); }
  void single_step(void) { file_stack_server::single_step(); }
};

#ifdef	USE_METAMORPH
class MetamorphStack_Controllable_Video : public Controllable_Video, public Metamorph_stack_server {
public:
  MetamorphStack_Controllable_Video(const char *filename) : Metamorph_stack_server(filename) {};
  virtual ~MetamorphStack_Controllable_Video() {};
  void play(void) { Metamorph_stack_server::play(); }
  void pause(void) { Metamorph_stack_server::pause(); }
  void rewind(void) { pause(); Metamorph_stack_server::rewind(); }
  void single_step(void) { Metamorph_stack_server::single_step(); }
};
#endif

class SEM_Controllable_Video : public Controllable_Video, public SEM_camera_server {
public:
  SEM_Controllable_Video(const char *filename) : SEM_camera_server(filename) {};
  virtual ~SEM_Controllable_Video() {};
  void play(void) { SEM_camera_server::play(); }
  void pause(void) { SEM_camera_server::pause(); }
  void rewind(void) { pause(); SEM_camera_server::rewind(); }
  void single_step(void) { SEM_camera_server::single_step(); }
};

//--------------------------------------------------------------------------
// Glut wants to take over the world when it starts, so we need to make
// global access to the objects we will be using.

char  *g_device_name = NULL;			  //< Name of the device to open
base_camera_server  *g_camera;			  //< Camera used to get an image
image_wrapper	    *g_image;			  //< Image wrapper for the camera
copy_of_image	    *g_last_image = NULL;	  //< Copy of the last image we had, if any
float		    g_search_radius = 0;	  //< Search radius for doing local max in before optimizing.
Controllable_Video  *g_video = NULL;		  //< Video controls, if we have them
Tclvar_int_with_button	g_frame_number("frame_number",NULL,-1);  //< Keeps track of video frame number

int		    g_tracking_window;		  //< Glut window displaying tracking
unsigned char	    *g_glut_image = NULL;	  //< Pointer to the storage for the image

list <Spot_Information *>g_trackers;		  //< List of active trackers
spot_tracker	    *g_active_tracker = NULL;	  //< The tracker that the controls refer to
bool		    g_ready_to_display = false;	  //< Don't unless we get an image
bool		    g_already_posted = false;	  //< Posted redisplay since the last display?
int		    g_mousePressX, g_mousePressY; //< Where the mouse was when the button was pressed
int		    g_whichDragAction;		  //< What action to take for mouse drag

FILE		    *g_csv_file = NULL;		  //< File to save data in with .csv extension

//--------------------------------------------------------------------------
// Tcl controls and displays
void  device_filename_changed(char *newvalue, void *);
void  logfilename_changed(char *newvalue, void *);
void  rebuild_trackers(int newvalue, void *);
void  rebuild_trackers(float newvalue, void *);
void  set_maximum_search_radius(int newvalue, void *);
Tclvar_float		g_X("x");
Tclvar_float		g_Y("y");
Tclvar_float_with_scale	g_Radius("radius", ".kernel.radius", 1, 30, 5);
Tclvar_float_with_scale	*g_minX;
Tclvar_float_with_scale	*g_maxX;
Tclvar_float_with_scale	*g_minY;
Tclvar_float_with_scale	*g_maxY;
Tclvar_float_with_scale	g_exposure("exposure_millisecs", "", 1, 1000, 10);
Tclvar_float_with_scale	g_colorIndex("red_green_blue", "", 0, 2, 0);
Tclvar_float_with_scale	g_bitdepth("bit_depth", "", 8, 12, 8);
Tclvar_float_with_scale g_precision("precision", "", 0.001, 1.0, 0.05, rebuild_trackers);
Tclvar_float_with_scale g_sampleSpacing("sample_spacing", "", 0.1, 1.0, 1.0, rebuild_trackers);
Tclvar_int_with_button	g_invert("dark_spot",NULL,1, rebuild_trackers);
Tclvar_int_with_button	g_interpolate("interpolate",NULL,1, rebuild_trackers);
Tclvar_int_with_button	g_areamax("areamax",NULL,0, set_maximum_search_radius);
Tclvar_int_with_button	g_kernel_type("kerneltype", NULL, KERNEL_SYMMETRIC, rebuild_trackers);
Tclvar_int_with_button	g_opt("optimize",".kernel.optimize");
Tclvar_int_with_button	g_round_cursor("round_cursor","");
Tclvar_int_with_button	g_full_area("full_area","");
Tclvar_int_with_button	g_mark("show_tracker","",1);
Tclvar_int_with_button	g_show_clipping("show_clipping","",0);
Tclvar_int_with_button	g_quit("quit","");
Tclvar_int_with_button	*g_play = NULL, *g_rewind = NULL, *g_step = NULL;
Tclvar_selector		g_logfilename("logfilename", NULL, NULL, "", logfilename_changed, NULL);
double			g_log_offset_x, g_log_offset_y;
char			*g_logfile_base_name = NULL;
copy_of_image		*g_log_last_image = NULL;
unsigned		g_log_frame_number_last_logged = -1;
bool g_video_valid = false; // Do we have a valid video frame in memory?

//--------------------------------------------------------------------------
// Helper routine to get the Y coordinate right when going between camera
// space and openGL space.
double	flip_y(double y)
{
  return g_camera->get_num_rows() - 1 - y;
}

/// Open the wrapped camera we want to use depending on the name of the
//  camera we're trying to open.
bool  get_camera_and_imager(const char *type, base_camera_server **camera, image_wrapper **imager,
			    Controllable_Video **video)
{
#ifdef VST_USE_ROPER
  if (!strcmp(type, "roper")) {
    // XXX Starts with binning of 2 to get the image size down so that
    // it fits on the screen.
    roper_server *r = new roper_server(2);
    *camera = r;
    *imager = r;
  } else
#endif  
  if (!strcmp(type, "diaginc")) {
    // XXX Starts with binning of 2 to get the image size down so that
    // it fits on the screen.
    diaginc_server *r = new diaginc_server(2);
    *camera = r;
    *imager = r;
    g_exposure = 80;	// Seems to be the minimum exposure for the one we have
  } else if (!strcmp(type, "directx")) {
    // Passing width and height as zero leaves it open to whatever the camera has
    directx_camera_server *d = new directx_camera_server(1,0,0);	// Use camera #1 (first one found)
    *camera = d;
    *imager = d;
  } else if (!strcmp(type, "directx640x480")) {
    directx_camera_server *d = new directx_camera_server(1,640,480);	// Use camera #1 (first one found)
    *camera = d;
    *imager = d;

  // If this is a VRPN URL for an SEM device, then open the file and set up
  // to read from that device.
  } else if (!strncmp(type, "SEM@", 4)) {
    SEM_Controllable_Video  *s = new SEM_Controllable_Video (type);
    *camera = s;
    *video = s;
    *imager = s;

  // Unknown type, so we presume that it is a file.  Now we figure out what
  // kind of file based on the extension and open the appropriate type of
  // imager.
  } else {
    fprintf(stderr,"get_camera_and_imager(): Assuming filename (%s)\n", type);

    // If the extension is ".raw" then we assume it is a Pulnix file and open
    // it that way.
    if (strcmp(".raw", &type[strlen(type)-4]) == 0) {
      Pulnix_Controllable_Video *f = new Pulnix_Controllable_Video(type);
      *camera = f;
      *video = f;
      *imager = f;

    // If the extension is ".sem" then we assume it is a VRPN-format file
    // with an SEM device in it, so we form the name of the device and open
    // a VRPN Remote object to handle it.
    } else if (strcmp(".sem", &type[strlen(type)-4]) == 0) {
      char *name;
      if ( NULL == (name = new char[strlen(type) + 20]) ) {
	fprintf(stderr,"Out of memory when allocating file name\n");
	cleanup();
        exit(-1);
      }
      sprintf(name, "SEM@file:%s", type);
      SEM_Controllable_Video *s = new SEM_Controllable_Video(name);
      *camera = s;
      *video = s;
      *imager = s;
      delete [] name;

    // If the extension is ".tif" or ".tiff" or ".bmp" then we assume it is
    // a file or stack of files to be opened by ImageMagick.
    } else if (   (strcmp(".tif", &type[strlen(type)-4]) == 0) ||
		  (strcmp(".TIF", &type[strlen(type)-4]) == 0) ||
		  (strcmp(".bmp", &type[strlen(type)-4]) == 0) ||
		  (strcmp(".BMP", &type[strlen(type)-4]) == 0) ||
		  (strcmp(".tiff", &type[strlen(type)-5]) == 0) || 
		  (strcmp(".TIFF", &type[strlen(type)-5]) == 0) ) {
      FileStack_Controllable_Video *s = new FileStack_Controllable_Video(type);
      *camera = s;
      *video = s;
      *imager = s;

    // If the extension is ".stk"  then we assume it is a Metamorph file
    // to be opened by the Metamorph reader.
#ifdef	USE_METAMORPH
    } else if (strcmp(".stk", &type[strlen(type)-4]) == 0) {
      MetamorphStack_Controllable_Video *s = new MetamorphStack_Controllable_Video(type);
      *camera = s;
      *video = s;
      *imager = s;

    // File of unknown type.  We assume that it is something that DirectX knows
    // how to open.
#endif
    } else {
      Directx_Controllable_Video *f = new Directx_Controllable_Video(type);
      *camera = f;
      *video = f;
      *imager = f;
    }
  }
  return true;
}


/// Create a pointer to a new tracker of the appropriate type,
// given the global settings for interpolation and inversion.
// Return NULL on failure.

spot_tracker  *create_appropriate_tracker(void)
{
    if (g_kernel_type == KERNEL_SYMMETRIC) {
      g_interpolate = 1;
      return new symmetric_spot_tracker_interp(g_Radius,(g_invert != 0), g_precision, 0.1, g_sampleSpacing);
    } else if (g_kernel_type == KERNEL_CONE) {
      g_interpolate = 1;
      return new cone_spot_tracker_interp(g_Radius,(g_invert != 0), g_precision, 0.1, g_sampleSpacing);
    } else if (g_interpolate) {
      return new disk_spot_tracker_interp(g_Radius,(g_invert != 0), g_precision, 0.1, g_sampleSpacing);
    } else {
      return new disk_spot_tracker(g_Radius,(g_invert != 0), g_precision, 0.1, g_sampleSpacing);
    }
}

static	bool  save_log_frame(unsigned frame_number)
{
    char  *filename = new char[strlen(g_logfile_base_name) + 15];
    if (filename == NULL) {
      fprintf(stderr, "Out of memory!\n");
      return false;
    }
    g_log_frame_number_last_logged = frame_number;
    sprintf(filename, "%s.opt.%04d.tif", g_logfile_base_name, frame_number);

    // Make a transformed image class to re-index the copied image.
    transformed_image shifted(*g_log_last_image, 
	g_active_tracker->get_x() - g_log_offset_x,
	g_active_tracker->get_y() - g_log_offset_y);

    if (!shifted.write_to_tiff_file(filename, 1, true)) {
      delete [] filename;
      return false;
    }
    (*g_log_last_image) = *g_camera;

    delete [] filename;
    return true;
}

//--------------------------------------------------------------------------
// Glut callback routines.

void drawStringAtXY(double x, double y, char *string)
{
  void *font = GLUT_BITMAP_TIMES_ROMAN_24;
  int len, i;

  glRasterPos2f(x, y);
  len = (int) strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, string[i]);
  }
}


// This is called when someone kills the task by closing Glut or some
// other means we don't have control over.
static void  dirtyexit(void)
{
  static bool did_already = false;

  if (did_already) {
    return;
  } else {
    did_already = true;
  }

  // Done with the camera and other objects.
  printf("Exiting\n");

  list<Spot_Information *>::iterator  loop;

  for (loop = g_trackers.begin(); loop != g_trackers.end(); loop++) {
    delete *loop;
  }
  if (g_camera) { delete g_camera; g_camera = NULL; }
  if (g_glut_image) { delete [] g_glut_image; g_glut_image = NULL; };
  if (g_play) { delete g_play; g_play = NULL; };
  if (g_rewind) { delete g_rewind; g_rewind = NULL; };
  if (g_step) { delete g_step; g_step = NULL; };
  if (g_csv_file) { fclose(g_csv_file); g_csv_file = NULL; g_csv_file = NULL; };
}

static void  cleanup(void)
{
  static bool cleaned_up_already = false;

  if (cleaned_up_already) {
    return;
  } else {
    cleaned_up_already = true;
  }

  // Done with the camera and other objects.
  printf("Cleanly ");

  dirtyexit();
}

void myDisplayFunc(void)
{
  unsigned  r,c;
  vrpn_uint16  uns_pix;

  if (!g_ready_to_display) { return; }

  // Clear the window and prepare to draw in the back buffer
  glDrawBuffer(GL_BACK);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);

  if (g_show_video) {
    // Copy pixels into the image buffer.  Flip the image over in
    // Y so that the image coordinates display correctly in OpenGL.
#ifdef DEBUG
    printf("Filling pixels %d,%d through %d,%d\n", (int)(*g_minX),(int)(*g_minY), (int)(*g_maxX), (int)(*g_maxY));
#endif
    int shift = g_bitdepth - 8;
    for (r = *g_minY; r <= *g_maxY; r++) {
      for (c = *g_minX; c <= *g_maxX; c++) {
	if (!g_camera->get_pixel_from_memory(c, r, uns_pix, g_colorIndex)) {
	  fprintf(stderr, "Cannot read pixel from region\n");
	  cleanup();
      	  exit(-1);
	}

	// This assumes that the pixels are actually 8-bit values
	// and will clip if they go above this.  It also writes pixels
	// from the first channel into all colors of the image.  It uses
	// RGBA so that we don't have to worry about byte-alignment problems
	// that plagued us when using RGB pixels.
	g_glut_image[0 + 4 * (c + g_camera->get_num_columns() * r)] = uns_pix >> shift;
	g_glut_image[1 + 4 * (c + g_camera->get_num_columns() * r)] = uns_pix >> shift;
	g_glut_image[2 + 4 * (c + g_camera->get_num_columns() * r)] = uns_pix >> shift;
	g_glut_image[3 + 4 * (c + g_camera->get_num_columns() * r)] = 255;

#ifdef DEBUG
	// If we're debugging, fill the border pixels with green
	if ( (r == *g_minY) || (r == *g_maxY) || (c == *g_minX) || (c == *g_maxX) ) {
	  g_glut_image[0 + 4 * (c + g_camera->get_num_columns() * r)] = 0;
	  g_glut_image[1 + 4 * (c + g_camera->get_num_columns() * r)] = 255;
	  g_glut_image[2 + 4 * (c + g_camera->get_num_columns() * r)] = 0;
	  g_glut_image[3 + 4 * (c + g_camera->get_num_columns() * r)] = 255;
	}
#endif
      }
    }

    // Store the pixels from the image into the frame buffer
    // so that they cover the entire image (starting from lower-left
    // corner, which is at (-1,-1)).
    glRasterPos2f(-1, -1);
#ifdef DEBUG
    printf("Drawing %dx%d pixels\n", g_camera->get_num_columns(), g_camera->get_num_rows());
#endif
    glDrawPixels(g_camera->get_num_columns(),g_camera->get_num_rows(),
      GL_RGBA, GL_UNSIGNED_BYTE, g_glut_image);
  }

  // If we have been asked to show the tracking markers, draw them.
  // The active one is drawn in red and the others are drawn in blue.
  // The markers may be either cross-hairs with a radius matching the
  // bead radius or a circle with a radius twice that of the bead; the
  // marker type depends on the g_round_cursor variable.
  if (g_mark) {
    list <Spot_Information *>::iterator loop;
    glEnable(GL_LINE_SMOOTH); //< Use smooth lines here to avoid aliasing showing spot in wrong place
    for (loop = g_trackers.begin(); loop != g_trackers.end(); loop++) {
      // Normalize center and radius so that they match the coordinates
      // (-1..1) in X and Y.
      double  x = -1.0 + (*loop)->tracker()->get_x() * (2.0/g_camera->get_num_columns());
      double  y = -1.0 + ((*loop)->tracker()->get_y()) * (2.0/g_camera->get_num_rows());
      double  dx = (*loop)->tracker()->get_radius() * (2.0/g_camera->get_num_columns());
      double  dy = (*loop)->tracker()->get_radius() * (2.0/g_camera->get_num_rows());

      if ((*loop)->tracker() == g_active_tracker) {
	glColor3f(1,0,0);
      } else {
	glColor3f(0,0,1);
      }
      if (g_round_cursor) {
	double stepsize = M_PI / (*loop)->tracker()->get_radius();
	double runaround;
	glBegin(GL_LINE_STRIP);
	  for (runaround = 0; runaround <= 2*M_PI; runaround += stepsize) {
	    glVertex2f(x + 2*dx*cos(runaround),y + 2*dy*sin(runaround));
	  }
	  glVertex2f(x + 2*dx, y);  // Close the circle
	glEnd();
      } else {
	glBegin(GL_LINES);
	  glVertex2f(x-dx,y);
	  glVertex2f(x+dx,y);
	  glVertex2f(x,y-dy);
	  glVertex2f(x,y+dy);
	glEnd();
      }
      // Label the marker with its index
      char numString[10];
      sprintf(numString,"%d", (*loop)->index());
      drawStringAtXY(x+dx,y, numString);
    }
  }

  // Draw a green border around the selected area.  It will be beyond the
  // window border if it is set to the edges; it will just surround the
  // region being selected if it is inside the window.
  glDisable(GL_LINE_SMOOTH);
  glColor3f(0,1,0);
  glBegin(GL_LINE_STRIP);
  glVertex3f( -1 + 2*((*g_minX-1) / (g_camera->get_num_columns()-1)),
	      -1 + 2*((*g_minY-1) / (g_camera->get_num_rows()-1)) , 0.0 );
  glVertex3f( -1 + 2*((*g_maxX+1) / (g_camera->get_num_columns()-1)),
	      -1 + 2*((*g_minY-1) / (g_camera->get_num_rows()-1)) , 0.0 );
  glVertex3f( -1 + 2*((*g_maxX+1) / (g_camera->get_num_columns()-1)),
	      -1 + 2*((*g_maxY+1) / (g_camera->get_num_rows()-1)) , 0.0 );
  glVertex3f( -1 + 2*((*g_minX-1) / (g_camera->get_num_columns()-1)),
	      -1 + 2*((*g_maxY+1) / (g_camera->get_num_rows()-1)) , 0.0 );
  glVertex3f( -1 + 2*((*g_minX-1) / (g_camera->get_num_columns()-1)),
	      -1 + 2*((*g_minY-1) / (g_camera->get_num_rows()-1)) , 0.0 );
  glEnd();

  // Swap buffers so we can see it.
  glutSwapBuffers();

  // Capture timing information and print out how many frames per second
  // are being drawn.

  { static struct timeval last_print_time;
    struct timeval now;
    static bool first_time = true;
    static int frame_count = 0;

    if (first_time) {
      gettimeofday(&last_print_time, NULL);
      first_time = false;
    } else {
      frame_count++;
      gettimeofday(&now, NULL);
      double timesecs = 0.001 * vrpn_TimevalMsecs(vrpn_TimevalDiff(now, last_print_time));
      if (timesecs >= 5) {
	double frames_per_sec = frame_count / timesecs;
	frame_count = 0;
	printf("Displayed frames per second = %lg\n", frames_per_sec);
	last_print_time = now;
      }
    }
  }

  // Have no longer posted redisplay since the last display.
  g_already_posted = false;
}
static	double	timediff(struct timeval t1, struct timeval t2)
{
	return (t1.tv_usec - t2.tv_usec) / 1e6 +
	       (t1.tv_sec - t2.tv_sec);
}

void myIdleFunc(void)
{
  list<Spot_Information *>::iterator loop;

  //------------------------------------------------------------
  // This must be done in any Tcl app, to allow Tcl/Tk to handle
  // events.  This must happen at the beginning of the idle function
  // so that the camera-capture and video-display routines are
  // using the same values for the global parameters.

  while (Tk_DoOneEvent(TK_DONT_WAIT)) {};

  //------------------------------------------------------------
  // This is called once every time through the main loop.  It
  // pushes changes in the C variables over to Tcl.

  if (Tclvar_mainloop()) {
    fprintf(stderr,"Tclvar Mainloop failed\n");
  }

  // If they just asked for the full area, reset to that and
  // then turn off the check-box.
  if (g_full_area) {
    *g_minX = 0;
    *g_minY = 0;
    *g_maxX = g_camera->get_num_columns() - 1;
    *g_maxY = g_camera->get_num_rows() - 1;
    g_full_area = 0;
  }

  // If we're doing a search for local maximum during optimization, then make a
  // copy of the previous image before reading a new one.

  if (g_search_radius > 0) {
    if (g_last_image == NULL) {
      g_last_image = new copy_of_image(*g_image);
    } else {
      *g_last_image = *g_image;
    }
  }

  // Read an image from the camera into memory, within a structure that
  // is wrapped by an image_wrapper object that the tracker can use.
  // Tell Glut that it can display an image.
  // We ignore the error return if we're doing a video file because
  // this can happen due to timeouts when we're paused or at the
  // end of a file.
  if (!g_camera->read_image_to_memory((int)(*g_minX),(int)(*g_maxX), (int)(*g_minY),(int)(*g_maxY), g_exposure)) {
    if (!g_video) {
      fprintf(stderr, "Can't read image to memory!\n");
      cleanup();
      exit(-1);
    } else {
      // We timed out; either paused or at the end.  Don't log in this case.
      g_video_valid = false;
    }
  } else {
    // Got a valid video frame; can log it.  Add to the frame number.
    g_video_valid = true;
    g_frame_number++;
  }
  g_ready_to_display = true;

  // If we've gotten a new valid frame, then it is time to store the image
  // for the previous frame and get a copy of the current frame so that we
  // can store it next time around.  We do this after saving the previous
  // frame (named based on the base log file name and the past frame number).
  if (g_log_last_image && g_video_valid) {
    if (!save_log_frame(g_frame_number - 1)) {
      fprintf(stderr,"Couldn't save log file\n");
      cleanup();
      exit(-1);
    }
  }

  if (g_active_tracker) { 
    g_active_tracker->set_radius(g_Radius);
  }
  if (g_opt && g_active_tracker) {
    double  x, y;

    // This variable is used to determine if we should consider doing maximum fits,
    // by determining if the frame number has changed since last time.
    static int last_optimized_frame_number = -1000;

    // Optimize to find the best fit starting from last position for each tracker.
    // Invert the Y values on the way in and out.
    // Don't let it adjust the radius here (otherwise it gets too
    // jumpy).
    for (loop = g_trackers.begin(); loop != g_trackers.end(); loop++) {

      // If the frame-number has changed, and we are doing global searches
      // within a radius, then create an image-based tracker at the last
      // location for the current tracker on the last frame; scan all locations
      // within radius and find the maximum fit on this frame; move the tracker
      // location to that maximum before doing the optimization.  We use the
      // image-based tracker for this because other trackers may have maximum
      // fits outside the region where the bead is -- the symmetric tracker often
      // has a local maximum at best fit and global maxima elsewhere.
      if ( g_last_image && (g_search_radius > 0) && (last_optimized_frame_number != g_frame_number) ) {

	double x_base = (*loop)->tracker()->get_x();
	double y_base = (*loop)->tracker()->get_y();

	// Create an image spot tracker and initize it at the location where the current
	// tracker is, but in the last image.  Grab enough of the image that we will be able
	// to check over the g_search_radius for a match.
	image_spot_tracker_interp max_find((*loop)->tracker()->get_radius(), (g_invert != 0), g_precision,
	  0.1, g_sampleSpacing);
	max_find.set_location(x_base, y_base);
	max_find.set_image(*g_last_image, x_base, y_base, (*loop)->tracker()->get_radius() + g_search_radius);

	// Loop over the pixels within g_search_radius of the initial location and find the
	// location with the best match over all of these points.  Do this in the current image.
	double radsq = g_search_radius * g_search_radius;
	int x_offset, y_offset;
	int best_x_offset = 0;
	int best_y_offset = 0;
	double best_value = max_find.check_fitness(*g_image);
	for (x_offset = -floor(g_search_radius); x_offset <= floor(g_search_radius); x_offset++) {
	  for (y_offset = -floor(g_search_radius); y_offset <= floor(g_search_radius); y_offset++) {
	    if ( (x_offset * x_offset) + (y_offset * y_offset) <= radsq) {
	      max_find.set_location(x_base + x_offset, y_base + y_offset);
	      double val = max_find.check_fitness(*g_image);
	      if (val > best_value) {
		best_x_offset = x_offset;
		best_y_offset = y_offset;
		best_value = val;
	      }
	    }
	  }
	}

	// Put the tracker at the location of the maximum, so that it will find the
	// total maximum when it finds the local maximum.
	(*loop)->tracker()->set_location(x_base + best_x_offset, y_base + best_y_offset);
      }

      // Here's where the tracker is optimized to its new location
      (*loop)->tracker()->optimize_xy(*g_image, x, y, (*loop)->tracker()->get_x(), (*loop)->tracker()->get_y() );
    }

    last_optimized_frame_number = g_frame_number;
  }

  //------------------------------------------------------------
  // Make the GUI track the result for the active tracker
  if (g_active_tracker) {
    g_X = (float)g_active_tracker->get_x();
    g_Y = (float)flip_y(g_active_tracker->get_y());
    g_Radius = (float)g_active_tracker->get_radius();
  }

  //------------------------------------------------------------
  // If we have a video object, let the video controls operate
  // on it.
  if (g_video) {
    static  int	last_play = 0;

    // If the user has pressed step, then run the video for a
    // single step and pause it.
    if (*g_step) {
      g_video->single_step();
      *g_play = 0;
      *g_step = 0;
    }

    // If the user has pressed play, start the video playing
    if (!last_play && *g_play) {
      g_video->play();
      *g_rewind = 0;
    }

    // If the user has cleared play, then pause the video
    if (last_play && !(*g_play)) {
      g_video->pause();
    }
    last_play = *g_play;

    // If the user has pressed rewind, go the the beginning of
    // the stream and then pause (by clearing play).  This has
    // to come after the checking for stop play above so that the
    // video doesn't get paused.  Also, reset the frame count
    // when we rewind.
    if (*g_rewind) {
      *g_play = 0;
      *g_rewind = 0;
      g_video->rewind();
      g_frame_number = -1;
    }
  }

  //------------------------------------------------------------
  // Post a redisplay so that Glut will draw the new image
  if (!g_already_posted) {
    glutSetWindow(g_tracking_window);
    glutPostRedisplay();
    g_already_posted = true;
  }

  //------------------------------------------------------------
  // Time to quit?
  if (g_quit) {
    // If we have been logging, then see if we have saved the
    // current frame's image.  If not, go ahead and do it now.
    if (g_log_last_image && (g_log_frame_number_last_logged != g_frame_number)) {
      if (!save_log_frame(g_frame_number)) {
	fprintf(stderr, "logfile_changed: Could not save log frame\n");
	cleanup();
	exit(-1);
      }
    }

    cleanup();
    exit(0);
  }
  
  // Sleep a little while so that we don't eat the whole CPU.
  vrpn_SleepMsecs(1);
}

// This routine finds the tracker whose coordinates are
// the nearest to those specified, makes it the active
// tracker, and moved it to the specified location.
void  activate_and_drag_nearest_tracker_to(double x, double y)
{
  // Looks for the minimum squared distance and the tracker that is
  // there.
  double  minDist2 = 1e100;
  Spot_Information *minTracker = NULL;
  double  dist2;
  list<Spot_Information *>::iterator loop;

  for (loop = g_trackers.begin(); loop != g_trackers.end(); loop++) {
    dist2 = (x - (*loop)->tracker()->get_x())*(x - (*loop)->tracker()->get_x()) +
      (y - (*loop)->tracker()->get_y())*(y - (*loop)->tracker()->get_y());
    if (dist2 < minDist2) {
      minDist2 = dist2;
      minTracker = *loop;
    }
  }
  if (minTracker == NULL) {
    fprintf(stderr, "No tracker to pick out of %d\n", g_trackers.size());
  } else {
    g_active_tracker = minTracker->tracker();
    g_active_tracker->set_location(x, y);
    g_X = g_active_tracker->get_x();
    g_Y = flip_y(g_active_tracker->get_y());
    g_Radius = g_active_tracker->get_radius();
  }
}

void keyboardCallbackForGLUT(unsigned char key, int x, int y)
{
  switch (key) {
  case 'q':
  case 'Q':
    g_quit = 1;
    break;

  case 8:   // Backspace
  case 127: // Delete on Windows
    // Nothing yet...
    break;
  }
}

void mouseCallbackForGLUT(int button, int state, int x, int y)
{
    // Record where the button was pressed for use in the motion
    // callback, flipping the Y axis to make the coordinates match
    // image coordinates.
    g_mousePressX = x;
    g_mousePressY = y = flip_y(y);

    switch(button) {
      // The right button will set the clipping window to a single-
      // pixel-wide spot surrounding the click location and it will
      // set the motion callback mode to expand the area as the user
      // moves.
      case GLUT_RIGHT_BUTTON:
	if (state == GLUT_DOWN) {
	  g_whichDragAction = 1;
	  *g_minX = x;
	  *g_maxX = x;
	  *g_minY = y;
	  *g_maxY = y;
	} else {
	  // Nothing to do at release.
	}
	break;

      case GLUT_MIDDLE_BUTTON:
	if (state == GLUT_DOWN) {
	  g_whichDragAction = 0;
	}
	break;

      // The left button will pull the closest existing tracker
      // to the location where the mouse button was pressed, and
      // then let the user pull it around the screen
      case GLUT_LEFT_BUTTON:
	if (state == GLUT_DOWN) {
	  g_whichDragAction = 2;
	  activate_and_drag_nearest_tracker_to(x,y);
	}
	break;
    }
}

void motionCallbackForGLUT(int x, int y) {

  // Make mouse coordinates match image coordinates.
  y = flip_y(y);

  switch (g_whichDragAction) {

  case 0: //< Do nothing on drag.
    break;

  // Expand the selection area so that it is a rectangle centered
  // on the press location, clipped to the boundaries of the video.
  case 1:
    {
      int x_dist = (int)(fabs(x-g_mousePressX));
      int y_dist = (int)(fabs(y-g_mousePressY));
      int max_dist;
      if (x_dist > y_dist) { max_dist = x_dist; }
      else { max_dist = y_dist; }

      *g_minX = g_mousePressX - max_dist;
      *g_maxX = g_mousePressX + max_dist;
      *g_minY = g_mousePressY - max_dist;
      *g_maxY = g_mousePressY + max_dist;

      // Clip the size to stay within the window boundaries.
      if (*g_minX < 0) { *g_minX = 0; };
      if (*g_minY < 0) { *g_minY = 0; };
      if (*g_maxX >= (int)g_camera->get_num_columns()) { *g_maxX = g_camera->get_num_columns() - 1; }
      if (*g_maxY >= (int)g_camera->get_num_rows()) { *g_maxY = g_camera->get_num_rows() - 1; };
    }
    break;

  // Pull the closest existing tracker
  // to the location where the mouse button was pressed, and
  // keep pulling it around if the mouse is moved while this
  // button is held down.
  case 2:
    activate_and_drag_nearest_tracker_to(x,y);
    break;

  default:
    fprintf(stderr,"Internal Error: Unknown drag action (%d)\n", g_whichDragAction);
  }
  return;
}


//--------------------------------------------------------------------------
// Tcl callback routines.

// If the device filename becomes non-empty, then set the global
// device name to match what it is set to.

void  device_filename_changed(char *newvalue, void *)
{
  if (strlen(newvalue) > 0) {
    g_device_name = new char[strlen(newvalue)+1];
    strcpy(g_device_name, newvalue);
  }
}

// If the logfilename becomes non-empty, then start a new sequence
// of images to be stored.  This is done by setting the global log
// file base name to the value of the file name.
// We use the "newvalue" here rather than the file name because the
// file name gets truncated to the maximum TCLVAR string length.

void  logfilename_changed(char *newvalue, void *)
{
  static  char	name_buffer[4096];

  // If we have been logging, then see if we have saved the
  // current frame's image.  If not, go ahead and do it now.
  if (g_log_last_image && (g_log_frame_number_last_logged != g_frame_number)) {
    if (!save_log_frame(g_frame_number)) {
      fprintf(stderr, "logfile_changed: Could not save log frame\n");
      cleanup();
      exit(-1);
    }
  }

  // Stop the old logging by getting rid of the last log image
  // if there is one and resetting the logging parameters.
  if (g_log_last_image) {
    delete g_log_last_image;
    g_log_last_image = NULL;
  }
  g_log_frame_number_last_logged = -1;
  
  // If we have an empty name, then clear the global logging base
  // name so that no more files will be saved.
  strncpy(name_buffer, newvalue, sizeof(name_buffer)-1);
  name_buffer[sizeof(name_buffer)-1] = '\0';
  if (strlen(newvalue) == 0) {
    g_logfile_base_name = NULL;
    return;
  }

  // Store the name.
  g_logfile_base_name = name_buffer;

  // Make a copy of the current image so that we can save it when
  // it is appropriate to do so.
  g_log_last_image = new copy_of_image(*g_camera);

  // Set the offsets to use when logging to the current position of
  // the zeroeth tracker.
  g_log_offset_x = g_active_tracker->get_x();
  g_log_offset_y = g_active_tracker->get_y();
}

// Routine that rebuilds all trackers with the format that matches
// current settings.  This callback is called when one of the Tcl
// integer-with-buttons changes its value.

void  rebuild_trackers(int newvalue, void *)
{
  // Delete all trackers and replace with the correct types.
  // Make sure to put them back where they came from.
  // Re-point the active tracker at its corresponding new
  // tracker.
  list<Spot_Information *>::iterator  loop;

  for (loop = g_trackers.begin(); loop != g_trackers.end(); loop++) {
    double x = (*loop)->tracker()->get_x();
    double y = (*loop)->tracker()->get_y();
    double r = (*loop)->tracker()->get_radius();

    if (g_active_tracker == (*loop)->tracker()) {
      delete (*loop)->tracker();
      delete *loop;
      *loop = new Spot_Information(create_appropriate_tracker());
      g_active_tracker = (*loop)->tracker();
    } else {
      delete (*loop)->tracker();
      delete *loop;
      *loop = new Spot_Information(create_appropriate_tracker());
    }
    (*loop)->tracker()->set_location(x,y);
    (*loop)->tracker()->set_radius(r);
  }
}

// This version is for float sliders
void  rebuild_trackers(float newvalue, void *) {
  rebuild_trackers((int)newvalue, NULL);
}

// Sets the radius as the check-box is turned on (1) and off (0);
// it will be set to the current bead radius
void  set_maximum_search_radius(int newvalue, void *)
{
  g_search_radius = g_Radius * newvalue;
}


//--------------------------------------------------------------------------

int main(int argc, char *argv[])
{
  //------------------------------------------------------------------
  // If there is a command-line argument, treat it as the name of a
  // file that is to be loaded.
  switch (argc) {
  case 1:
    // No arguments, so ask the user for a file name
    g_device_name = NULL;
    break;
  case 2:
    // Filename argument: open the file specified.
    g_device_name = argv[1];
    break;

  default:
    fprintf(stderr, "Usage: %s [roper|diaginc|directx|directx640x480|filename]\n", argv[0]);
    exit(-1);
  };
  
  // Set up exit handler to make sure we clean things up no matter
  // how we are quit.  We hope that we exit in a good way and so
  // cleanup() gets called, but if not then we do a dirty exit.
  atexit(dirtyexit);

  //------------------------------------------------------------------
  // VRPN state setting so that we don't try to preload a video file
  // when it is opened, which wastes time.  Also tell it not to
  // accumulate messages, which can cause us to run out of memory.
  vrpn_FILE_CONNECTIONS_SHOULD_PRELOAD = false;
  vrpn_FILE_CONNECTIONS_SHOULD_ACCUMULATE = false;

  //------------------------------------------------------------------
  // Generic Tcl startup.  Getting and interpreter and mainwindow.

  char		command[256];
  Tcl_Interp	*tk_control_interp;
  Tk_Window       tk_control_window;
  tk_control_interp = Tcl_CreateInterp();

  /* Start a Tcl interpreter */
  if (Tcl_Init(tk_control_interp) == TCL_ERROR) {
          fprintf(stderr,
                  "Tcl_Init failed: %s\n",tk_control_interp->result);
          return(-1);
  }

  /* Start a Tk mainwindow to hold the widgets */
  if (Tk_Init(tk_control_interp) == TCL_ERROR) {
	  fprintf(stderr,
	  "Tk_Init failed: %s\n",tk_control_interp->result);
	  return(-1);
  }
  tk_control_window = Tk_MainWindow(tk_control_interp);
  if (tk_control_window == NULL) {
          fprintf(stderr,"%s\n", tk_control_interp->result);
          return(-1);
  }

  //------------------------------------------------------------------
  // Loading the particular definition files we need.  russ_widgets is
  // required by the Tclvar_float_with_scale class.  simple_magnet_drive
  // is application-specific and sets up the controls for the integer
  // and float variables.

  /* Load the Tcl scripts that handle widget definition and
   * variable controls */
  sprintf(command, "source russ_widgets.tcl");
  if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
          fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
                  tk_control_interp->result);
          return(-1);
  }

  //------------------------------------------------------------------
  // Put the version number into the main window.
  sprintf(command, "label .versionlabel -text CISMM_Video_Optimizer_v:%s", Version_string);
  if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
          fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
                  tk_control_interp->result);
          return(-1);
  }
  sprintf(command, "pack .versionlabel");
  if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
          fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
                  tk_control_interp->result);
          return(-1);
  }

  //------------------------------------------------------------------
  // Load the specialized Tcl code needed by this program.  This must
  // be loaded before the Tclvar_init() routine is called because it
  // puts together some of the windows needed by the variables.
  sprintf(command, "source cismm_video_optimizer.tcl");
  if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
          fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
                  tk_control_interp->result);
          return(-1);
  }

  //------------------------------------------------------------------
  // This routine must be called in order to initialize all of the
  // variables that came into scope before the interpreter was set
  // up, and to tell the variables which interpreter to use.  It is
  // called once, after the interpreter exists.

  // Initialize the variables using the interpreter
  if (Tclvar_init(tk_control_interp)) {
	  fprintf(stderr,"Can't do init!\n");
	  return -1;
  }
  sprintf(command, "wm geometry . +10+10");
  if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
          fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
                  tk_control_interp->result);
          return(-1);
  }

  //------------------------------------------------------------------
  // If we don't have a device name, then throw a Tcl dialog asking
  // the user for the name of a file to use and wait until they respond.
  if (g_device_name == NULL) {

    //------------------------------------------------------------
    // Create a callback for a variable that will hold the device
    // name and then create a dialog box that will ask the user
    // to either fill it in or quit.
    Tclvar_selector filename("device_filename", NULL, NULL, "", device_filename_changed, NULL);
    if (Tcl_Eval(tk_control_interp, "ask_user_for_filename") != TCL_OK) {
      fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command, tk_control_interp->result);
      cleanup();
      exit(-1);
    }

    do {
      //------------------------------------------------------------
      // This pushes changes in the C variables over to Tcl.

      while (Tk_DoOneEvent(TK_DONT_WAIT)) {};
      if (Tclvar_mainloop()) {
	fprintf(stderr,"Tclvar Mainloop failed\n");
      }
    } while ( (g_device_name == NULL) && !g_quit);
    if (g_quit) {
      cleanup();
      exit(0);
    }
  }

  //------------------------------------------------------------------
  // Open the camera and image wrapper.  If we have a video file, then
  // set up the Tcl controls to run it.  Also, report the frame number.
  if (!get_camera_and_imager(g_device_name, &g_camera, &g_image, &g_video)) {
    fprintf(stderr,"Cannot open camera/imager\n");
    if (g_camera) { delete g_camera; g_camera = NULL; }
    cleanup();
    exit(-1);
  }
  if (g_video) {  // Put these in a separate control panel?
    // Start out paused at the beginning of the file.
    g_play = new Tclvar_int_with_button("play_video","",0);
    g_rewind = new Tclvar_int_with_button("rewind_video","",1);
    g_step = new Tclvar_int_with_button("single_step_video","");
    sprintf(command, "label .frametitle -text FrameNum");
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
	    fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
		    tk_control_interp->result);
	    return(-1);
    }
    sprintf(command, "pack .frametitle");
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
	    fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
		    tk_control_interp->result);
	    return(-1);
    }
    sprintf(command, "label .framevalue -textvariable frame_number");
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
	    fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
		    tk_control_interp->result);
	    return(-1);
    }
    sprintf(command, "pack .framevalue");
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
	    fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
		    tk_control_interp->result);
	    return(-1);
    }
  }

  // Verify that the camera is working.
  if (!g_camera->working()) {
    fprintf(stderr,"Could not establish connection to camera\n");
    if (g_camera) { delete g_camera; g_camera = NULL; }
    cleanup();
    exit(-1);
  }

  //------------------------------------------------------------------
  // Initialize the controls for the clipping based on the size of
  // the image we got.
  g_minX = new Tclvar_float_with_scale("minX", ".clipping", 0, g_camera->get_num_columns()-1, 0);
  g_maxX = new Tclvar_float_with_scale("maxX", ".clipping", 0, g_camera->get_num_columns()-1, g_camera->get_num_columns()-1);
  g_minY = new Tclvar_float_with_scale("minY", ".clipping", 0, g_camera->get_num_rows()-1, 0);
  g_maxY = new Tclvar_float_with_scale("maxY", ".clipping", 0, g_camera->get_num_rows()-1, g_camera->get_num_rows()-1);

  //------------------------------------------------------------------
  // Initialize GLUT and create the window that will display the
  // video -- name the window after the device that has been
  // opened in VRPN.  Also set mouse callbacks.
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowPosition(190, 230);
  glutInitWindowSize(g_camera->get_num_columns(), g_camera->get_num_rows());
#ifdef DEBUG
  printf("initializing window to %dx%d\n", g_camera->get_num_columns(), g_camera->get_num_rows());
#endif
  g_tracking_window = glutCreateWindow(g_device_name);
  glutMotionFunc(motionCallbackForGLUT);
  glutMouseFunc(mouseCallbackForGLUT);
  glutKeyboardFunc(keyboardCallbackForGLUT);

  // Create the buffer that Glut will use to send to the tracking window.  This is allocating an
  // RGBA buffer.  It needs to be 4-byte aligned, so we allocated it as a group of
  // words and then cast it to the right type.  We're using RGBA rather than just RGB
  // because it also solves the 4-byte alignment problem caused by funky sizes of image
  // that are RGB images.
  if ( (g_glut_image = (unsigned char *)(void*)new vrpn_uint32
      [g_camera->get_num_columns() * g_camera->get_num_rows()]) == NULL) {
    fprintf(stderr,"Out of memory when allocating image!\n");
    fprintf(stderr,"  (Image is %u by %u)\n", g_camera->get_num_columns(), g_camera->get_num_rows());
    if (g_camera) { delete g_camera; g_camera = NULL; }
    cleanup();
    exit(-1);
  }

  // Create a tracker and place it in the center of the window.
  g_trackers.push_back(new Spot_Information(g_active_tracker = create_appropriate_tracker()));
  g_active_tracker->set_location(g_camera->get_num_columns()/2, g_camera->get_num_rows()/2);


  // Set the display functions for each window and idle function for GLUT (they
  // will do all the work) and then give control over to GLUT.
  glutSetWindow(g_tracking_window);
  glutDisplayFunc(myDisplayFunc);
  glutIdleFunc(myIdleFunc);

  glutMainLoop();
  // glutMainLoop() NEVER returns.  Wouldn't it be nice if it did when Glut was done?
  // Nope: Glut calls exit(0);

  return 0;
}
