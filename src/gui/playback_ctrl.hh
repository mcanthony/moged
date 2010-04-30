#ifndef INCLUDED_moged_playback_ctrl_HH
#define INCLUDED_moged_playback_ctrl_HH

#include <wx/stopwatch.h>
#include "gui/canvas.hh"
#include "mogedevents.hh"
#include "render/gridhelper.hh"
#include "render/meshhelper.hh"

class AppContext;
class Pose;
class ClipController;

class PlaybackCanvasController : public CanvasController, public Events::EventHandler
{
	Events::EventSystem* m_evsys;
	GridHelper m_grid;
	MeshHelper m_drawmesh;
	AppContext *m_appctx;

	bool m_playing;
	wxStopWatch m_watch;
	float m_accum_time ;

	Pose *m_current_pose;
	ClipController *m_anim_controller;
public:
	PlaybackCanvasController(Events::EventSystem *evsys, AppContext* context);
	~PlaybackCanvasController();

	void Enter();
	void Render(int width, int height);

	void HandleEvent(Events::Event* ev);

private:
	void SetClip(Clip *clip);
	void ResetPose();
	void HandlePlaybackCommand(int type);

};

#endif
