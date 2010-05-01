#ifndef INCLUDED_moged_canvas_HH
#define INCLUDED_moged_canvas_HH

#include <wx/glcanvas.h>
#include "render/editcam.hh"
#include "reltrack.hh"

class CanvasController {
	wxString m_name;
public:
	CanvasController(wxString const& name);
	virtual ~CanvasController() ; // will be deleting from base type ptr

	virtual void Enter() {}
	virtual void Exit() {}
	virtual void Render(int width, int height) = 0 ; 
	virtual void OnMouseEvent( wxMouseEvent& event ) { MoveCamera(event); }

	const wxString& GetName( ) const { return m_name; }
	void SetCameraSize(int width, int height);
protected:
	RelativeTracker m_track_x;
	RelativeTracker m_track_y;
	int m_wheel_rot;

	EditorCamera m_camera;
	void MoveCamera(wxMouseEvent& event);
};

class Canvas : public wxGLCanvas
{
	wxGLContext* m_context;
	CanvasController* m_controller;
	CanvasController* m_last_controller; // bleh, used for lazy init crap
	bool m_lazy_init; // oh, wxWidgets - you do not make life easy.
public:
	Canvas(wxWindow* parent, int *attribList, long style, const wxString& name);

	CanvasController* GetController() const { return m_controller; }
	void SetController(CanvasController* controller) ;
	void SetContext(wxGLContext* context) { m_context = context; }

	void Render() ;
protected:
	void OnSize(wxSizeEvent& evt);
	void OnPaint(wxPaintEvent& evt);
	void OnMouseEvent(wxMouseEvent& event);

private:
	DECLARE_EVENT_TABLE()
};


#endif
