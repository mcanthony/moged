#include <cstdio>
#include <GL/gl.h>
#include "motiongraph_ctrl.hh"
#include "mogedevents.hh"

using namespace std;

static const int kMaxNumPoints = 2000;
static const float kSamplePeriod = 1/10.f;
static const float kSampleDist = 0.1f;

MotionGraphCanvasController::MotionGraphCanvasController(Events::EventSystem* evsys, AppContext* ctx)
	: CanvasController(_("MotionGraph"))
	, m_evsys(evsys)
	, m_grid(20.f, 0.25f)
	, m_appctx(ctx)
	, m_accum_time(0.f)
	, m_working_path(kMaxNumPoints)
{
	m_watch.Pause();
}

void MotionGraphCanvasController::Render(int width, int height)
{
	glViewport(0,0,width,height);
	glClearColor(0.1f,0.1f,0.1f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	m_camera.Draw();
	m_grid.Draw();

	// draw clouds if any are specified.
	glColor3f(1,0,0);
	m_cloud_a.Draw();
	glColor3f(0,0,1);
	m_cloud_b.Draw();

	glColor3f(1,1,0);
	m_working_path.Draw();

	// draw character 

}

void MotionGraphCanvasController::HandleEvent(Events::Event* ev)
{
	using namespace Events;
	if(ev->GetType() == EventID_PublishCloudDataEvent) {
		PublishCloudDataEvent* pcde = static_cast<PublishCloudDataEvent*>(ev);
		m_cloud_a.SetCloud(pcde->CloudA, pcde->SamplesPerFrame * pcde->CloudALen);
		m_cloud_b.SetCloud(pcde->CloudB, pcde->SamplesPerFrame * pcde->CloudBLen);
		if(pcde->Align) {
			m_cloud_b.SetAlignment(pcde->AlignRotation, pcde->AlignTranslation);
		} else {
			m_cloud_b.SetAlignment(0.f,Vec3(0,0,0));
		}
	}
}

void MotionGraphCanvasController::OnMouseEvent( wxMouseEvent& event ) 
{ 
	if(event.ShiftDown()) {
		EditPath(event);
	} else {
		MoveCamera(event); 
	}
}

void MotionGraphCanvasController::EditPath(wxMouseEvent& event)
{
	if(event.LeftUp()) {
		m_working_path.SmoothPath();
		m_mg_state.Reset();
		m_mg_state.SetRequestedPath(m_working_path);
		return;
	} else if( event.LeftDown()) {
		m_working_path.Clear();
		m_accum_time = kSamplePeriod + 0.01f;
	}

	if(!event.LeftIsDown()) {
		return;
	}

	float newTime = m_watch.Time();
	m_watch.Start();
	m_accum_time += newTime;

	if(m_accum_time < kSamplePeriod) {
		return;
	}

	m_accum_time = 0.f;

	float x = event.GetX();
	float y = event.GetY();
	Vec3 raydir = m_camera.GetDirectionFromScreen(x,y);
	Vec3 from = m_camera.GetPosition();
	if(Abs(raydir.y) < 1e-3f) {
		return;
	}
	float param = -from.y / raydir.y;
	if(param > 0.f) {
		Vec3 path_point = from + param * raydir;
		if(!m_working_path.Empty() && magnitude(path_point - m_working_path.Back()) < kSampleDist) {
			return;
		}
		m_working_path.AddPoint( path_point );
	}
}
