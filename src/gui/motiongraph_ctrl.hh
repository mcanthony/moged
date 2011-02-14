#ifndef INCLUDED_moged_motiongraph_ctrl_HH
#define INCLUDED_moged_motiongraph_ctrl_HH

#include <vector>
#include <wx/stopwatch.h>
#include "gui/canvas.hh"
#include "mogedevents.hh"
#include "render/gridhelper.hh"
#include "render/cloudhelper.hh"
#include "render/meshhelper.hh"
#include "Vector.hh"
#include "anim/mgcontroller.hh"

class AppContext;

class MotionGraphCanvasController : public CanvasController, public Events::EventHandler
{
	Events::EventSystem* m_evsys;
	GridHelper m_grid;
	AppContext *m_appctx;

	CloudHelper m_cloud_a;
	CloudHelper m_cloud_b;
	MeshHelper m_drawmesh;

	wxStopWatch m_watch;
	wxStopWatch m_mg_watch;
	float m_accum_time;
	float m_mg_accum_time;

	MGPath m_working_path;

    MotionGraphController *m_mgController;

public:
	MotionGraphCanvasController(Events::EventSystem *evsys, AppContext* context);
	void Enter() ;
	void Render(int width, int height);
	void HandleEvent(Events::Event* ev);
	void OnMouseEvent( wxMouseEvent& event ) ;

private:
	void ResetGraph(sqlite3_int64 graph_id);
	void EditPath(wxMouseEvent& event);
};

#endif
