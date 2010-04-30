#include <GL/glew.h>
#include <wx/wx.h>
#include "canvas.hh"
#include "assert.hh"

BEGIN_EVENT_TABLE(Canvas, wxGLCanvas)
EVT_PAINT(Canvas::OnPaint)
END_EVENT_TABLE()

Canvas::Canvas(wxWindow* parent, int *attribList, long style, const wxString& name)
	: wxGLCanvas(parent, wxID_ANY, attribList, wxDefaultPosition, wxDefaultSize, style, name)
	, m_context(0)
	, m_controller(0)
	, m_last_controller(0)
	, m_lazy_init(false)
{
}

void Canvas::Render()
{
	if(m_context)
	{
		int width, height;
		GetClientSize(&width, &height);
		SetCurrent(*m_context);                 

		// because calling SetCurrent() at the wrong time causes a messed up gdk error in X. The following works but it's pretty ugly.
		if(!m_lazy_init) { 
			// must be initialized for this GL context
			GLenum err = glewInit(); 
			if(err != GLEW_OK) {
				fprintf(stderr, "glewInit error: %s\n", glewGetErrorString(err));
			}
			printf("Using GLEW %s\n", glewGetString(GLEW_VERSION));							
			m_lazy_init = true;
		}

		// lazy-init (enter) canvas controllers since some of them rely on glew being initialized.
		if(m_last_controller != m_controller) {
			if(m_last_controller) {
				m_last_controller->Exit();
			}

			if(m_controller) {
				m_controller->Enter();
			}
			m_last_controller = m_controller;
		}

		if(m_controller)
		{
			m_controller->Render(width, height);
		}
		else 
		{
			glViewport(0,0,width,height);
			glClearColor(0.2f,0.1f,0.1f,1.0f);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		}
		SwapBuffers();  
	}
}

void Canvas::OnPaint(wxPaintEvent& evt)
{
	wxPaintDC dc(this);
}

void Canvas::SetController(CanvasController* controller) 
{
	m_controller = controller;
}
