///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Dec 29 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "MogedFrames.h"

///////////////////////////////////////////////////////////////////////////

ImportClipsDlg::ImportClipsDlg( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer2->AddGrowableCol( 0 );
	fgSizer2->AddGrowableRow( 1 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("ASF Skeleton") ), wxHORIZONTAL );
	
	m_staticText6 = new wxStaticText( this, wxID_ANY, wxT("Choose ASF File"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	sbSizer4->Add( m_staticText6, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );
	
	m_asf_file = new wxFilePickerCtrl( this, wxID_ANY, wxEmptyString, wxT("Open ASF File"), wxT("*.*"), wxDefaultPosition, wxSize( -1,-1 ), wxFLP_DEFAULT_STYLE|wxFLP_USE_TEXTCTRL );
	bSizer6->Add( m_asf_file, 0, wxALL|wxEXPAND, 5 );
	
	sbSizer4->Add( bSizer6, 1, wxEXPAND, 5 );
	
	bSizer13->Add( sbSizer4, 0, wxALL|wxEXPAND, 5 );
	
	fgSizer2->Add( bSizer13, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("AMC Clips to Add") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 1, 1, 0, 0 );
	fgSizer3->AddGrowableCol( 0 );
	fgSizer3->AddGrowableRow( 0 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_clip_list = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxLC_LIST );
	fgSizer3->Add( m_clip_list, 0, wxALL|wxEXPAND, 5 );
	
	sbSizer5->Add( fgSizer3, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxHORIZONTAL );
	
	m_add_button = new wxButton( this, wxID_ANY, wxT("Add Clips..."), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer15->Add( m_add_button, 0, wxALL, 5 );
	
	m_remove_button = new wxButton( this, wxID_ANY, wxT("Remove Clips"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer15->Add( m_remove_button, 0, wxALL, 5 );
	
	sbSizer5->Add( bSizer15, 0, wxEXPAND, 5 );
	
	bSizer12->Add( sbSizer5, 1, wxALL|wxEXPAND, 5 );
	
	fgSizer2->Add( bSizer12, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer41;
	sbSizer41 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Options") ), wxVERTICAL );
	
	wxBoxSizer* bSizer121;
	bSizer121 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText2 = new wxStaticText( this, wxID_ANY, wxT("Clip sample rate (FPS)"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_staticText2->Wrap( -1 );
	bSizer121->Add( m_staticText2, 0, wxALL, 5 );
	
	m_fps = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer121->Add( m_fps, 0, wxALL, 5 );
	
	sbSizer41->Add( bSizer121, 1, wxEXPAND, 5 );
	
	bSizer11->Add( sbSizer41, 1, wxALL|wxEXPAND, 5 );
	
	fgSizer2->Add( bSizer11, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer6;
	sbSizer6 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Import") ), wxHORIZONTAL );
	
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );
	
	m_report = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	bSizer10->Add( m_report, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxVERTICAL );
	
	m_gauge = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	bSizer7->Add( m_gauge, 0, wxALL|wxEXPAND, 5 );
	
	bSizer10->Add( bSizer7, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxVERTICAL );
	
	m_cancel_button = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer17->Add( m_cancel_button, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	bSizer8->Add( bSizer17, 1, wxEXPAND, 5 );
	
	bSizer9->Add( bSizer8, 1, wxEXPAND, 5 );
	
	m_import_button = new wxButton( this, wxID_ANY, wxT("Import"), wxDefaultPosition, wxDefaultSize, 0 );
	m_import_button->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );
	
	bSizer9->Add( m_import_button, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	m_done = new wxButton( this, wxID_ANY, wxT("Done"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer9->Add( m_done, 0, wxALL, 5 );
	
	bSizer10->Add( bSizer9, 1, wxEXPAND, 5 );
	
	sbSizer6->Add( bSizer10, 1, wxEXPAND, 5 );
	
	bSizer16->Add( sbSizer6, 1, wxALL|wxEXPAND, 5 );
	
	fgSizer2->Add( bSizer16, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer2 );
	this->Layout();
	
	// Connect Events
	m_add_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ImportClipsDlg::OnAddClips ), NULL, this );
	m_remove_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ImportClipsDlg::OnRemoveClips ), NULL, this );
	m_cancel_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ImportClipsDlg::OnCancel ), NULL, this );
	m_import_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ImportClipsDlg::OnImport ), NULL, this );
	m_done->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ImportClipsDlg::OnDone ), NULL, this );
}

ImportClipsDlg::~ImportClipsDlg()
{
	// Disconnect Events
	m_add_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ImportClipsDlg::OnAddClips ), NULL, this );
	m_remove_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ImportClipsDlg::OnRemoveClips ), NULL, this );
	m_cancel_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ImportClipsDlg::OnCancel ), NULL, this );
	m_import_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ImportClipsDlg::OnImport ), NULL, this );
	m_done->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ImportClipsDlg::OnDone ), NULL, this );
}

ClipView::ClipView( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer3->AddGrowableCol( 0 );
	fgSizer3->AddGrowableRow( 0 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Clips") ), wxVERTICAL );
	
	m_clips = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_ICON|wxRAISED_BORDER );
	sbSizer5->Add( m_clips, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxHORIZONTAL );
	
	m_delete = new wxButton( this, wxID_ANY, wxT("Remove Selected"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer13->Add( m_delete, 0, wxALL, 5 );
	
	sbSizer5->Add( bSizer13, 0, wxEXPAND, 5 );
	
	fgSizer3->Add( sbSizer5, 1, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( fgSizer3 );
	this->Layout();
}

ClipView::~ClipView()
{
}
