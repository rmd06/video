#include "PixelLine.h"


BEGIN_EVENT_TABLE(PixelLine, wxGLCanvas)
    EVT_SIZE(PixelLine::OnSize)
    EVT_PAINT(PixelLine::OnPaint)
    EVT_ERASE_BACKGROUND(PixelLine::OnEraseBackground)
    EVT_MOUSE_EVENTS(PixelLine::OnMouse)
END_EVENT_TABLE()

PixelLine::PixelLine(wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name, bool horizontal)
    : wxGLCanvas(parent, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE, name)
{
	InitGL();
	ResetProjectionMode();

	p_len = 0;

	m_horizontal = horizontal;
}

PixelLine::~PixelLine()
{
}

void PixelLine::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    // must always be here
    wxPaintDC dc(this);

#ifndef __WXMOTIF__
    if (!GetContext()) return;
#endif

    SetCurrent();
/*
    // Clear
	float red, green, blue;

	red = rand()/(float)RAND_MAX;
	green = rand()/(float)RAND_MAX;
	blue = rand()/(float)RAND_MAX;

    glClearColor( red, green, blue, 1.0f );
*/
	glClearColor( 0, 0, 0, 1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Transformations
    glLoadIdentity();

	glBegin(GL_QUADS);

	float dx = 2.0f / (float)p_len;

	/*
	if (m_horizontal)
		printf("H:\n");
	else
		printf("V:\n");

	printf("\tSMD = %f\n", calcSMD(0));
	*/

	for (int i = 0; i < p_len; ++i)
	{
		// set current pixel color
//		printf("%i: (%f, %f, %f)\n", i, R[i], G[i], B[i]);
		glColor3f(R[i], G[i], B[i]);
		if (m_horizontal)
		{
			glVertex2f(-1 + i * dx, -1);
			glVertex2f(-1 + i * dx, 1);
			glVertex2f(-1 + (i + 1) * dx, 1);
			glVertex2f(-1 + (i + 1) * dx, -1);
		}
		else
		{
			glVertex2f(-1, -1 + i * dx);
			glVertex2f(1, -1 + i * dx);
			glVertex2f(1, -1 + (i + 1) * dx);
			glVertex2f(-1, -1 + (i + 1) * dx);
		}
	}

	glEnd();

    // Flush
    glFlush();

    // Swap
    SwapBuffers();
}


void PixelLine::OnSize(wxSizeEvent& event)
{
    // this is also necessary to update the context on some platforms
    wxGLCanvas::OnSize(event);
    // Reset the OpenGL view aspect
    ResetProjectionMode();
}

void PixelLine::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
    // Do nothing, to avoid flashing on MSW
}

void PixelLine::OnMouse(wxMouseEvent& event)
{
    if (event.Dragging())
    {
        wxSize sz(GetClientSize());

        /* orientation has changed, redraw mesh */
        Refresh(false);
    }

    //m_gldata.beginx = event.GetX();
    //m_gldata.beginy = event.GetY();
}

void PixelLine::InitGL()
{
    glEnable(GL_DEPTH_TEST);

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
}

void PixelLine::ResetProjectionMode()
{
    int w, h;
    GetClientSize(&w, &h);
#ifndef __WXMOTIF__
    if ( GetContext() )
#endif
    {
        SetCurrent();
        glViewport(0, 0, (GLint) w, (GLint) h);

		// set up the projection matrix 
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();	

		gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
    }
}

float PixelLine::calcFocus(int channel, int method, int weightedMethod)
{
	float Ia = 0, Ib = 0;
	float sum = 0;
	float val = 0;

	float half = p_len / 2.0f;

	if (method == 0 || method == 1)
	{
		for (int i = 0; i < p_len - 1; ++i)
		{
			if (channel == 0) // red channel
			{
				Ia = R[i];
				Ib = R[i+1];
			}
			else if (channel == 1) // green channel
			{
				Ia = G[i];
				Ib = G[i+1];
			}
			else if (channel == 2) // blue channel
			{
				Ia = B[i];
				Ib = B[i+1];
			}
			else // INVALID CHANNEL
			{
				printf("Invalid channel \"%i\" passed to PixelLine::calcFocus()\n", channel);
			}

			if (Ia + Ib != 0)
			{
				val = abs(Ia - Ib);// / (Ia + Ib);
				float weight = 1.0f;
				if (weightedMethod == 1)
				{
					// weight with a simple 0-1 tent function centered at the bead center
					weight = 1 - abs(((float)i + 0.5f) - half) / half;
				}
				val *= weight;
			}
			else
			{
				val = 0;
			}
			sum += val;
		}
	} // end method == 0
	
	if (method == 1)
	{
		float center = 0;
		if (channel == 0) // red channel
			{
				center = R[p_len / 2];
			}
			else if (channel == 1) // green channel
			{
				center = G[p_len / 2];
			}
			else if (channel == 2) // blue channel
			{
				center = B[p_len / 2];
			}
			else // INVALID CHANNEL
			{
				printf("Invalid channel \"%i\" passed to PixelLine::calcFocus()\n", channel);
			}
		sum *= center;
	}

	return sum;
}