#include <cstdio>
#include <wx/config.h>
#include "gui/mainframe.hh"
#include "gui/canvas.hh"
#include "gui/gen/mogedImportClipsDlg.h"
#include "gui/gen/mogedClipView.h"
#include "gui/gen/mogedClipControls.h"
#include "gui/gen/mogedJointWeightEditor.h"
#include "gui/gen/mogedMotionGraphEditor.h"
#include "entity.hh"
#include "appcontext.hh"
#include "util.hh"
#include "acclaim.hh"
#include "skeleton.hh"
#include "convert.hh"
#include "clipdb.hh"
#include "mesh.hh"

using namespace std;

extern int gGLattribs[];

enum {
	ID_ImportMesh,
	ID_ClearMesh,
	ID_Exit,

	ID_PlaybackMode,
	ID_SkeletonMode,
	ID_SynthesizeMode,

	ID_NewEntity,
	ID_OpenEntity,
	ID_SaveEntity,
	ID_SaveEntityAs,

	ID_SetBaseFolder,

	ID_ImportAcclaim,

	ID_ViewPlayControls,
	ID_ViewClipList, 
	ID_ViewMotionGraphControls,
	ID_ViewJointWeightEditor,

	ID_MotionGraphWizard,

	ID_Options,
};

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(ID_ImportMesh, MainFrame::OnImportMesh)
EVT_MENU(ID_ClearMesh, MainFrame::OnClearMesh)
EVT_MENU(ID_Exit, MainFrame::OnQuit)
EVT_MENU(ID_PlaybackMode, MainFrame::OnPlaybackMode)
EVT_MENU(ID_SkeletonMode, MainFrame::OnSkeletonMode)
EVT_MENU(ID_SynthesizeMode, MainFrame::OnSynthesizeMode)
EVT_MENU(ID_NewEntity, MainFrame::OnNewEntity)
EVT_MENU(ID_OpenEntity, MainFrame::OnOpenEntity)
EVT_MENU(ID_SaveEntity, MainFrame::OnSaveEntity)
EVT_MENU(ID_SetBaseFolder, MainFrame::OnSetBaseFolder)
EVT_MENU(ID_ImportAcclaim, MainFrame::OnImportAcclaim)
EVT_MENU(ID_ViewPlayControls, MainFrame::OnToggleVisibility)
EVT_MENU(ID_ViewClipList, MainFrame::OnToggleVisibility)
EVT_MENU(ID_ViewMotionGraphControls, MainFrame::OnToggleVisibility)
EVT_MENU(ID_ViewJointWeightEditor, MainFrame::OnToggleVisibility)
EVT_MENU(ID_MotionGraphWizard, MainFrame::OnMotionGraphWizard)
END_EVENT_TABLE()

