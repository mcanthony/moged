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
#include <wx/panel.h>

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
		wxListCtrl* m_clips;
		wxButton* m_delete;
	
	public:
		ClipView( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 541,480 ), long style = wxTAB_TRAVERSAL );
		~ClipView();
	
};

#endif //__MogedFrames__
