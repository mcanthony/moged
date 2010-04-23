#include <cstdio>
#include <GL/gl.h>
#include <GL/glu.h>
#include "mogedevents.hh"
#include "gui/skeleton_ctrl.hh"
#include "render/gridhelper.hh"
#include "render/drawskeleton.hh"
#include "appcontext.hh"
#include "entity.hh"

SkeletonCanvasController::SkeletonCanvasController(Events::EventSystem *evsys, AppContext* appContext) 
	: m_evsys(evsys)
	, m_grid(20.f, 0.25f)
	, m_appctx(appContext)
{}

void SkeletonCanvasController::Render(int width, int height)
{
	glViewport(0,0,width,height);
	glClearColor(0.1f,0.1f,0.1f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.f, 1.0f*(width/(float)height), 0.1, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(5.f, 2.f, 1.f, 0,0,0, 0,1,0);

	m_grid.Draw();
	const Skeleton* skel = m_appctx->GetEntity()->GetSkeleton();
	if(skel) 
	{
		m_drawskel.SetSkeleton(skel);
		m_drawskel.Draw();
	}
}

void SkeletonCanvasController::HandleEvent(Events::Event* ev)
{
	using namespace Events;
//	if(ev->GetType() == EventID_SetSkeletonEvent) {
//		SetSkeletonEvent* sse = static_cast<SetSkeletonEvent*>(ev);
		// start rendering this skeleton
//	}
}
	