void ChooseFile(const char* message, const char* startingFolder, wxString& result, const wxString& wildCard = _("*.*"), int flags = 0)
{
	result = wxFileSelector(wxString(message, wxConvUTF8),
							wxString(startingFolder, wxConvUTF8),_(""),
							_(""), wildCard, flags);
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
	fileMenu->Append( ID_NewEntity, _("New Entity") );
	fileMenu->Append( ID_OpenEntity, _("&Open Entity...") );
	fileMenu->Append( ID_SaveEntity, _("&Save Entity") );
	fileMenu->Append( ID_SaveEntityAs, _("Save Entity &As...") );
	fileMenu->AppendSeparator();

	wxMenu* importMenu = new wxMenu;
	fileMenu->AppendSubMenu(importMenu, _("&Import"));
	importMenu->Append( ID_ImportAcclaim, _("Import Acclaim Skeleton/Clips..."));
	importMenu->Append( ID_ImportMesh, _("Import Mesh...") );

	fileMenu->AppendSeparator();
	fileMenu->Append( ID_Exit, _("E&xit"));

	wxMenu* editMenu = new wxMenu;
	menuBar->Append(editMenu, _("&Edit"));
	editMenu->Append( ID_MotionGraphWizard, _("Motion Graph Wizard..."));
	editMenu->Append( ID_ClearMesh, _("Clear Mesh"));
	editMenu->AppendSeparator();
	editMenu->Append( ID_Options, _("Options..."));
	editMenu->Append( ID_SetBaseFolder, _("Set Base Folder.."));

	wxMenu* viewMenu = new wxMenu;
	menuBar->Append(viewMenu, _("&View"));
	viewMenu->AppendRadioItem( ID_PlaybackMode, _("Playback Mode"));
	viewMenu->AppendRadioItem( ID_SkeletonMode, _("Skeleton Mode"));
	viewMenu->AppendRadioItem( ID_SynthesizeMode, _("Synthesize Mode"));
	viewMenu->AppendSeparator();
	viewMenu->AppendCheckItem( ID_ViewPlayControls, _("Play Controls"));
	viewMenu->AppendCheckItem( ID_ViewClipList, _("Clip List") );
	viewMenu->AppendCheckItem( ID_ViewMotionGraphControls, _("Motion Graph Controls"));
	viewMenu->AppendCheckItem( ID_ViewJointWeightEditor, _("Joint Weight Editor"));

	m_canvas = new Canvas(this, gGLattribs, wxSUNKEN_BORDER, _("Canvas"));
	m_context = new wxGLContext(m_canvas);
	m_canvas->SetContext(m_context);
	m_canvas->SetController( m_appctx->GetCanvasController( CanvasType_Playback ) );

	m_clipview = new mogedClipView(this, m_appctx);
	m_clipcontrols = new mogedClipControls(this, m_appctx);
	m_weighteditor = new mogedJointWeightEditor(this, m_appctx);
	
	m_mgr.AddPane(m_canvas, wxAuiPaneInfo().Name(_("Canvas")).CenterPane().Dock());
	m_mgr.AddPane(m_clipview, wxAuiPaneInfo().Name(_("ClipView")).Right().Dock());
	m_mgr.AddPane(m_clipcontrols, wxAuiPaneInfo().Name(_("ClipControls")).Bottom().Dock());
	m_mgr.AddPane(m_weighteditor, wxAuiPaneInfo().Name(_("JointWeightEditor")).Right().Dock());
	m_mgr.Update();

	UpdateFancyTitle();
	InitWiring();
	
	wxString persp;
	if(m_config->Read(_("/MainFrameForm/Perspective"), &persp))
	{
		m_mgr.LoadPerspective(persp);
	}

	m_mgr.Update();

	ToggleViewMenuItem( ID_ViewClipList, m_clipview->IsShown());
	ToggleViewMenuItem( ID_ViewPlayControls, m_clipcontrols->IsShown());
	ToggleViewMenuItem( ID_ViewJointWeightEditor, m_weighteditor->IsShown());
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

void MainFrame::ToggleViewMenuItem( int id, bool shown )
{
	wxMenuItem* menuItem = GetMenuBar()->FindItem( id );
	if(menuItem) {
		menuItem->Check(shown);
	}
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
	wxString mode = _("(unknown mode)");
	if(m_canvas->GetController()) {
		mode = m_canvas->GetController()->GetName();
	} 
	wxString entityName = wxString(m_appctx->GetEntity()->GetName(), wxConvUTF8);
	wxString stupidTitle = _("moged - ");
	stupidTitle << mode << _(" - [") << entityName << _("]");
	SetTitle(stupidTitle);
}

void MainFrame::TogglePaneVisibility( wxWindow *window, bool shown )
{
	wxAuiPaneInfo& info = m_mgr.GetPane( window );
	info.Show( shown ) ;
	m_mgr.Update();
}

////////////////////////////////////////////////////////////////////////////////

void MainFrame::OnImportMesh(wxCommandEvent& event)
{
	if(m_appctx->GetEntity()->GetMesh()) {
		wxMessageDialog dlg(this, _("Are you sure you want to replace the current mesh?"), _("Confirm"), wxNO_DEFAULT|wxYES_NO|wxICON_HAND);
		if(dlg.ShowModal() != wxID_YES) {
			return;
		}	
	}

	wxString file;
	ChooseFile("Import Mesh", m_appctx->GetBaseFolder(), file, _("lab bin file (*.lbf)|*.lbf"));
	if(file.length() > 0)
	{
		Mesh * mesh = loadMesh(file.char_str());
		if(mesh)
			m_appctx->GetEntity()->SetMesh(mesh);
	}
}

void MainFrame::OnClearMesh(wxCommandEvent& event)
{
	wxMessageDialog dlg(this, _("Are you sure you want to clear the current mesh?"), _("Confirm"), wxNO_DEFAULT|wxYES_NO|wxICON_HAND);
	if(dlg.ShowModal() == wxID_YES) {
		m_appctx->GetEntity()->SetMesh(0);
	}	
}

void MainFrame::OnQuit(wxCommandEvent& event)
{
	Close(true);
}

void MainFrame::OnPlaybackMode(wxCommandEvent& event)
{
	m_canvas->SetController( m_appctx->GetCanvasController( CanvasType_Playback ) );
	UpdateFancyTitle();
}

void MainFrame::OnSkeletonMode(wxCommandEvent& event)
{
	m_canvas->SetController( m_appctx->GetCanvasController( CanvasType_Skeleton ) );
	UpdateFancyTitle();
}

void MainFrame::OnSynthesizeMode(wxCommandEvent& event)
{
	m_canvas->SetController( m_appctx->GetCanvasController( CanvasType_MotionGraph ) );
	UpdateFancyTitle();
}


void MainFrame::OnNewEntity(wxCommandEvent& event)
{
	Entity* entity = new Entity();
	m_appctx->SetEntity(entity);
	UpdateFancyTitle();
	Events::EntitySkeletonChangedEvent ev;
	m_appctx->GetEventSystem()->Send(&ev);
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

			Events::EntitySkeletonChangedEvent ev;
			m_appctx->GetEventSystem()->Send(&ev);
		} else {
			wxMessageDialog dlg(this, _("Failed to open entity file"), _("error!"), wxOK|wxICON_EXCLAMATION);
			dlg.ShowModal();
		}
	}
}

void MainFrame::OnSaveEntity(wxCommandEvent& event)
{
	std::string entityName = m_appctx->GetEntity()->GetName();
	if(entityName.empty()) {
		wxString file;
		ChooseFile( "Save Entity", m_appctx->GetBaseFolder(), file, _("lab bin file (*.lbf)|*.lbf"), wxFD_SAVE);
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

void MainFrame::OnToggleVisibility(wxCommandEvent& event)
{
	switch(event.GetId())
	{
	case ID_ViewPlayControls:
		TogglePaneVisibility(m_clipcontrols, event.GetInt() == 1 );
		break;
	case ID_ViewClipList:
		TogglePaneVisibility(m_clipview, event.GetInt() == 1 );
		break;
	case ID_ViewMotionGraphControls:
		break;
	case ID_ViewJointWeightEditor:
		TogglePaneVisibility(m_weighteditor, event.GetInt() == 1);
		break;
	default:
		break;
	}
}

void MainFrame::OnMotionGraphWizard(wxCommandEvent& event)
{
	mogedMotionGraphEditor mgEd(this, m_appctx);
	mgEd.ShowModal();
}
