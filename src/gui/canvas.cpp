#include "canvas.hh"
#include "assert.hh"

Canvas::Canvas(wxWindow* parent, int *attribList, long style, const wxString& name)
	: wxGLCanvas(parent, wxID_ANY, attribList, wxDefaultPosition, wxDefaultSize, style, name)
	, m_context(0)
	, m_controller(0)
{
}

void Canvas::Render()
{
	if(m_context && IsShownOnScreen())
	{
		int width, height;
		GetClientSize(&width, &height);
		SetCurrent(*m_context);                 
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
