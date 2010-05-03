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

	int *m_selected_flags;
public:
	SkeletonCanvasController(Events::EventSystem *evsys, AppContext* context);
	~SkeletonCanvasController();
	void Render(int width, int height);
	void Enter() ;
	void HandleEvent(Events::Event* ev);
};

#endif
