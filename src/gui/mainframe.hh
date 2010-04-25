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

	void OnQuit(wxCommandEvent& event);

	void OnPlaybackMode(wxCommandEvent& event);
	void OnSkeletonMode(wxCommandEvent& event);

	void OnNewEntity(wxCommandEvent& event);
	void OnOpenEntity(wxCommandEvent& event);
	void OnSaveEntity(wxCommandEvent& event);

	void OnSetBaseFolder(wxCommandEvent& event);

	void OnImportAcclaim(wxCommandEvent& event);
private:
	void InitWiring();
	void UpdateFancyTitle();
	DECLARE_EVENT_TABLE();
};

#endif
