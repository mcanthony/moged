#include <cstdio>
#include <GL/gl.h>
#include "render/posehelper.hh"
#include "motiongraph_ctrl.hh"
#include "mogedevents.hh"
#include "appcontext.hh"
#include "entity.hh"
#include "mesh.hh"

using namespace std;

static const int kMaxNumPoints = 2000;
static const float kSamplePeriod = 1/60.f;
static const float kSampleDist = 0.1f;
static const float kMinTime = 1/30.f;

MotionGraphCanvasController::MotionGraphCanvasController(Events::EventSystem* evsys, AppContext* ctx)
	: CanvasController(_("MotionGraph"))
	, m_evsys(evsys)
	, m_grid(20.f, 0.25f)
	, m_appctx(ctx)
	, m_accum_time(0.f)
	, m_mg_accum_time(0.f)
	, m_working_path(kMaxNumPoints)
{
	m_watch.Pause();
	m_mg_watch.Pause();
}

void MotionGraphCanvasController::Enter() 
{
	m_drawmesh.Init();
}

void MotionGraphCanvasController::Render(int width, int height)
{
	
	glViewport(0,0,width,height);
	glClearColor(0.1f,0.1f,0.1f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);							
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);

	m_camera.Draw();
	m_grid.Draw();

	// update mg state
	long time = m_mg_watch.Time();
	m_mg_watch.Start();

	m_mg_accum_time += time / 1000.f;
	if(m_mg_accum_time > kMinTime) {
		float dt = m_mg_accum_time;
		m_mg_accum_time = 0.f;
		m_mg_state.Update(dt);
	}

	// draw clouds if any are specified.
	glColor3f(1,0,0);
	m_cloud_a.Draw();
	glColor3f(0,0,1);
	m_cloud_b.Draw();

	glColor3f(1,1,0);
	m_working_path.Draw();

	glColor3f(1,0,1);
	m_mg_state.GetCurrentPath().Draw();

	// draw character 
	const Mesh* mesh = m_appctx->GetEntity()->GetMesh();
	const Pose* pose = m_mg_state.GetPose();
	if(mesh && pose)
	{
		m_mg_state.ComputeMatrices(mesh->GetTransform());
		drawPose(m_mg_state.GetSkeleton(), pose);
		m_drawmesh.Draw(mesh, pose);
	}

	m_mg_state.DebugDraw();


	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
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
	} else if(ev->GetType() == EventID_MotionGraphChangedEvent) { // when the motion graph itself changes
		MotionGraphChangedEvent* mgce = static_cast<MotionGraphChangedEvent*>(ev);
		ResetGraph( mgce->MotionGraphID );
	} else if(ev->GetType() == EventID_EntityMotionGraphChangedEvent) { // when the current mg is switched
		EntityMotionGraphChangedEvent* emgce = static_cast<EntityMotionGraphChangedEvent*>(ev);
		ResetGraph( emgce->MotionGraphID );
	} else if(ev->GetType() == EventID_EntitySkeletonChangedEvent) {
		const MotionGraph* graph = m_appctx->GetEntity()->GetMotionGraph();
		ResetGraph( graph ? graph->GetID() : 0 );
	}
}

void MotionGraphCanvasController::ResetGraph(sqlite3_int64 graph_id)
{
	const Skeleton* skel = m_appctx->GetEntity()->GetSkeleton();
	const MotionGraph* graph = m_appctx->GetEntity()->GetMotionGraph();

	if(graph) printf("current graph id: %d\n", (int)graph->GetID());
	printf("EntityMotionGraphChangedEvent %d\n", (int)graph_id);

	if(graph && graph->GetID() == graph_id) {
		m_mg_state.SetGraph( m_appctx->GetEntity()->GetDB(), skel, graph->GetAlgorithmGraph() );
	} else {
		m_mg_state.SetGraph( m_appctx->GetEntity()->GetDB(), skel, AlgorithmMotionGraphHandle() );
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
		m_mg_state.ResetPaths();
		m_mg_state.SetRequestedPath(m_working_path);
		return;
	} else if( event.LeftDown()) {
		m_working_path.Clear();
		m_accum_time = kSamplePeriod + 0.01f;
	}

	if(!event.LeftIsDown()) {
		return;
	}

	long newTime = m_watch.Time();
	m_watch.Start();
	m_accum_time += newTime / 1000.f;

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
