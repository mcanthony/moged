#ifndef INCLUDED_moged_mainframe_HH
#define INCLUDED_moged_mainframe_HH

#include <wx/wx.h>
#include <wx/aui/aui.h>

class wxConfig;
class wxGLContext;
class Canvas;
class AppContext;
class mogedClipView;
class mogedClipControls;

class MainFrame : public wxFrame
{
	wxConfig* m_config;
	wxAuiManager m_mgr;
	wxGLContext* m_context;

	Canvas *m_canvas;
	AppContext* m_appctx;

	mogedClipView* m_clipview;
	mogedClipControls* m_clipcontrols;
public:
	MainFrame( const wxString& title, const wxPoint& pos, const wxSize& size, wxConfig* config, AppContext* context );
	~MainFrame();
	void Update();
protected:
	void OnImportMesh(wxCommandEvent& event);
	void OnClearMesh(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);

	void OnPlaybackMode(wxCommandEvent& event);
	void OnSkeletonMode(wxCommandEvent& event);

	void OnNewEntity(wxCommandEvent& event);
	void OnOpenEntity(wxCommandEvent& event);
	void OnSaveEntity(wxCommandEvent& event);

	void OnSetBaseFolder(wxCommandEvent& event);

	void OnImportAcclaim(wxCommandEvent& event);
	
	void OnToggleVisibility(wxCommandEvent& event);   
private:
	void InitWiring();
	void UpdateFancyTitle();
	void ToggleViewMenuItem( int id, bool shown );
	void TogglePaneVisibility( wxWindow *window, bool shown );
	DECLARE_EVENT_TABLE();
};

#endif
