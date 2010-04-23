#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/aui/aui.h>
#include <wx/config.h>
#include "gui/mainframe.hh"
#include "appcontext.hh"

int gGLattribs[] = {
	WX_GL_DEPTH_SIZE, 16,
	WX_GL_RGBA, 
	WX_GL_LEVEL, 0,
	WX_GL_DOUBLEBUFFER, 
	0
};

class UpdateTimer : public wxTimer
{
	MainFrame *m_frame;
	AppContext* m_app_context;
public:
	UpdateTimer(MainFrame* mw, AppContext* context) 
		: m_frame(mw)
		, m_app_context(context)
		{}

	void Notify() 
		{
			m_frame->Update();
			m_app_context->Update();
		}       
};

class App : public wxGLApp
{
	wxConfig* m_config;
	MainFrame* m_mainframe;
	UpdateTimer* m_timer;
	AppContext* m_app_context;
public:
	App() : wxGLApp()
		{
			m_config = 0;
			m_mainframe = 0;
			m_timer = 0;
			m_app_context = 0;
		}

	bool OnInit()
		{
			if(!InitGLVisual(gGLattribs)) {
				fprintf(stderr, "Unsupported display");
				return false;
			}
			
			m_config = new wxConfig(_("moged"));
			wxConfigBase::Set(m_config);

			m_app_context = new AppContext;
			m_app_context->SetRunLevel(1);

			m_mainframe = new MainFrame( _("moged"), wxPoint(50,50), wxSize(800,600), m_config, m_app_context );
			m_mainframe->Centre();
			m_mainframe->Show();
			SetTopWindow(m_mainframe);


			m_timer = new UpdateTimer(m_mainframe, m_app_context);
			m_timer->Start(10);
			return true;
		}

	int OnExit()
		{
			m_timer->Stop();
			delete m_timer; m_timer = 0;

			m_app_context->SetRunLevel(0);		 
			delete m_app_context;

			wxConfigBase::Set(0);
			delete m_config; m_config = 0;
			return 0;
		}
};

IMPLEMENT_APP(App)

