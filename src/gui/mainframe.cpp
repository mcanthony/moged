#include <cstdio>
#include <wx/config.h>
#include "gui/mainframe.hh"
#include "gui/canvas.hh"
#include "gui/gen/mogedImportClipsDlg.h"
#include "gui/gen/mogedClipView.h"
#include "gui/gen/mogedClipControls.h"
#include "gui/gen/mogedJointWeightEditor.h"
#include "gui/gen/mogedMotionGraphEditor.h"
#include "gui/gen/mogedAnnotations.h"
#include "gui/gen/mogedChangeSkeleton.h"
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

	ID_OpenEntity,

	ID_SetBaseFolder,

	ID_ImportAcclaim,

	ID_ExportLBF,
	ID_ImportEntity,

	ID_ViewPlayControls,
	ID_ViewClipList, 
	ID_ViewMotionGraphControls,
	ID_ViewJointWeightEditor,
	ID_ViewAnnotations,

	ID_MotionGraphWizard,

	ID_Options,

	ID_ChangeSkeleton,
};

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(ID_ImportMesh, MainFrame::OnImportMesh)
EVT_MENU(ID_ClearMesh, MainFrame::OnClearMesh)
EVT_MENU(ID_Exit, MainFrame::OnQuit)
EVT_MENU(ID_PlaybackMode, MainFrame::OnPlaybackMode)
EVT_MENU(ID_SkeletonMode, MainFrame::OnSkeletonMode)
EVT_MENU(ID_SynthesizeMode, MainFrame::OnSynthesizeMode)
EVT_MENU(ID_OpenEntity, MainFrame::OnOpenEntity)
EVT_MENU(ID_SetBaseFolder, MainFrame::OnSetBaseFolder)
EVT_MENU(ID_ImportAcclaim, MainFrame::OnImportAcclaim)
EVT_MENU(ID_ViewPlayControls, MainFrame::OnToggleVisibility)
EVT_MENU(ID_ViewClipList, MainFrame::OnToggleVisibility)
EVT_MENU(ID_ViewMotionGraphControls, MainFrame::OnToggleVisibility)
EVT_MENU(ID_ViewJointWeightEditor, MainFrame::OnToggleVisibility)
EVT_MENU(ID_ViewAnnotations, MainFrame::OnToggleVisibility)
EVT_MENU(ID_MotionGraphWizard, MainFrame::OnMotionGraphWizard)
EVT_MENU(ID_ExportLBF, MainFrame::OnExportLBF)
EVT_MENU(ID_ImportEntity, MainFrame::OnImportEntityLBF)
EVT_MENU(ID_ChangeSkeleton, MainFrame::OnChangeSkeleton)
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
	fileMenu->Append( ID_OpenEntity, _("&Open Entity...") );
	fileMenu->AppendSeparator();

	wxMenu* importMenu = new wxMenu;
	fileMenu->AppendSubMenu(importMenu, _("&Import"));
	importMenu->Append( ID_ImportEntity, _("Import Entity (LBF)..."));
	importMenu->Append( ID_ImportAcclaim, _("Import Acclaim Skeleton/Clips (ASF/AMC)..."));
	importMenu->Append( ID_ImportMesh, _("Import Mesh (LBF)...") );
	fileMenu->AppendSeparator();

	wxMenu* exportMenu = new wxMenu;
	fileMenu->AppendSubMenu(exportMenu, _("&Export"));
	exportMenu->Append( ID_ExportLBF, _("Export LBF...") );
	fileMenu->AppendSeparator();

	fileMenu->Append( ID_Exit, _("E&xit"));

	wxMenu* editMenu = new wxMenu;
	menuBar->Append(editMenu, _("&Edit"));
	editMenu->Append( ID_ChangeSkeleton, _("Choose Skeleton..."));
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
	viewMenu->AppendCheckItem( ID_ViewAnnotations, _("Clip Annotations"));

	m_canvas = new Canvas(this, gGLattribs, wxSUNKEN_BORDER, _("Canvas"));
	m_context = new wxGLContext(m_canvas);
	m_canvas->SetContext(m_context);
	m_canvas->SetController( m_appctx->GetCanvasController( CanvasType_Playback ) );

	m_clipview = new mogedClipView(this, m_appctx);
	m_clipcontrols = new mogedClipControls(this, m_appctx);
	m_weighteditor = new mogedJointWeightEditor(this, m_appctx);
	m_annotations = new mogedAnnotations(this, m_appctx);
	
	m_mgr.AddPane(m_canvas, wxAuiPaneInfo().Name(_("Canvas")).CenterPane().Dock());
	m_mgr.AddPane(m_clipview, wxAuiPaneInfo().Name(_("ClipView")).Right().Dock());
	m_mgr.AddPane(m_clipcontrols, wxAuiPaneInfo().Name(_("ClipControls")).Bottom().Dock());
	m_mgr.AddPane(m_weighteditor, wxAuiPaneInfo().Name(_("JointWeightEditor")).Right().Dock());
	m_mgr.AddPane(m_annotations, wxAuiPaneInfo().Name(_("ClipAnnotations")).Right().Dock());
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
	ToggleViewMenuItem( ID_ViewAnnotations, m_annotations->IsShown());
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

		{ Events::EventID_EntitySkeletonChangedEvent, m_weighteditor },

		{ Events::EventID_ActiveClipEvent, m_annotations },
		{ Events::EventID_AnnotationsAddedEvent, m_annotations },

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
	wxString entityName = wxString(m_appctx->GetEntity()->GetFilename(), wxConvUTF8);
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
	if(!m_appctx->GetEntity()->HasDB()) {
		wxMessageDialog dlg(this, _("Please open an entity before importing."));
		dlg.ShowModal();
		return;
	}

	(void)event;
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
		sqlite3_int64 new_mesh_id = importMesh(file.char_str(), m_appctx->GetEntity()->GetDB(), 
											   m_appctx->GetEntity()->GetCurrentSkeleton());
		if(new_mesh_id > 0)
			m_appctx->GetEntity()->SetCurrentMesh(new_mesh_id);
	}
}

