#ifndef INCLUDED_moged_canvas_HH
#define INCLUDED_moged_canvas_HH

#include <wx/glcanvas.h>

class CanvasController {
public:
	virtual ~CanvasController() {} // will be deleting from base type ptr
	virtual void Render(int width, int height) = 0 ; 
};

class Canvas : public wxGLCanvas
{
	wxGLContext* m_context;
	CanvasController* m_controller;
public:
	Canvas(wxWindow* parent, int *attribList, long style, const wxString& name);

	inline CanvasController* GetController() const { return m_controller; }
	inline void SetController(CanvasController* controller) { m_controller = controller; }
	inline void SetContext(wxGLContext* context) { m_context = context; }

	void Render() ;
};


#endif
