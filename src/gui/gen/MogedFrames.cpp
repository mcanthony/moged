///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Dec 21 2009)
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
	
	m_clip_list = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxLC_LIST|wxRAISED_BORDER );
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
	
	wxBoxSizer* bSizer49;
	bSizer49 = new wxBoxSizer( wxHORIZONTAL );
	
	m_btn_clear_sel = new wxButton( this, wxID_ANY, wxT("Clear Selection"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer49->Add( m_btn_clear_sel, 0, wxALL, 5 );
	
	sbSizer5->Add( bSizer49, 0, wxEXPAND, 5 );
	
	m_clips = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_EDIT_LABELS|wxLC_LIST|wxRAISED_BORDER );
	sbSizer5->Add( m_clips, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxHORIZONTAL );
	
	m_delete = new wxButton( this, wxID_ANY, wxT("Remove Selected"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer13->Add( m_delete, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer48;
	bSizer48 = new wxBoxSizer( wxVERTICAL );
	
	m_check_transitions = new wxCheckBox( this, wxID_ANY, wxT("Show Transitions"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer48->Add( m_check_transitions, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_check_originals = new wxCheckBox( this, wxID_ANY, wxT("Show Originals"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_originals->SetValue(true); 
	bSizer48->Add( m_check_originals, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer13->Add( bSizer48, 1, wxEXPAND, 5 );
	
	sbSizer5->Add( bSizer13, 0, wxEXPAND, 5 );
	
	fgSizer3->Add( sbSizer5, 1, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( fgSizer3 );
	this->Layout();
	
	// Connect Events
	m_btn_clear_sel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ClipView::OnClearSelection ), NULL, this );
	m_clips->Connect( wxEVT_COMMAND_LIST_END_LABEL_EDIT, wxListEventHandler( ClipView::OnRenameClip ), NULL, this );
	m_clips->Connect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler( ClipView::OnActivateClip ), NULL, this );
	m_clips->Connect( wxEVT_RIGHT_UP, wxMouseEventHandler( ClipView::OnRightClick ), NULL, this );
	m_delete->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ClipView::OnDelete ), NULL, this );
	m_check_transitions->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ClipView::OnShowTransitions ), NULL, this );
	m_check_originals->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ClipView::OnShowOriginals ), NULL, this );
}

ClipView::~ClipView()
{
	// Disconnect Events
	m_btn_clear_sel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ClipView::OnClearSelection ), NULL, this );
	m_clips->Disconnect( wxEVT_COMMAND_LIST_END_LABEL_EDIT, wxListEventHandler( ClipView::OnRenameClip ), NULL, this );
	m_clips->Disconnect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler( ClipView::OnActivateClip ), NULL, this );
	m_clips->Disconnect( wxEVT_RIGHT_UP, wxMouseEventHandler( ClipView::OnRightClick ), NULL, this );
	m_delete->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ClipView::OnDelete ), NULL, this );
	m_check_transitions->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ClipView::OnShowTransitions ), NULL, this );
	m_check_originals->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( ClipView::OnShowOriginals ), NULL, this );
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

