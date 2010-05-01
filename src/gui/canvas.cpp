#include <GL/glew.h>
#include <wx/wx.h>
#include "canvas.hh"
#include "assert.hh"

BEGIN_EVENT_TABLE(Canvas, wxGLCanvas)
EVT_PAINT(Canvas::OnPaint)
EVT_MOUSE_EVENTS( Canvas::OnMouseEvent )
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
			m_controller->SetCameraSize(width, height);
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

////////////////////////////////////////////////////////////////////////////////
void Canvas::OnMouseEvent(wxMouseEvent& event)
{
	if(m_controller)
		m_controller->OnMouseEvent(event);
	
	if(event.GetId() == wxEVT_LEFT_DOWN) {
		event.Skip(); // let the window get focus
	}
}

////////////////////////////////////////////////////////////////////////////////
CanvasController::CanvasController()
	: m_wheel_rot(0)
{
	m_camera.SetTarget(0,0,0);
	m_camera.SetZoom( 5 );
	m_camera.SetPitch( -10.f );
	m_camera.SetYaw( 10.f );
}

CanvasController::~CanvasController()
{
	//TODO: persist camera controls....
}

void CanvasController::SetCameraSize(int width, int height)
{
	m_camera.SetDimensions(width,height);
}

void CanvasController::MoveCamera(wxMouseEvent& event)
{
	if(event.Entering() || event.Leaving()) {
		m_track_x.Reset();
		m_track_y.Reset();
		m_wheel_rot = 0;
	}
	m_track_x.Update( event.GetX() );
	m_track_y.Update( event.GetY() );

	if(event.Dragging())
	{
		if(event.LeftIsDown())
		{
			const float kYawRate = -0.2f;
			const float kPitchRate = -0.2f;
			m_camera.RotateYaw( m_track_x.GetRel() * kYawRate );
			m_camera.RotatePitch( m_track_y.GetRel() * kPitchRate );
		} 
		else if(event.RightIsDown())
		{
			// Move relative to the camera
			const Mat4& mat = m_camera.GetMatrix();
			Vec3 x_axis = get_column(mat, 0);
			Vec3 y_axis = get_column(mat, 1);
			
			const float kScale = 0.02f;
			Vec3 move_by = -m_track_x.GetRel() * kScale * x_axis +
				m_track_y.GetRel() * kScale * y_axis ;
			m_camera.MoveBy( move_by );
		}
	}
	else if(event.GetWheelRotation() != 0) 
	{
		m_wheel_rot += event.GetWheelRotation();
		while(m_wheel_rot > event.GetWheelDelta()) {
			m_wheel_rot -= event.GetWheelDelta();
			m_camera.ZoomBy(1.f);	 
		}
		while(m_wheel_rot < -event.GetWheelDelta()) {
			m_wheel_rot += event.GetWheelDelta();
			m_camera.ZoomBy(-1.f);
		}
	}
}