void MainFrame::OnClearMesh(wxCommandEvent& event)
{
	(void)event;
	wxMessageDialog dlg(this, _("Are you sure you want to clear the current mesh?"), _("Confirm"), wxNO_DEFAULT|wxYES_NO|wxICON_HAND);
	if(dlg.ShowModal() == wxID_YES) {
		m_appctx->GetEntity()->SetCurrentMesh(0);
	}	
}

void MainFrame::OnQuit(wxCommandEvent& event)
{
	(void)event;
	Close(true);
}

void MainFrame::OnPlaybackMode(wxCommandEvent& event)
{
	(void)event;
	m_canvas->SetController( m_appctx->GetCanvasController( CanvasType_Playback ) );
	UpdateFancyTitle();
}

void MainFrame::OnSkeletonMode(wxCommandEvent& event)
{
	(void)event;
	m_canvas->SetController( m_appctx->GetCanvasController( CanvasType_Skeleton ) );
	UpdateFancyTitle();
}

void MainFrame::OnSynthesizeMode(wxCommandEvent& event)
{
	(void)event;
	m_canvas->SetController( m_appctx->GetCanvasController( CanvasType_MotionGraph ) );
	UpdateFancyTitle();
}


void MainFrame::OnOpenEntity(wxCommandEvent& event)
{
	(void)event;
	wxString file;
	ChooseFile("Open Entity", m_appctx->GetBaseFolder(), file, _("moged db (*.moged)|*.moged"));
	if(!file.empty()) {
		m_appctx->GetEntity()->SetFilename(file.char_str());
		UpdateFancyTitle();
	}
}

void MainFrame::OnSetBaseFolder(wxCommandEvent& event)
{
	(void)event;

	wxString file ;
	ChooseFolder("Set Base Folder", m_appctx->GetBaseFolder(), file);
	if(!file.empty())
		m_appctx->SetBaseFolder(file.char_str());
}

void MainFrame::OnImportAcclaim(wxCommandEvent& event)
{
	(void)event;

	if(!m_appctx->GetEntity()->HasDB()) {
		wxMessageDialog dlg(this, _("Please open an entity before importing."));
		dlg.ShowModal();
		return;
	}

	mogedImportClipsDlg dlg(this, m_appctx);
	dlg.ShowModal();
}

void MainFrame::OnToggleVisibility(wxCommandEvent& event)
{
	(void)event;

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
	case ID_ViewAnnotations:
		TogglePaneVisibility(m_annotations, event.GetInt() == 1);
		break;
	default:
		break;
	}
}

void MainFrame::OnMotionGraphWizard(wxCommandEvent& event)
{
	(void)event;

	mogedMotionGraphEditor mgEd(this, m_appctx);
	mgEd.ShowModal();
}

void MainFrame::OnExportLBF(wxCommandEvent& event)
{
	(void)event;

	wxString file;
	ChooseFile( "Export Entity LBF", m_appctx->GetBaseFolder(), file, _("lab bin file (*.lbf)|*.lbf"), wxFD_SAVE);
	if(file.length() > 0) {
		if(!exportEntityLBF(m_appctx->GetEntity(), file.char_str()))
		{
			wxMessageDialog dlg(this, _("Failed to save entity file"), _("error!"), wxOK|wxICON_EXCLAMATION);
			dlg.ShowModal();			
		}
	}
}

void MainFrame::OnImportEntityLBF(wxCommandEvent& event)
{
	(void)event;

	if(!m_appctx->GetEntity()->HasDB()) {
		wxMessageDialog dlg(this, _("Please open an entity before importing."));
		dlg.ShowModal();
		return;
	}

	wxString file;
	ChooseFile( "Import Entity LBF", m_appctx->GetBaseFolder(), file, _("lab bin file (*.lbf)|*.lbf"));
	if(file.length() > 0) {
		
		if(!importEntityLBF(m_appctx->GetEntity(), file.char_str())) {
			wxMessageDialog dlg(this, _("Failed to open entity file"), _("error!"), wxOK|wxICON_EXCLAMATION);
			dlg.ShowModal();
		}
	}
}

void MainFrame::OnChangeSkeleton(wxCommandEvent& event)
{
	(void)event;
	mogedChangeSkeleton dlg(this, m_appctx);
	dlg.ShowModal();
}

////////////////////////////////////////////////////////////////////////////////
void MainFrame::HandleEvent( Events::Event* ev )
{
	using namespace Events;
	if(ev->GetType() == EventID_EntitySkeletonChangedEvent)
		UpdateFancyTitle();
}

