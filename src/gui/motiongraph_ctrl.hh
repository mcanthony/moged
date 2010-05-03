#ifndef INCLUDED_moged_motiongraph_ctrl_HH
#define INCLUDED_moged_motiongraph_ctrl_HH

#include "gui/canvas.hh"
#include "mogedevents.hh"
#include "render/gridhelper.hh"

class AppContext;

class MotionGraphCanvasController : public CanvasController, public Events::EventHandler
{
	Events::EventSystem* m_evsys;
	GridHelper m_grid;
	AppContext *m_appctx;
	
public:
	MotionGraphCanvasController(Events::EventSystem *evsys, AppContext* context);
	void Render(int width, int height);
	void HandleEvent(Events::Event* ev);
};

#endif
