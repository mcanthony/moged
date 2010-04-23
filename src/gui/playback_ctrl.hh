#ifndef INCLUDED_moged_playback_ctrl_HH
#define INCLUDED_moged_playback_ctrl_HH

#include "gui/canvas.hh"
#include "mogedevents.hh"
#include "render/gridhelper.hh"
#include "render/drawskeleton.hh"
class AppContext;

class PlaybackCanvasController : public CanvasController, public Events::EventHandler
{
	Events::EventSystem* m_evsys;
	GridHelper m_grid;
	DrawSkeletonHelper m_drawskel;
	AppContext *m_appctx;

	Clip *m_current_clip;
	float m_time ;

	bool m_playing;
public:
	PlaybackCanvasController(Events::EventSystem *evsys, AppContext* context);
	void Render(int width, int height);

	void HandleEvent(Events::Event* ev);

private:
	void SetClip(Clip *clip);
	void SetTime(float t);
	void HandlePlaybackCommand(int type);
	int GetCurrentFrame();

};

#endif