MotionGraphEditor::MotionGraphEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxVERTICAL );
	
	m_listbook = new wxListbook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_DEFAULT|wxLB_TOP|wxSUNKEN_BORDER );
	m_transition_panel = new wxPanel( m_listbook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL, wxT("Transitions") );
	wxBoxSizer* bSizer37;
	bSizer37 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 1, 1, 0, 0 );
	fgSizer6->AddGrowableCol( 0 );
	fgSizer6->AddGrowableRow( 0 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer58;
	bSizer58 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText201 = new wxStaticText( m_transition_panel, wxID_ANY, wxT("Motion Graph Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText201->Wrap( -1 );
	bSizer58->Add( m_staticText201, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_mg_name = new wxTextCtrl( m_transition_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer58->Add( m_mg_name, 1, wxALL, 5 );
	
	bSizer18->Add( bSizer58, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer9;
	sbSizer9 = new wxStaticBoxSizer( new wxStaticBox( m_transition_panel, wxID_ANY, wxT("Transition Properties") ), wxVERTICAL );
	
	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText7 = new wxStaticText( m_transition_panel, wxID_ANY, wxT("Error Threshold:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText7->Wrap( -1 );
	bSizer20->Add( m_staticText7, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_error_slider = new wxSlider( m_transition_panel, wxID_ANY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	m_error_slider->SetToolTip( wxT("Maximum error allowed to consider a transition.") );
	
	bSizer20->Add( m_error_slider, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_error_value = new wxTextCtrl( m_transition_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	m_error_value->SetToolTip( wxT("Maximum error allowed to consider a transition.") );
	
	bSizer20->Add( m_error_value, 0, wxALL|wxSHAPED|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer19->Add( bSizer20, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer44;
	bSizer44 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText19 = new wxStaticText( m_transition_panel, wxID_ANY, wxT("Sample Rate (FPS):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText19->Wrap( -1 );
	bSizer44->Add( m_staticText19, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_fps_sample_rate = new wxTextCtrl( m_transition_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_fps_sample_rate->SetToolTip( wxT("Rate to sample clips at. This allows the clip DB to consist of clips of varying frame rates.") );
	
	bSizer44->Add( m_fps_sample_rate, 0, wxALL|wxSHAPED|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText21 = new wxStaticText( m_transition_panel, wxID_ANY, wxT("OMP Threads:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText21->Wrap( -1 );
	bSizer44->Add( m_staticText21, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_num_threads = new wxTextCtrl( m_transition_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_num_threads->SetToolTip( wxT("Number of OpenMP threads to use.") );
	
	bSizer44->Add( m_num_threads, 0, wxALL|wxSHAPED|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer19->Add( bSizer44, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText8 = new wxStaticText( m_transition_panel, wxID_ANY, wxT("Transition Length (s):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	bSizer21->Add( m_staticText8, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_transition_length = new wxTextCtrl( m_transition_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_transition_length->SetToolTip( wxT("Length in seconds of each transition.") );
	
	bSizer21->Add( m_transition_length, 0, wxALL|wxSHAPED|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText9 = new wxStaticText( m_transition_panel, wxID_ANY, wxT("Frames Per Transition:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	bSizer21->Add( m_staticText9, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_transition_frames = new wxTextCtrl( m_transition_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_transition_frames->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT ) );
	m_transition_frames->SetToolTip( wxT("Number of frames in each transition. This is based on the sample rate and the transition length.") );
	
	bSizer21->Add( m_transition_frames, 0, wxALL|wxSHAPED|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer19->Add( bSizer21, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText10 = new wxStaticText( m_transition_panel, wxID_ANY, wxT("Point Cloud Sample Rate (%):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	bSizer22->Add( m_staticText10, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_point_cloud_rate = new wxSlider( m_transition_panel, wxID_ANY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	m_point_cloud_rate->SetToolTip( wxT("Percentage of mesh verts to use to generate a point cloud.") );
	
	bSizer22->Add( m_point_cloud_rate, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_point_cloud_rate_value = new wxTextCtrl( m_transition_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	m_point_cloud_rate_value->SetToolTip( wxT("Percentage of mesh verts to use to generate a point cloud.") );
	
	bSizer22->Add( m_point_cloud_rate_value, 0, wxALL|wxSHAPED|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer19->Add( bSizer22, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer371;
	bSizer371 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText15 = new wxStaticText( m_transition_panel, wxID_ANY, wxT("Maximum Point Cloud Size:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText15->Wrap( -1 );
	bSizer371->Add( m_staticText15, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_max_point_cloud_size = new wxTextCtrl( m_transition_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer371->Add( m_max_point_cloud_size, 0, wxALL|wxSHAPED|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer19->Add( bSizer371, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText14 = new wxStaticText( m_transition_panel, wxID_ANY, wxT("Weight Falloff:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText14->Wrap( -1 );
	bSizer32->Add( m_staticText14, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_weight_falloff = new wxSlider( m_transition_panel, wxID_ANY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	m_weight_falloff->SetToolTip( wxT("Specified the factor of original joint weights at the end of a sample window.") );
	
	bSizer32->Add( m_weight_falloff, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_weight_falloff_value = new wxTextCtrl( m_transition_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	m_weight_falloff_value->SetToolTip( wxT("How fast to reduce weights over the range of a point cloud. Factor between 0 and 1 (0 being no falloff, 1 being instant falloff).") );
	
	bSizer32->Add( m_weight_falloff_value, 0, wxALL|wxSHAPED|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer19->Add( bSizer32, 1, wxEXPAND, 5 );
	
	sbSizer9->Add( bSizer19, 1, wxEXPAND, 5 );
	
	bSizer18->Add( sbSizer9, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer39;
	bSizer39 = new wxBoxSizer( wxHORIZONTAL );
	
	m_btn_create = new wxButton( m_transition_panel, wxID_ANY, wxT("Create"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer39->Add( m_btn_create, 0, wxALL, 5 );
	
	m_btn_cancel = new wxButton( m_transition_panel, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer39->Add( m_btn_cancel, 0, wxALL, 5 );
	
	m_btn_pause = new wxButton( m_transition_panel, wxID_ANY, wxT("Pause"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer39->Add( m_btn_pause, 0, wxALL, 5 );
	
	m_btn_next = new wxButton( m_transition_panel, wxID_ANY, wxT("Next"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer39->Add( m_btn_next, 0, wxALL, 5 );
	
	m_btn_continue = new wxButton( m_transition_panel, wxID_ANY, wxT("Continue"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer39->Add( m_btn_continue, 0, wxALL, 5 );
	
	bSizer18->Add( bSizer39, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer38;
	bSizer38 = new wxBoxSizer( wxHORIZONTAL );
	
	m_progress = new wxGauge( m_transition_panel, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	bSizer38->Add( m_progress, 1, wxALL|wxEXPAND, 5 );
	
	bSizer18->Add( bSizer38, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer33;
	bSizer33 = new wxBoxSizer( wxHORIZONTAL );
	
	m_btn_view_diff = new wxButton( m_transition_panel, wxID_ANY, wxT("View Difference Function..."), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer33->Add( m_btn_view_diff, 0, wxALL, 5 );
	
	bSizer18->Add( bSizer33, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer45;
	bSizer45 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText20 = new wxStaticText( m_transition_panel, wxID_ANY, wxT("Estimated Time Left:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText20->Wrap( -1 );
	bSizer45->Add( m_staticText20, 0, wxALL, 5 );
	
	m_time_left = new wxTextCtrl( m_transition_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_time_left->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT ) );
	
	bSizer45->Add( m_time_left, 1, wxALL|wxEXPAND, 5 );
	
	bSizer18->Add( bSizer45, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer40;
	bSizer40 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer10;
	sbSizer10 = new wxStaticBoxSizer( new wxStaticBox( m_transition_panel, wxID_ANY, wxT("Report") ), wxVERTICAL );
	
	wxBoxSizer* bSizer43;
	bSizer43 = new wxBoxSizer( wxVERTICAL );
	
	m_report = new wxTextCtrl( m_transition_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	m_report->SetToolTip( wxT("Stuff shows up here.") );
	
	bSizer43->Add( m_report, 1, wxALL|wxEXPAND, 5 );
	
	sbSizer10->Add( bSizer43, 1, wxEXPAND, 5 );
	
	bSizer40->Add( sbSizer10, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer41;
	bSizer41 = new wxBoxSizer( wxHORIZONTAL );
	
	bSizer40->Add( bSizer41, 0, wxEXPAND, 5 );
	
	bSizer18->Add( bSizer40, 1, wxEXPAND, 5 );
	
	fgSizer6->Add( bSizer18, 1, wxEXPAND, 5 );
	
	bSizer37->Add( fgSizer6, 1, wxALL|wxEXPAND, 5 );
	
	m_transition_panel->SetSizer( bSizer37 );
	m_transition_panel->Layout();
	bSizer37->Fit( m_transition_panel );
	m_listbook->AddPage( m_transition_panel, wxT("a page"), true );
	m_prune_panel = new wxPanel( m_listbook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, wxT("Graph Pruning") );
	wxBoxSizer* bSizer59;
	bSizer59 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer60;
	bSizer60 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText211 = new wxStaticText( m_prune_panel, wxID_ANY, wxT("Currently Editing:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText211->Wrap( -1 );
	bSizer60->Add( m_staticText211, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxArrayString m_prune_editChoices;
	m_prune_edit = new wxChoice( m_prune_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_prune_editChoices, 0 );
	m_prune_edit->SetSelection( 0 );
	bSizer60->Add( m_prune_edit, 1, wxALL, 5 );
	
	bSizer59->Add( bSizer60, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer61;
	bSizer61 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer62;
	bSizer62 = new wxBoxSizer( wxVERTICAL );
	
	m_btn_prune = new wxButton( m_prune_panel, wxID_ANY, wxT("Prune Graph"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer62->Add( m_btn_prune, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	bSizer61->Add( bSizer62, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer66;
	bSizer66 = new wxBoxSizer( wxVERTICAL );
	
	m_btn_export_dot = new wxButton( m_prune_panel, wxID_ANY, wxT("Export GraphViz (*.dot) ..."), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer66->Add( m_btn_export_dot, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	bSizer61->Add( bSizer66, 1, wxEXPAND, 5 );
	
	bSizer59->Add( bSizer61, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer64;
	bSizer64 = new wxBoxSizer( wxHORIZONTAL );
	
	m_prune_progress = new wxGauge( m_prune_panel, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	bSizer64->Add( m_prune_progress, 1, wxALL|wxEXPAND, 5 );
	
	bSizer59->Add( bSizer64, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer63;
	bSizer63 = new wxBoxSizer( wxVERTICAL );
	
	m_transition_report = new wxTextCtrl( m_prune_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	bSizer63->Add( m_transition_report, 1, wxALL|wxEXPAND, 5 );
	
	bSizer59->Add( bSizer63, 1, wxEXPAND, 5 );
	
	m_prune_panel->SetSizer( bSizer59 );
	m_prune_panel->Layout();
	bSizer59->Fit( m_prune_panel );
	m_listbook->AddPage( m_prune_panel, wxT("a page"), false );
	#ifndef __WXGTK__ // Small icon style not supported in GTK
	wxListView* m_listbookListView = m_listbook->GetListView();
	long m_listbookFlags = m_listbookListView->GetWindowStyleFlag();
	m_listbookFlags = ( m_listbookFlags & ~wxLC_ICON ) | wxLC_SMALL_ICON;
	m_listbookListView->SetWindowStyleFlag( m_listbookFlags );
	#endif
	
	bSizer25->Add( m_listbook, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( bSizer25 );
	this->Layout();
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MotionGraphEditor::OnClose ) );
	this->Connect( wxEVT_IDLE, wxIdleEventHandler( MotionGraphEditor::OnIdle ) );
	m_listbook->Connect( wxEVT_COMMAND_LISTBOOK_PAGE_CHANGED, wxListbookEventHandler( MotionGraphEditor::OnPageChanged ), NULL, this );
	m_listbook->Connect( wxEVT_COMMAND_LISTBOOK_PAGE_CHANGING, wxListbookEventHandler( MotionGraphEditor::OnPageChanging ), NULL, this );
	m_error_slider->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_slider->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_slider->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_slider->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_slider->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_slider->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_slider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_slider->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_slider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_value->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( MotionGraphEditor::OnEditErrorThreshold ), NULL, this );
	m_fps_sample_rate->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( MotionGraphEditor::OnTransitionLengthChanged ), NULL, this );
	m_transition_length->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( MotionGraphEditor::OnTransitionLengthChanged ), NULL, this );
	m_point_cloud_rate->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate_value->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( MotionGraphEditor::OnEditCloudSampleRate ), NULL, this );
	m_weight_falloff->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff_value->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( MotionGraphEditor::OnEditFalloff ), NULL, this );
	m_btn_create->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MotionGraphEditor::OnCreate ), NULL, this );
	m_btn_cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MotionGraphEditor::OnCancel ), NULL, this );
	m_btn_pause->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MotionGraphEditor::OnPause ), NULL, this );
	m_btn_next->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MotionGraphEditor::OnNext ), NULL, this );
	m_btn_continue->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MotionGraphEditor::OnContinue ), NULL, this );
	m_btn_view_diff->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MotionGraphEditor::OnViewDistanceFunction ), NULL, this );
	m_btn_prune->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MotionGraphEditor::OnPruneGraph ), NULL, this );
	m_btn_export_dot->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MotionGraphEditor::OnExportGraphViz ), NULL, this );
}

MotionGraphEditor::~MotionGraphEditor()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MotionGraphEditor::OnClose ) );
	this->Disconnect( wxEVT_IDLE, wxIdleEventHandler( MotionGraphEditor::OnIdle ) );
	m_listbook->Disconnect( wxEVT_COMMAND_LISTBOOK_PAGE_CHANGED, wxListbookEventHandler( MotionGraphEditor::OnPageChanged ), NULL, this );
	m_listbook->Disconnect( wxEVT_COMMAND_LISTBOOK_PAGE_CHANGING, wxListbookEventHandler( MotionGraphEditor::OnPageChanging ), NULL, this );
	m_error_slider->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_slider->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_slider->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_slider->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_slider->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_slider->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_slider->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_slider->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_slider->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( MotionGraphEditor::OnScrollErrorThreshold ), NULL, this );
	m_error_value->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( MotionGraphEditor::OnEditErrorThreshold ), NULL, this );
	m_fps_sample_rate->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( MotionGraphEditor::OnTransitionLengthChanged ), NULL, this );
	m_transition_length->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( MotionGraphEditor::OnTransitionLengthChanged ), NULL, this );
	m_point_cloud_rate->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( MotionGraphEditor::OnScrollCloudSampleRate ), NULL, this );
	m_point_cloud_rate_value->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( MotionGraphEditor::OnEditCloudSampleRate ), NULL, this );
	m_weight_falloff->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( MotionGraphEditor::OnScrollFalloff ), NULL, this );
	m_weight_falloff_value->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( MotionGraphEditor::OnEditFalloff ), NULL, this );
	m_btn_create->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MotionGraphEditor::OnCreate ), NULL, this );
	m_btn_cancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MotionGraphEditor::OnCancel ), NULL, this );
	m_btn_pause->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MotionGraphEditor::OnPause ), NULL, this );
	m_btn_next->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MotionGraphEditor::OnNext ), NULL, this );
	m_btn_continue->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MotionGraphEditor::OnContinue ), NULL, this );
	m_btn_view_diff->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MotionGraphEditor::OnViewDistanceFunction ), NULL, this );
	m_btn_prune->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MotionGraphEditor::OnPruneGraph ), NULL, this );
	m_btn_export_dot->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MotionGraphEditor::OnExportGraphViz ), NULL, this );
}

JointWeightEditor::JointWeightEditor( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 1, 1, 0, 0 );
	fgSizer4->AddGrowableCol( 0 );
	fgSizer4->AddGrowableRow( 0 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer9;
	sbSizer9 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Joint Weights") ), wxVERTICAL );
	
	m_bone_grid = new wxGrid( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER );
	
	// Grid
	m_bone_grid->CreateGrid( 0, 1 );
	m_bone_grid->EnableEditing( true );
	m_bone_grid->EnableGridLines( true );
	m_bone_grid->EnableDragGridSize( false );
	m_bone_grid->SetMargins( 0, 0 );
	
	// Columns
	m_bone_grid->EnableDragColMove( false );
	m_bone_grid->EnableDragColSize( true );
	m_bone_grid->SetColLabelSize( 30 );
	m_bone_grid->SetColLabelValue( 0, wxT("weight") );
	m_bone_grid->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Rows
	m_bone_grid->AutoSizeRows();
	m_bone_grid->EnableDragRowSize( true );
	m_bone_grid->SetRowLabelSize( 80 );
	m_bone_grid->SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Label Appearance
	
	// Cell Defaults
	m_bone_grid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	sbSizer9->Add( m_bone_grid, 1, wxALL|wxEXPAND, 5 );
	
	fgSizer4->Add( sbSizer9, 1, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( fgSizer4 );
	this->Layout();
	
	// Connect Events
	m_bone_grid->Connect( wxEVT_GRID_CELL_CHANGE, wxGridEventHandler( JointWeightEditor::OnChangeCell ), NULL, this );
	m_bone_grid->Connect( wxEVT_GRID_RANGE_SELECT, wxGridRangeSelectEventHandler( JointWeightEditor::OnSelectRange ), NULL, this );
	m_bone_grid->Connect( wxEVT_GRID_SELECT_CELL, wxGridEventHandler( JointWeightEditor::OnSelectCell ), NULL, this );
}

JointWeightEditor::~JointWeightEditor()
{
	// Disconnect Events
	m_bone_grid->Disconnect( wxEVT_GRID_CELL_CHANGE, wxGridEventHandler( JointWeightEditor::OnChangeCell ), NULL, this );
	m_bone_grid->Disconnect( wxEVT_GRID_RANGE_SELECT, wxGridRangeSelectEventHandler( JointWeightEditor::OnSelectRange ), NULL, this );
	m_bone_grid->Disconnect( wxEVT_GRID_SELECT_CELL, wxGridEventHandler( JointWeightEditor::OnSelectCell ), NULL, this );
}

DifferenceFunctionViewer::DifferenceFunctionViewer( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer34;
	bSizer34 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer35;
	bSizer35 = new wxBoxSizer( wxHORIZONTAL );
	
	m_info = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_info->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT ) );
	
	bSizer35->Add( m_info, 1, wxALL, 5 );
	
	m_btn_close = new wxButton( this, wxID_ANY, wxT("Close"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer35->Add( m_btn_close, 0, wxALL, 5 );
	
	bSizer34->Add( bSizer35, 0, wxEXPAND, 5 );
	
	wxBoxSizer* m_panel_sizer;
	m_panel_sizer = new wxBoxSizer( wxVERTICAL );
	
	m_view_panel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE|wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
	m_panel_sizer->Add( m_view_panel, 1, wxEXPAND | wxALL, 5 );
	
	bSizer34->Add( m_panel_sizer, 1, wxEXPAND, 5 );
	
	this->SetSizer( bSizer34 );
	this->Layout();
	
	// Connect Events
	m_btn_close->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DifferenceFunctionViewer::OnCloseButton ), NULL, this );
	m_view_panel->Connect( wxEVT_PAINT, wxPaintEventHandler( DifferenceFunctionViewer::OnPaintView ), NULL, this );
}

DifferenceFunctionViewer::~DifferenceFunctionViewer()
{
	// Disconnect Events
	m_btn_close->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DifferenceFunctionViewer::OnCloseButton ), NULL, this );
	m_view_panel->Disconnect( wxEVT_PAINT, wxPaintEventHandler( DifferenceFunctionViewer::OnPaintView ), NULL, this );
}

Annotations::Annotations( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer38;
	bSizer38 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer10;
	sbSizer10 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Annotations") ), wxVERTICAL );
	
	wxBoxSizer* bSizer39;
	bSizer39 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer40;
	bSizer40 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText16 = new wxStaticText( this, wxID_ANY, wxT("Clip:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText16->Wrap( -1 );
	bSizer40->Add( m_staticText16, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_info = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_info->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT ) );
	
	bSizer40->Add( m_info, 1, wxALL, 5 );
	
	bSizer39->Add( bSizer40, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer41;
	bSizer41 = new wxBoxSizer( wxVERTICAL );
	
	m_list = new wxGrid( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER );
	
	// Grid
	m_list->CreateGrid( 0, 2 );
	m_list->EnableEditing( true );
	m_list->EnableGridLines( true );
	m_list->EnableDragGridSize( false );
	m_list->SetMargins( 0, 0 );
	
	// Columns
	m_list->SetColSize( 0, 194 );
	m_list->SetColSize( 1, 181 );
	m_list->EnableDragColMove( false );
	m_list->EnableDragColSize( true );
	m_list->SetColLabelSize( 30 );
	m_list->SetColLabelValue( 0, wxT("Annotation") );
	m_list->SetColLabelValue( 1, wxT("Fidelity Threshold") );
	m_list->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Rows
	m_list->AutoSizeRows();
	m_list->EnableDragRowSize( true );
	m_list->SetRowLabelSize( 80 );
	m_list->SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Label Appearance
	
	// Cell Defaults
	m_list->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	bSizer41->Add( m_list, 1, wxALL|wxEXPAND, 5 );
	
	bSizer39->Add( bSizer41, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer42;
	bSizer42 = new wxBoxSizer( wxHORIZONTAL );
	
	m_anno_name = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	bSizer42->Add( m_anno_name, 0, wxALL, 5 );
	
	m_btn_add = new wxButton( this, wxID_ANY, wxT("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer42->Add( m_btn_add, 0, wxALL, 5 );
	
	m_btn_remove = new wxButton( this, wxID_ANY, wxT("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer42->Add( m_btn_remove, 0, wxALL, 5 );
	
	bSizer39->Add( bSizer42, 0, wxEXPAND, 5 );
	
	sbSizer10->Add( bSizer39, 1, wxEXPAND, 5 );
	
	bSizer38->Add( sbSizer10, 1, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( bSizer38 );
	this->Layout();
	
	// Connect Events
	m_list->Connect( wxEVT_GRID_CELL_CHANGE, wxGridEventHandler( Annotations::OnEditCell ), NULL, this );
	m_btn_add->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Annotations::OnAddAnnotation ), NULL, this );
	m_btn_remove->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Annotations::OnRemoveAnnotation ), NULL, this );
}

Annotations::~Annotations()
{
	// Disconnect Events
	m_list->Disconnect( wxEVT_GRID_CELL_CHANGE, wxGridEventHandler( Annotations::OnEditCell ), NULL, this );
	m_btn_add->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Annotations::OnAddAnnotation ), NULL, this );
	m_btn_remove->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Annotations::OnRemoveAnnotation ), NULL, this );
}

ChangeSkeleton::ChangeSkeleton( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer44;
	bSizer44 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer11;
	sbSizer11 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Skeleton") ), wxVERTICAL );
	
	wxBoxSizer* bSizer45;
	bSizer45 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText17 = new wxStaticText( this, wxID_ANY, wxT("Select Skeleton:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText17->Wrap( -1 );
	bSizer45->Add( m_staticText17, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxArrayString m_skeletonsChoices;
	m_skeletons = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_skeletonsChoices, 0 );
	m_skeletons->SetSelection( 0 );
	bSizer45->Add( m_skeletons, 1, wxALL, 5 );
	
	sbSizer11->Add( bSizer45, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer46;
	bSizer46 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer77;
	bSizer77 = new wxBoxSizer( wxVERTICAL );
	
	m_btn_delete_skel = new wxButton( this, wxID_ANY, wxT("Delete Skeleton"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer77->Add( m_btn_delete_skel, 0, wxALL|wxALIGN_RIGHT, 5 );
	
	bSizer46->Add( bSizer77, 1, wxEXPAND, 5 );
	
	sbSizer11->Add( bSizer46, 0, wxEXPAND, 5 );
	
	bSizer44->Add( sbSizer11, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer12;
	sbSizer12 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Motion Graph") ), wxVERTICAL );
	
	wxBoxSizer* bSizer451;
	bSizer451 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText171 = new wxStaticText( this, wxID_ANY, wxT("Select Motion Graph:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText171->Wrap( -1 );
	bSizer451->Add( m_staticText171, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxArrayString m_mgsChoices;
	m_mgs = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_mgsChoices, 0 );
	m_mgs->SetSelection( 0 );
	bSizer451->Add( m_mgs, 1, wxALL, 5 );
	
	sbSizer12->Add( bSizer451, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer461;
	bSizer461 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer79;
	bSizer79 = new wxBoxSizer( wxVERTICAL );
	
	m_btn_delete_mg = new wxButton( this, wxID_ANY, wxT("Delete Motion Graph"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer79->Add( m_btn_delete_mg, 0, wxALL|wxALIGN_RIGHT, 5 );
	
	bSizer461->Add( bSizer79, 1, wxEXPAND, 5 );
	
	sbSizer12->Add( bSizer461, 0, wxEXPAND, 5 );
	
	bSizer44->Add( sbSizer12, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer13;
	sbSizer13 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Mesh") ), wxVERTICAL );
	
	wxBoxSizer* bSizer78;
	bSizer78 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText22 = new wxStaticText( this, wxID_ANY, wxT("Select Mesh:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText22->Wrap( -1 );
	bSizer78->Add( m_staticText22, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxArrayString m_meshesChoices;
	m_meshes = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_meshesChoices, 0 );
	m_meshes->SetSelection( 0 );
	bSizer78->Add( m_meshes, 1, wxALL, 5 );
	
	sbSizer13->Add( bSizer78, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer80;
	bSizer80 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer82;
	bSizer82 = new wxBoxSizer( wxVERTICAL );
	
	m_btn_delete_mesh = new wxButton( this, wxID_ANY, wxT("Delete Mesh"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer82->Add( m_btn_delete_mesh, 0, wxALL|wxALIGN_RIGHT, 5 );
	
	bSizer80->Add( bSizer82, 1, wxEXPAND, 5 );
	
	sbSizer13->Add( bSizer80, 1, wxEXPAND, 5 );
	
	bSizer44->Add( sbSizer13, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer55;
	bSizer55 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer57;
	bSizer57 = new wxBoxSizer( wxVERTICAL );
	
	bSizer55->Add( bSizer57, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer56;
	bSizer56 = new wxBoxSizer( wxVERTICAL );
	
	m_btn_apply = new wxButton( this, wxID_ANY, wxT("Apply"), wxDefaultPosition, wxDefaultSize, 0 );
	m_btn_apply->SetDefault(); 
	m_btn_apply->SetFont( wxFont( 8, 74, 90, 92, false, wxT("Sans") ) );
	
	bSizer56->Add( m_btn_apply, 0, wxALL|wxALIGN_RIGHT, 5 );
	
	bSizer55->Add( bSizer56, 0, wxEXPAND, 5 );
	
	bSizer44->Add( bSizer55, 0, wxEXPAND, 5 );
	
	this->SetSizer( bSizer44 );
	this->Layout();
	
	// Connect Events
	m_skeletons->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ChangeSkeleton::OnChooseSkeleton ), NULL, this );
	m_btn_delete_skel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChangeSkeleton::OnDeleteSkeleton ), NULL, this );
	m_btn_delete_mg->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChangeSkeleton::OnDeleteMotionGraph ), NULL, this );
	m_btn_delete_mesh->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChangeSkeleton::OnDeleteMesh ), NULL, this );
	m_btn_apply->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChangeSkeleton::OnApply ), NULL, this );
}

ChangeSkeleton::~ChangeSkeleton()
{
	// Disconnect Events
	m_skeletons->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( ChangeSkeleton::OnChooseSkeleton ), NULL, this );
	m_btn_delete_skel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChangeSkeleton::OnDeleteSkeleton ), NULL, this );
	m_btn_delete_mg->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChangeSkeleton::OnDeleteMotionGraph ), NULL, this );
	m_btn_delete_mesh->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChangeSkeleton::OnDeleteMesh ), NULL, this );
	m_btn_apply->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChangeSkeleton::OnApply ), NULL, this );
}
