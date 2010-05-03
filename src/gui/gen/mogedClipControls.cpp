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
	(void)event;
	Events::ClipPlaybackEvent ev;
	ev.PlaybackType = PlaybackEventType_RewindAll;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipControls::OnRewind( wxCommandEvent& event )
{
	(void)event;
	Events::ClipPlaybackEvent ev;
	ev.PlaybackType = PlaybackEventType_Rewind;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipControls::OnPlay( wxCommandEvent& event )
{
	(void)event;
	Events::ClipPlaybackEvent ev;
	ev.PlaybackType = PlaybackEventType_Play;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipControls::OnFwd( wxCommandEvent& event )
{
	(void)event;
	Events::ClipPlaybackEvent ev;
	ev.PlaybackType = PlaybackEventType_Fwd;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipControls::OnFwdAll( wxCommandEvent& event )
{
	(void)event;
	Events::ClipPlaybackEvent ev;
	ev.PlaybackType = PlaybackEventType_FwdAll;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipControls::OnStop( wxCommandEvent& event )
{
	(void)event;
	Events::ClipPlaybackEvent ev;
	ev.PlaybackType = PlaybackEventType_Stop;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipControls::OnScrollFrame( wxScrollEvent& event )
{
	(void)event;
	int tick = m_frame_slider->GetValue();
	
	Events::ClipPlaybackTimeEvent ev;
	ev.Time = (float)tick;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipControls::HandleEvent( Events::Event* ev )
{
	using namespace Events;
	if(ev->GetType() == EventID_PlaybackFrameInfoEvent) {
		PlaybackFrameInfoEvent* pfie = static_cast<PlaybackFrameInfoEvent*>(ev);
		SetPlaybackInfo(pfie->Frame, pfie->Playing);
	} else if(ev->GetType() == EventID_ActiveClipEvent) {
		ActiveClipEvent* ace = static_cast<ActiveClipEvent*>(ev);
		SetClip(ace->ClipPtr);
	} else if(ev->GetType() == EventID_ClipModifiedEvent) {
		ClipModifiedEvent* cme = static_cast<ClipModifiedEvent*>(ev);
		if(cme->ClipPtr == m_current_clip) 
			UpdateClipDetails();
	}
}

void mogedClipControls::SetClip( Clip* clip )
{
	m_current_clip = clip;
	UpdateClipDetails();
	if(m_current_clip)
	{
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

void mogedClipControls::SetPlaybackInfo( float frame, bool is_playing )
{
	m_cur_frame->Clear();
	(*m_cur_frame) << frame+1; // visually it is indexed from 1
	if(m_current_clip) {
		m_frame_slider->SetValue( int(frame) );
	
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

void mogedClipControls::UpdateClipDetails()
{
	m_clip_name->Clear();
	m_frame_count->Clear();
	m_clip_length->Clear();

	if(m_current_clip) {
		(*m_clip_name) << wxString(m_current_clip->GetName(), wxConvUTF8);
		(*m_frame_count) << m_current_clip->GetNumFrames();
		(*m_clip_length) << m_current_clip->GetClipTime();
		m_frame_slider->SetRange( 0, m_current_clip->GetNumFrames()-1 );
	} else {
		m_frame_slider->SetRange( 0, 1 );
	}

}

