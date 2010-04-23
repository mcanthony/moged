#include <cstdio>
#include <GL/gl.h>
#include <GL/glu.h>
#include "mogedevents.hh"
#include "gui/playback_ctrl.hh"
#include "render/gridhelper.hh"
#include "render/drawskeleton.hh"
#include "appcontext.hh"
#include "entity.hh"
#include "MathUtil.hh"
#include "assert.hh"
#include "playback_enum.hh"
#include "clip.hh"

PlaybackCanvasController::PlaybackCanvasController(Events::EventSystem *evsys, AppContext* appContext) 
	: m_evsys(evsys)
	, m_grid(20.f, 0.25f)
	, m_appctx(appContext)
	, m_current_clip(0)
	, m_frame(0)
{}

void PlaybackCanvasController::Render(int width, int height)
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
	if(skel && m_current_clip) 
	{
		m_drawskel.SetSkeleton(skel);
		m_drawskel.Draw();

		// advance frame, twiddle with m_playing and time

		// Update the world
		Events::PlaybackFrameInfoEvent ev;
		ev.Frame = m_frame;
		ev.Playing = m_playing;
		m_appctx->GetEventSystem()->Send(&ev);
	}
}

void PlaybackCanvasController::HandleEvent(Events::Event* ev)
{
	using namespace Events;
	if(ev->GetType() == EventID_ClipPlaybackEvent) {
		ClipPlaybackEvent* cpe = static_cast<ClipPlaybackEvent*>(ev);
		HandlePlaybackCommand(cpe->PlaybackType);
	}
	else if(ev->GetType() == EventID_ClipPlaybackTimeEvent) {
		ClipPlaybackTimeEvent* cpte = static_cast<ClipPlaybackTimeEvent*>(ev);
		SetTime( cpte->Time );
	}
	else if(ev->GetType() == EventID_ActiveClipEvent) {
		ActiveClipEvent* ace = static_cast<ActiveClipEvent*>(ev);
		SetClip(ace->ClipPtr);
	} 
	else if(ev->GetType() == EventID_EntitySkeletonChangedEvent) {
		SetClip(0);
	}
}

void PlaybackCanvasController::SetClip(Clip* clip)
{
	m_current_clip = clip;
	SetTime(0.f);
}
	
void PlaybackCanvasController::SetTime(float t)
{
	if(m_current_clip) {
		m_frame = Clamp(t, 0.f, float(m_current_clip->GetNumFrames()) );
	} else 
		m_frame = 0.f;	
}

void PlaybackCanvasController::HandlePlaybackCommand(int type)
{
	switch(type)
	{
	case PlaybackEventType_Play:
		m_playing = true;
		break;
	case PlaybackEventType_Stop:
		m_playing = false;
		break;
	case PlaybackEventType_Fwd:
		if(m_current_clip)
		{
			float newTime = floor(m_frame+1.f);
			SetTime(newTime);
			break;
		}
	case PlaybackEventType_FwdAll:
		if(m_current_clip) 
			SetTime( m_current_clip->GetNumFrames() );
		break;
	case PlaybackEventType_Rewind:
		if(m_current_clip)
		{
			float newTime = floor(m_frame-1.f);
			SetTime(newTime);
			break;
		}
	case PlaybackEventType_RewindAll:
		SetTime(0.f);
		break;
	default: ASSERT(false); break;
	}
}
