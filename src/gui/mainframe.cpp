#include <cstdio>
#include <wx/config.h>
#include "gui/mainframe.hh"
#include "gui/canvas.hh"
#include "gui/gen/mogedImportClipsDlg.h"
#include "gui/gen/mogedClipView.h"
#include "gui/gen/mogedClipControls.h"
#include "entity.hh"
#include "appcontext.hh"
#include "util.hh"
#include "acclaim.hh"
#include "skeleton.hh"
#include "convert.hh"
#include "clipdb.hh"

using namespace std;

extern int gGLattribs[];

enum {
	ID_Exit,

	ID_PlaybackMode,
	ID_SkeletonMode,

	ID_NewEntity,
	ID_OpenEntity,
	ID_SaveEntity,

	ID_SetBaseFolder,

	ID_ImportAcclaim,
};

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(ID_Exit, MainFrame::OnQuit)
EVT_MENU(ID_PlaybackMode, MainFrame::OnPlaybackMode)
EVT_MENU(ID_SkeletonMode, MainFrame::OnSkeletonMode)
EVT_MENU(ID_NewEntity, MainFrame::OnNewEntity)
EVT_MENU(ID_OpenEntity, MainFrame::OnOpenEntity)
EVT_MENU(ID_SaveEntity, MainFrame::OnSaveEntity)
EVT_MENU(ID_SetBaseFolder, MainFrame::OnSetBaseFolder)
EVT_MENU(ID_ImportAcclaim, MainFrame::OnImportAcclaim)
END_EVENT_TABLE()

void ChooseFile(const char* message, const char* startingFolder, wxString& result, const wxString& wildCard = _("*.*"))
{
	result = wxFileSelector(wxString(message, wxConvUTF8),
							wxString(startingFolder, wxConvUTF8),_(""),
							_(""), wildCard);
}

void ChooseFolder(const char* title, const char* defaultPath, wxString& result)
{
	result = wxDirSelector(wxString(title, wxConvUTF8),
						   wxString(defaultPath, wxConvUTF8));
}

MainFrame::MainFrame( const wxString& title, const wxPoint& pos, const wxSize& size, wxConfig* config, AppContext* context )
	: wxFrame(NULL, wxID_ANY, title, pos, size)
	, m_config(config)
	, m_context(0)
	, m_canvas(0)
	, m_appctx(context)
{
	m_mgr.SetManagedWindow(this);
	int x,y,w,h;

	bool hasPos = false;
	hasPos = m_config->Read(_("/MainFrameForm/Xpos"), &x);
	hasPos = m_config->Read(_("/MainFrameForm/Ypos"), &y) && hasPos;
	hasPos = m_config->Read(_("/MainFrameForm/Width"), &w) && hasPos;
	hasPos = m_config->Read(_("/MainFrameForm/Height"), &h) && hasPos;
	
	if(hasPos)
		SetSize(x,y,w,h);

	wxMenuBar* menuBar = new wxMenuBar;
	SetMenuBar(menuBar);

	wxMenu* fileMenu = new wxMenu;
	menuBar->Append(fileMenu, _("&File"));
	fileMenu->Append( ID_SetBaseFolder, _("Set Base Folder.."));
	fileMenu->Append( ID_NewEntity, _("New Entity ") );
	fileMenu->Append( ID_OpenEntity, _("Open Entity...") );
	fileMenu->Append( ID_SaveEntity, _("Save Entity...") );
	fileMenu->Append( ID_Exit, _("E&xit"));

	wxMenu* viewMenu = new wxMenu;
	menuBar->Append(viewMenu, _("&View"));
	viewMenu->Append( ID_PlaybackMode, _("Playback Mode"));
	viewMenu->Append( ID_SkeletonMode, _("Skeleton Mode"));

	wxMenu* importMenu = new wxMenu;
	menuBar->Append(importMenu, _("Import"));
	importMenu->Append( ID_ImportAcclaim, _("Import Acclaim Skeleton/Clips..."));

	m_canvas = new Canvas(this, gGLattribs, wxSUNKEN_BORDER, _("Canvas"));
	m_context = new wxGLContext(m_canvas);
	m_canvas->SetContext(m_context);
	m_canvas->SetController( m_appctx->GetCanvasController( CanvasType_Playback ) );

	m_clipview = new mogedClipView(this, m_appctx);
	m_clipcontrols = new mogedClipControls(this, m_appctx);
	
	m_mgr.AddPane(m_canvas, wxAuiPaneInfo().Name(_("Canvas")).CenterPane().Dock());
	m_mgr.AddPane(m_clipview, wxAuiPaneInfo().Name(_("ClipView")).Right().Dock());
	m_mgr.AddPane(m_clipcontrols, wxAuiPaneInfo().Name(_("ClipControls")).Bottom().Dock());
	m_mgr.Update();

	UpdateFancyTitle();
	InitWiring();
	
	wxString persp;
	if(m_config->Read(_("/MainFrameForm/Perspective"), &persp))
	{
		m_mgr.LoadPerspective(persp);
	}

	m_mgr.Update();
}

