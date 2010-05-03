#include <GL/gl.h>
#include "motiongraph_ctrl.hh"
#include "mogedevents.hh"

MotionGraphCanvasController::MotionGraphCanvasController(Events::EventSystem* evsys, AppContext* ctx)
	: CanvasController(_("MotionGraph"))
	, m_evsys(evsys)
	, m_grid(20.f, 0.25f)
	, m_appctx(ctx)
{
}

void MotionGraphCanvasController::Render(int width, int height)
{
	glViewport(0,0,width,height);
	glClearColor(0.1f,0.1f,0.1f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	m_camera.Draw();
	m_grid.Draw();

}

void MotionGraphCanvasController::HandleEvent(Events::Event* ev)
{
	using namespace Events;
//	if(ev->GetType() == EventID_SetSkeletonEvent) {
//		SetSkeletonEvent* sse = static_cast<SetSkeletonEvent*>(ev);
		// start rendering this skeleton
//	}
}
