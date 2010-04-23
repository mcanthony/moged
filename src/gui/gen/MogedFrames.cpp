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

ClipControls::ClipControls( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxStaticBoxSizer* sbSizer8;
	sbSizer8 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Clip Controls") ), wxVERTICAL );
	
	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText6 = new wxStaticText( this, wxID_ANY, wxT("Clip Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	bSizer18->Add( m_staticText6, 0, wxALL, 5 );
	
	m_clip_name = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer18->Add( m_clip_name, 1, wxEXPAND, 5 );
	
	bSizer16->Add( bSizer18, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxHORIZONTAL );
	
	m_rewind = new wxButton( this, wxID_ANY, wxT("|<"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer15->Add( m_rewind, 0, wxALL, 5 );
	
	m_step_back = new wxButton( this, wxID_ANY, wxT("<"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer15->Add( m_step_back, 0, wxALL, 5 );
	
	m_play = new wxButton( this, wxID_ANY, wxT("Play"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer15->Add( m_play, 0, wxALL, 5 );
	
	m_step_fwd = new wxButton( this, wxID_ANY, wxT(">"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer15->Add( m_step_fwd, 0, wxALL, 5 );
	
	m_jump_end = new wxButton( this, wxID_ANY, wxT(">|"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer15->Add( m_jump_end, 0, wxALL, 5 );
	
	m_stop = new wxButton( this, wxID_ANY, wxT("Stop"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer15->Add( m_stop, 0, wxALL, 5 );
	
	bSizer16->Add( bSizer15, 0, wxEXPAND, 5 );
	
	m_frame_slider = new wxSlider( this, wxID_ANY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_AUTOTICKS|wxSL_HORIZONTAL );
	bSizer16->Add( m_frame_slider, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText3 = new wxStaticText( this, wxID_ANY, wxT("Current Frame:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	bSizer17->Add( m_staticText3, 0, wxALL, 5 );
	
	m_cur_frame = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer17->Add( m_cur_frame, 0, wxALL|wxSHAPED, 5 );
	
	m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("Number of Frames:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	bSizer17->Add( m_staticText4, 0, wxALL, 5 );
	
	m_frame_count = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer17->Add( m_frame_count, 0, wxALL|wxSHAPED, 5 );
	
	m_staticText5 = new wxStaticText( this, wxID_ANY, wxT("Clip length (s):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5->Wrap( -1 );
	bSizer17->Add( m_staticText5, 0, wxALL, 5 );
	
	m_clip_length = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer17->Add( m_clip_length, 0, wxALL|wxSHAPED, 5 );
	
	bSizer16->Add( bSizer17, 1, wxEXPAND, 5 );
	
	sbSizer8->Add( bSizer16, 1, wxEXPAND, 5 );
	
	this->SetSizer( sbSizer8 );
	this->Layout();
	
	// Connect Events
	m_rewind->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ClipControls::OnRewindAll ), NULL, this );
	m_step_back->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ClipControls::OnRewind ), NULL, this );
	m_play->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ClipControls::OnPlay ), NULL, this );
	m_step_fwd->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ClipControls::OnFwd ), NULL, this );
	m_jump_end->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ClipControls::OnFwdAll ), NULL, this );
	m_stop->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ClipControls::OnStop ), NULL, this );
	m_frame_slider->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
	m_frame_slider->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
	m_frame_slider->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
	m_frame_slider->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
	m_frame_slider->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
	m_frame_slider->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
	m_frame_slider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
	m_frame_slider->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
	m_frame_slider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
}

ClipControls::~ClipControls()
{
	// Disconnect Events
	m_rewind->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ClipControls::OnRewindAll ), NULL, this );
	m_step_back->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ClipControls::OnRewind ), NULL, this );
	m_play->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ClipControls::OnPlay ), NULL, this );
	m_step_fwd->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ClipControls::OnFwd ), NULL, this );
	m_jump_end->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ClipControls::OnFwdAll ), NULL, this );
	m_stop->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ClipControls::OnStop ), NULL, this );
	m_frame_slider->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
	m_frame_slider->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
	m_frame_slider->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
	m_frame_slider->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
	m_frame_slider->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
	m_frame_slider->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
	m_frame_slider->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
	m_frame_slider->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
	m_frame_slider->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( ClipControls::OnScrollFrame ), NULL, this );
}
