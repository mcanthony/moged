///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Dec 29 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __MogedFrames__
#define __MogedFrames__

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/filepicker.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/listctrl.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/gauge.h>
#include <wx/dialog.h>
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/slider.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/listbook.h>
#include <wx/grid.h>
#include <wx/combobox.h>
#include <wx/choice.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class ImportClipsDlg
///////////////////////////////////////////////////////////////////////////////
class ImportClipsDlg : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText6;
		wxFilePickerCtrl* m_asf_file;
		wxListCtrl* m_clip_list;
		wxButton* m_add_button;
		wxButton* m_remove_button;
		wxStaticText* m_staticText2;
		wxTextCtrl* m_fps;
		wxTextCtrl* m_report;
		wxGauge* m_gauge;
		wxButton* m_cancel_button;
		wxButton* m_import_button;
		wxButton* m_done;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnAddClips( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnRemoveClips( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnImport( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnDone( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		ImportClipsDlg( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Import Acclaim Clips"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 567,621 ), long style = wxDEFAULT_DIALOG_STYLE );
		~ImportClipsDlg();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ClipView
///////////////////////////////////////////////////////////////////////////////
class ClipView : public wxPanel 
{
	private:
	
	protected:
		wxButton* m_btn_clear_sel;
		wxListCtrl* m_clips;
		wxButton* m_delete;
		wxCheckBox* m_check_transitions;
		wxCheckBox* m_check_originals;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClearSelection( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnRenameClip( wxListEvent& event ){ event.Skip(); }
		virtual void OnActivateClip( wxListEvent& event ){ event.Skip(); }
		virtual void OnRightClick( wxMouseEvent& event ){ event.Skip(); }
		virtual void OnDelete( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		ClipView( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 541,480 ), long style = wxTAB_TRAVERSAL );
		~ClipView();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ClipControls
///////////////////////////////////////////////////////////////////////////////
class ClipControls : public wxPanel 
{
	private:
	
	protected:
		wxStaticText* m_staticText6;
		wxTextCtrl* m_clip_name;
		wxButton* m_rewind;
		wxButton* m_step_back;
		wxButton* m_play;
		wxButton* m_step_fwd;
		wxButton* m_jump_end;
		wxButton* m_stop;
		wxSlider* m_frame_slider;
		wxStaticText* m_staticText3;
		wxTextCtrl* m_cur_frame;
		wxStaticText* m_staticText4;
		wxTextCtrl* m_frame_count;
		wxStaticText* m_staticText5;
		wxTextCtrl* m_clip_length;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnRewindAll( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnRewind( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnPlay( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnFwd( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnFwdAll( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnStop( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnScrollFrame( wxScrollEvent& event ){ event.Skip(); }
		
	
	public:
		ClipControls( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 601,161 ), long style = wxTAB_TRAVERSAL );
		~ClipControls();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class MotionGraphEditor
///////////////////////////////////////////////////////////////////////////////
class MotionGraphEditor : public wxDialog 
{
	private:
	
	protected:
		wxListbook* m_listbook;
		wxPanel* m_transition_panel;
		wxStaticText* m_staticText7;
		wxSlider* m_error_slider;
		wxTextCtrl* m_error_value;
		wxStaticText* m_staticText19;
		wxTextCtrl* m_fps_sample_rate;
		wxStaticText* m_staticText21;
		wxTextCtrl* m_num_threads;
		wxStaticText* m_staticText8;
		wxTextCtrl* m_transition_length;
		wxStaticText* m_staticText9;
		wxTextCtrl* m_transition_frames;
		wxStaticText* m_staticText10;
		wxSlider* m_point_cloud_rate;
		wxTextCtrl* m_point_cloud_rate_value;
		wxStaticText* m_staticText15;
		wxTextCtrl* m_max_point_cloud_size;
		wxStaticText* m_staticText14;
		wxSlider* m_weight_falloff;
		wxTextCtrl* m_weight_falloff_value;
		wxButton* m_btn_create;
		wxButton* m_btn_cancel;
		wxButton* m_btn_pause;
		wxButton* m_btn_next;
		wxButton* m_btn_continue;
		wxGauge* m_progress;
		wxButton* m_btn_view_diff;
		wxStaticText* m_staticText20;
		wxTextCtrl* m_time_left;
		wxTextCtrl* m_report;
		wxButton* m_continue;
		wxPanel* m_prune_panel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ){ event.Skip(); }
		virtual void OnIdle( wxIdleEvent& event ){ event.Skip(); }
		virtual void OnPageChanged( wxListbookEvent& event ){ event.Skip(); }
		virtual void OnPageChanging( wxListbookEvent& event ){ event.Skip(); }
		virtual void OnScrollErrorThreshold( wxScrollEvent& event ){ event.Skip(); }
		virtual void OnEditErrorThreshold( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnTransitionLengthChanged( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnScrollCloudSampleRate( wxScrollEvent& event ){ event.Skip(); }
		virtual void OnEditCloudSampleRate( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnScrollFalloff( wxScrollEvent& event ){ event.Skip(); }
		virtual void OnEditFalloff( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnCreate( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnPause( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnNext( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnContinue( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnViewDistanceFunction( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnNextStage( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		MotionGraphEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Motion Graph Wizard"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 595,660 ), long style = wxDEFAULT_DIALOG_STYLE );
		~MotionGraphEditor();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class JointWeightEditor
///////////////////////////////////////////////////////////////////////////////
class JointWeightEditor : public wxPanel 
{
	private:
	
	protected:
		wxGrid* m_bone_grid;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnChangeCell( wxGridEvent& event ){ event.Skip(); }
		virtual void OnSelectRange( wxGridRangeSelectEvent& event ){ event.Skip(); }
		virtual void OnSelectCell( wxGridEvent& event ){ event.Skip(); }
		
	
	public:
		JointWeightEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 280,576 ), long style = wxTAB_TRAVERSAL );
		~JointWeightEditor();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DifferenceFunctionViewer
///////////////////////////////////////////////////////////////////////////////
class DifferenceFunctionViewer : public wxDialog 
{
	private:
	
	protected:
		wxTextCtrl* m_info;
		wxButton* m_btn_close;
		wxPanel* m_view_panel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnCloseButton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnPaintView( wxPaintEvent& event ){ event.Skip(); }
		
	
	public:
		DifferenceFunctionViewer( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("View Difference Function"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 580,508 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
		~DifferenceFunctionViewer();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class Annotations
///////////////////////////////////////////////////////////////////////////////
class Annotations : public wxPanel 
{
	private:
	
	protected:
		wxStaticText* m_staticText16;
		wxTextCtrl* m_info;
		wxGrid* m_list;
		wxComboBox* m_anno_name;
		wxButton* m_btn_add;
		wxButton* m_btn_remove;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnEditCell( wxGridEvent& event ){ event.Skip(); }
		virtual void OnAddAnnotation( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnRemoveAnnotation( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		Annotations( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 547,371 ), long style = wxTAB_TRAVERSAL );
		~Annotations();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ChangeSkeleton
///////////////////////////////////////////////////////////////////////////////
class ChangeSkeleton : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText17;
		wxChoice* m_skeletons;
		wxButton* m_btn_delete_skel;
		wxStaticText* m_staticText171;
		wxChoice* m_mgs;
		wxButton* m_btn_delete_mg;
		wxStaticText* m_staticText22;
		wxChoice* m_meshes;
		wxButton* m_btn_delete_mesh;
		wxButton* m_btn_apply;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnChooseSkeleton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnDeleteSkeleton( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnDeleteMotionGraph( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnDeleteMesh( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnApply( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		ChangeSkeleton( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Modify Entity"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 508,413 ), long style = wxDEFAULT_DIALOG_STYLE );
		~ChangeSkeleton();
	
};

#endif //__MogedFrames__