MainFrame::~MainFrame()
{
	int w,h,x,y;
	GetSize(&w,&h);
	GetPosition(&x,&y);

	m_config->Write(_("/MainFrameForm/Xpos"), x);
	m_config->Write(_("/MainFrameForm/Ypos"), y);
	m_config->Write(_("/MainFrameForm/Width"), w);
	m_config->Write(_("/MainFrameForm/Height"), h);

	wxString perspective = m_mgr.SavePerspective();
	m_config->Write(_("/MainFrameForm/Perspective"), perspective);
	m_mgr.UnInit();

	delete m_context;
	// everything should be deleted by wx
}

void MainFrame::Update()
{
	m_canvas->Render();
}

void MainFrame::InitWiring()
{
	Events::HandlerEntry handlers[] = {
		{ Events::EventID_PlaybackFrameInfoEvent, m_clipcontrols },
		{ Events::EventID_ActiveClipEvent, m_clipcontrols },
		{ Events::EventID_ClipModifiedEvent, m_clipcontrols },
		
		{ Events::EventID_EntitySkeletonChangedEvent, m_clipview },
		{ Events::EventID_ClipAddedEvent, m_clipview },

		{-1,0}
	};

	m_appctx->GetEventSystem()->RegisterHandlers(handlers);

}

void MainFrame::UpdateFancyTitle( )
{
	wxString mode = _("some");//wxString(mode, wxConvUTF8);
	wxString entityName = wxString(m_appctx->GetEntity()->GetName(), wxConvUTF8);
	wxString stupidTitle = _("moged - ");
	stupidTitle << mode << _(" - [") << entityName << _("]");
	SetTitle(stupidTitle);
}

////////////////////////////////////////////////////////////////////////////////

void MainFrame::OnQuit(wxCommandEvent& event)
{
	Close(true);
}

void MainFrame::OnPlaybackMode(wxCommandEvent& event)
{
	m_canvas->SetController( m_appctx->GetCanvasController( CanvasType_Playback ) );
}

void MainFrame::OnSkeletonMode(wxCommandEvent& event)
{
	m_canvas->SetController( m_appctx->GetCanvasController( CanvasType_Skeleton ) );
}

void MainFrame::OnNewEntity(wxCommandEvent& event)
{
	Entity* entity = new Entity();
	m_appctx->SetEntity(entity);
	UpdateFancyTitle();
}

void MainFrame::OnOpenEntity(wxCommandEvent& event)
{
	wxString file;
	ChooseFile( "Open Entity", m_appctx->GetBaseFolder(), file, _("lab bin file (*.lbf)|*.lbf"));
	if(file.length() > 0) {
		std::string str = (char*)file.char_str();
		Entity* entity = loadEntity(str.c_str());
		if(entity) {
			m_appctx->SetEntity(entity);
			UpdateFancyTitle();
		} else {
			// TODO: FAILCAKE
		}
	}
}

void MainFrame::OnSaveEntity(wxCommandEvent& event)
{
	std::string entityName = m_appctx->GetEntity()->GetName();
	if(entityName.empty()) {
		wxString file;
		ChooseFile( "Save Entity", m_appctx->GetBaseFolder(), file, _("lab bin file (*.lbf)|*.lbf"));
		if(file.length() > 0) {
			entityName = (char*)file.char_str();
			m_appctx->GetEntity()->SetName(entityName.c_str());
			UpdateFancyTitle();
		} else {
			return;
		}
	}

	saveEntity(m_appctx->GetEntity());
}

void MainFrame::OnSetBaseFolder(wxCommandEvent& event)
{
	wxString file ;
	ChooseFolder("Set Base Folder", m_appctx->GetBaseFolder(), file);
	if(!file.empty())
	{
		wxWritableCharBuffer temp = file.char_str();
		m_appctx->SetBaseFolder(temp);
	}
}

void MainFrame::OnImportAcclaim(wxCommandEvent& event)
{
	mogedImportClipsDlg dlg(this, m_appctx);
	dlg.ShowModal();
}

