#include <wx/wx.h>
#include <wx/config.h>
#include "appcontext.hh"
#include "gui/skeleton_ctrl.hh"
#include "gui/playback_ctrl.hh"
#include "mogedevents.hh"
#include "entity.hh"
#include "assert.hh"

AppContext::AppContext()
{
	m_run_level = 0;
	
	m_evsys = 0;
	m_base_folder = "~/moged_data/" ;// allow this to be changed with options in wxConfig
	
	m_current_entity = new Entity;
}

AppContext::~AppContext()
{
	SetRunLevel(0);

	delete m_current_entity;
}

void AppContext::Update()
{
	if(m_run_level > 0) {
		m_evsys->Update();
	}
}

void AppContext::SetRunLevel(int runlevel)
{
	int oldlevel = m_run_level;
	
	if(oldlevel < runlevel)
	{
		// increasing runlevel
		while(runlevel > m_run_level)
		{
			++m_run_level;
			switch(m_run_level)
			{
			case 1:
			{
				m_evsys = new Events::EventSystem();

				m_canvas_controllers.resize(CanvasTypeCount);
				m_canvas_controllers[CanvasType_Skeleton] = new SkeletonCanvasController(m_evsys, this);
				m_canvas_controllers[CanvasType_Playback] = new PlaybackCanvasController(m_evsys, this);

				wxString str;
				wxConfigBase* cfg = wxConfigBase::Get();
				if(cfg->Read(_("BaseFolder"), &str)) {
					m_base_folder = str.fn_str();
				}

				InitWiring();

				break;
			}
			default:
				break;
			}
		}
	}
	else
	{
		// decreasing runlevel
		while(runlevel < m_run_level)
		{
			--m_run_level;
			switch(m_run_level)
			{
			case 1:
			{
				for(int i = 0; i < (int)m_canvas_controllers.size(); ++i) {
					delete m_canvas_controllers[i];
				}
				m_canvas_controllers.clear();

				delete m_evsys;
				break;
			}
			default: 
				break;
			}
		}
	}
}

void AppContext::SetEntity(Entity* entity) 
{
	ASSERT(entity);
	if(m_current_entity) delete m_current_entity;
	m_current_entity = entity;
}

Entity* AppContext::GetEntity() 
{
	return m_current_entity;
}

CanvasController* AppContext::GetCanvasController(int type)
{
	return m_canvas_controllers[type];
}

void AppContext::InitWiring()
{
	PlaybackCanvasController *playCtrl = static_cast<PlaybackCanvasController *>(m_canvas_controllers[ CanvasType_Playback ]);
	Events::HandlerEntry handlers[] = {
		{ Events::EventID_ClipPlaybackEvent, playCtrl },
		{ Events::EventID_ClipPlaybackTimeEvent, playCtrl },
		{ Events::EventID_ActiveClipEvent, playCtrl },
		{ Events::EventID_EntitySkeletonChangedEvent, playCtrl },

		{-1,0}
	};

	m_evsys->RegisterHandlers(handlers);
}

void AppContext::SetBaseFolder(const char* folder) 
{
	wxString str(folder, wxConvUTF8);
	wxConfigBase* cfg = wxConfigBase::Get();
	cfg->Write(_("BaseFolder"), str);
	m_base_folder = folder;
}
