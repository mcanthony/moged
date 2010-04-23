#include "mogedClipControls.h"
#include "playback_enum.hh"
#include "clip.hh"
#include "appcontext.hh"

static const float kSliderResolution = 4.0f;

mogedClipControls::mogedClipControls( wxWindow* parent , AppContext *ctx)
:
ClipControls( parent )
, m_ctx(ctx)
, m_current_clip(0)
{
	SetClip(0);
}

void mogedClipControls::OnRewindAll( wxCommandEvent& event )
{
	Events::ClipPlaybackEvent ev;
	ev.PlaybackType = PlaybackEventType_RewindAll;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipControls::OnRewind( wxCommandEvent& event )
{
	Events::ClipPlaybackEvent ev;
	ev.PlaybackType = PlaybackEventType_Rewind;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipControls::OnPlay( wxCommandEvent& event )
{
	Events::ClipPlaybackEvent ev;
	ev.PlaybackType = PlaybackEventType_Play;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipControls::OnFwd( wxCommandEvent& event )
{
	Events::ClipPlaybackEvent ev;
	ev.PlaybackType = PlaybackEventType_Fwd;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipControls::OnFwdAll( wxCommandEvent& event )
{
	Events::ClipPlaybackEvent ev;
	ev.PlaybackType = PlaybackEventType_FwdAll;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipControls::OnStop( wxCommandEvent& event )
{
	Events::ClipPlaybackEvent ev;
	ev.PlaybackType = PlaybackEventType_Stop;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipControls::OnScrollFrame( wxScrollEvent& event )
{
	int tick = m_frame_slider->GetValue();
	
	Events::ClipPlaybackTimeEvent ev;
	ev.Time = float(tick)/kSliderResolution;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipControls::HandleEvent( Events::Event* ev )
{
	using namespace Events;
	if(ev->GetType() == EventID_PlaybackFrameInfoEvent) {
		PlaybackFrameInfoEvent* pfie = static_cast<PlaybackFrameInfoEvent*>(ev);
		SetPlaybackInfo(pfie->Time, pfie->Playing);
	} else if(ev->GetType() == EventID_ActiveClipEvent) {
		ActiveClipEvent* ace = static_cast<ActiveClipEvent*>(ev);
		SetClip(ace->ClipPtr);
	}
}

void mogedClipControls::SetClip( Clip* clip )
{
	m_current_clip = clip;
	m_clip_name->Clear();
	m_frame_count->Clear();
	m_clip_length->Clear();
	if(m_current_clip)
	{
		(*m_clip_name) << wxString(clip->GetName(), wxConvUTF8);
		(*m_frame_count) << clip->GetNumFrames();
		(*m_clip_length) << clip->GetClipTime();

		m_frame_slider->SetRange( 0, int(clip->GetClipTime() * kSliderResolution));

		m_rewind->Enable();
		m_step_back->Enable();
		m_play->Enable();
		m_step_fwd->Enable();
		m_jump_end->Enable();
		m_stop->Enable();
		m_frame_slider->Enable();
	}
	else
	{
		m_frame_slider->SetRange(0, 1);
		m_rewind->Disable();
		m_step_back->Disable();
		m_play->Disable();
		m_step_fwd->Disable();
		m_jump_end->Disable();
		m_stop->Disable();
		m_frame_slider->Disable();
	}
	SetPlaybackInfo(0.f, false);	
}

void mogedClipControls::SetPlaybackInfo( float time, bool is_playing )
{
	m_cur_frame->Clear();
	(*m_cur_frame) << time;
	if(m_current_clip) {
		m_frame_slider->SetValue(int(time*kSliderResolution));
	
		if(is_playing)
		{
			m_play->Disable();
			m_stop->Enable();
		}
		else 
		{
			m_play->Enable();
			m_stop->Disable();
		}
	} else {
		m_frame_slider->SetValue(0);
	}

}

