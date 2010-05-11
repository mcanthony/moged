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
{
	m_watch.Pause();
	m_path_points.reserve(kMaxNumPoints);
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

	// draw current path
	glBegin(GL_LINE_STRIP);
	glColor3f(1,1,0);
	const int num_points = m_path_points.size();
	for(int i = 0; i < num_points; ++i) {
		glVertex3fv(&m_path_points[i].x);
	}
	glEnd();
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
		DrawPath(event);
	}
	else {
		MoveCamera(event); 
	}
}

void MotionGraphCanvasController::DrawPath(wxMouseEvent& event)
{
	if(event.LeftUp()) {
		SmoothPath();
		return;
	} else if( event.LeftDown()) {
		m_path_points.clear();
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

	if((int)m_path_points.size() > kMaxNumPoints) {
		return;
	}

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
		if(!m_path_points.empty() &&
		   magnitude(path_point - m_path_points.back()) < kSampleDist) {
			return;
		}
		m_path_points.push_back(path_point);
	}
}

void MotionGraphCanvasController::SmoothPath()
{
	static const float kSmoothFilter[] = {0.0674508058663448, 0.183350299901404, 0.498397788464502, 0.183350299901404, 0.0674508058663448};
	static const int kSmoothLen = sizeof(kSmoothFilter)/sizeof(float);

	const int num_points = m_path_points.size();
	std::vector< Vec3 > result_points( num_points );
	for(int i = 0; i < num_points; ++i) {
		Vec3 smoothed(0,0,0);
		for(int j = 0; j < kSmoothLen; ++j) {
			int idx = Clamp( i - kSmoothLen/2 + j, 0, num_points-1 );
			smoothed += kSmoothFilter[j] * m_path_points[idx] ;
		}
		result_points[i] = smoothed;
	}
	m_path_points = result_points;
}
