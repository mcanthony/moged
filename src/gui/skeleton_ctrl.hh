#ifndef INCLUDED_moged_skeleton_ctrl_HH
#define INCLUDED_moged_skeleton_ctrl_HH

#include "gui/canvas.hh"
#include "mogedevents.hh"
#include "render/gridhelper.hh"
#include "render/drawskeleton.hh"

class AppContext;

class SkeletonCanvasController : public CanvasController, public Events::EventHandler
{
	Events::EventSystem* m_evsys;
	GridHelper m_grid;
	DrawSkeletonHelper m_drawskel;
	AppContext *m_appctx;
	
public:
	SkeletonCanvasController(Events::EventSystem *evsys, AppContext* context);
	void Render(int width, int height);
	void HandleEvent(Events::Event* ev);
};

#endif
