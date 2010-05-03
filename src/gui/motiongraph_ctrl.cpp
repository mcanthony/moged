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

	glColor3f(1,0,0);
	m_cloud_a.Draw();
	glColor3f(0,0,1);
	m_cloud_b.Draw();
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
