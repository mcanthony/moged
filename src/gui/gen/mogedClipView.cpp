#include <cstdio>
#include <algorithm>
#include <wx/wx.h>
#include <wx/confbase.h>
#include "mogedClipView.h"
#include "clipdb.hh"
#include "clip.hh"
#include "appcontext.hh"
#include "entity.hh"

int wxCALLBACK clipListCompare(long item1, long item2, long sortData)
{
	(void)item2;
	char* toPtr = reinterpret_cast<char*>(sortData);	
	wxListCtrl * ctrl = reinterpret_cast<wxListCtrl*>(toPtr);
	return -ctrl->GetItemText(item1).Cmp(ctrl->GetItemText(2));
}

mogedClipView::mogedClipView( wxWindow* parent, AppContext* ctx )
: ClipView( parent )
, m_ctx(ctx)
, m_current_clip(0)
{
	bool checkTrans = false;
	bool checkOriginals = true;
	wxConfigBase* cfg = wxConfigBase::Get();
	if(cfg)
	{
		cfg->Read(_("ClipView/ShowTransitions"), &checkTrans, false);
		cfg->Read(_("ClipView/ShowOriginals"), &checkOriginals, true);
	}
	m_check_transitions->SetValue(checkTrans);
	m_check_originals->SetValue(checkOriginals);

	RefreshView();
}

mogedClipView::~mogedClipView( ) 
{
	wxConfigBase* cfg = wxConfigBase::Get();
	if(cfg == 0) return;
	
	cfg->Write(_("ClipView/ShowTransitions"), m_check_transitions->GetValue());
	cfg->Write(_("ClipView/ShowOriginals"), m_check_originals->GetValue());
}

void mogedClipView::HandleEvent( Events::Event* ev)
{
	using namespace Events;
	if(ev->GetType() == EventID_EntitySkeletonChangedEvent)
		RefreshView();
	else if(ev->GetType() == EventID_ClipAddedEvent) {
		ClipAddedEvent* cae = static_cast<ClipAddedEvent*>(ev);

		if(m_ctx->GetEntity()->GetClips()) {
			ClipInfoBrief info;
			m_ctx->GetEntity()->GetClips()->GetClipInfoBrief( cae->ClipID, info );
			if(info.id > 0) {
				int idx = m_infos.size();
				m_infos.push_back( info );
				wxListItem newItem;
				newItem.SetText( wxString(info.name.c_str(), wxConvUTF8) );				
				newItem.SetData( (void*)idx );
				m_clips->InsertItem( newItem );
				m_clips->SortItems( clipListCompare, (long)(m_clips));
			}
		}
	}
}

void mogedClipView::SimpleRefreshView()
{
	m_clips->ClearAll();
	const int num_clips = m_infos.size();
	for(int i = 0; i < num_clips; ++i) {
		wxString name = _("no-name clip");
		if(!m_infos[i].name.empty()) {
			name = wxString(m_infos[i].name.c_str(), wxConvUTF8);
		}
			
		wxListItem newItem ;
		newItem.SetId(0);
		newItem.SetText( name );
		newItem.SetData( (void*)i );			
		m_clips->InsertItem( newItem );
	}
	m_clips->SortItems(clipListCompare, (long)(m_clips));	
}

void mogedClipView::RefreshView()
{
	bool checkTrans = m_check_transitions->GetValue();
	bool checkOriginals = m_check_originals->GetValue();

	m_infos.clear();
	const ClipDB* db = m_ctx->GetEntity()->GetClips();
	if(db)
	{
		db->GetAllClipInfoBrief( m_infos, checkOriginals, checkTrans );
		SimpleRefreshView();
	}
}

void mogedClipView::OnDelete( wxCommandEvent& event) 
{
	(void)event;
	ClipDB* db = m_ctx->GetEntity()->GetClips();

	long item = m_clips->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	std::vector< int > toRemove;
	while(item != -1) {
		toRemove.push_back(m_clips->GetItemData(item));
		item = m_clips->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	}

	if(!toRemove.empty()) {
		wxString msg = _("Are you sure you want to remove the following clips?\n\n");
		wxString list ;
		GenerateNameList( toRemove, list );
		wxMessageDialog dlg(this, msg + list, _("Confirm"), wxYES_NO|wxICON_HAND);
		if( dlg.ShowModal() == wxID_NO ) {
			return;
		}
	}

	std::vector< int > actuallyRemoved ;
	std::vector< int > removalFail ;
	for(int i = 0; i < (int)toRemove.size(); ++i) 
	{
		sqlite3_int64 clip_id = m_infos[ toRemove[i] ].id;		
		if(db->RemoveClip( clip_id )) {
			if(clip_id == m_current_clip) {
				Events::ActiveClipEvent ev;
				m_current_clip = ev.ClipID = 0;
				m_ctx->GetEventSystem()->Send(&ev);			
			}
			actuallyRemoved.push_back( toRemove[i] );
		} else { 
			removalFail.push_back( toRemove[i] );
		}
	}
	
	if(!removalFail.empty()) {
		wxString msg = _("The following clips did not get removed:\n\n");
		wxString list;
		GenerateNameList( removalFail, list );
		wxMessageDialog dlg( this, msg + list, _("Error"), wxOK|wxICON_ERROR);
		dlg.ShowModal();
	}

	// will be in decreasing order because they are added in increasing order.
	std::sort(actuallyRemoved.begin(), actuallyRemoved.end());
	for(int i = actuallyRemoved.size()-1; i>=0; --i) {
		m_infos.erase(m_infos.begin() + actuallyRemoved[i]);
	}
	SimpleRefreshView();
}

void mogedClipView::OnRenameClip( wxListEvent& event )
{
	if( event.GetLabel().empty() )
		event.Veto();
	else 
	{
		int idx = event.GetItem().GetData();
		sqlite3_int64 clip_id = m_infos[idx].id;
		
		ClipDB* db = m_ctx->GetEntity()->GetClips();
		if(db) {
			ClipHandle clip = db->GetClip(clip_id);
			clip->SetName( event.GetItem().GetText().char_str() );

			Events::ClipModifiedEvent ev;
			ev.ClipID = clip_id;
			m_ctx->GetEventSystem()->Send(&ev);
		}		
	}
}

void mogedClipView::OnActivateClip( wxListEvent& event)
{
	int idx = event.GetItem().GetData();
	sqlite3_int64 clip_id = m_infos[idx].id;

	Events::ActiveClipEvent ev;
	m_current_clip = ev.ClipID = clip_id;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipView::OnClearSelection( wxCommandEvent& event )
{
	(void)event;
	Events::ActiveClipEvent ev;
	m_current_clip = ev.ClipID = 0;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipView::OnRightClick( wxMouseEvent& event ) 
{
	(void)event;
	
	const ClipDB* clips = m_ctx->GetEntity()->GetClips();
	if(clips == 0) return;
	clips->GetAnnotations(m_annotations);
	if(m_annotations.empty()) return;

	wxMenu mnu;

	wxMenu *addMenu = new wxMenu;
	const int count = m_annotations.size();
	for(int i = 0; i < count; ++i) {
		addMenu->Append( i, wxString( m_annotations[i].GetName(), wxConvUTF8 ) );
	}
	addMenu->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&mogedClipView::OnApplyAnnotation, NULL, this);

	wxMenu *subMenu = new wxMenu;
	for(int i = 0; i < count; ++i) {
		subMenu->Append( i, wxString( m_annotations[i].GetName(), wxConvUTF8 ) );
	}
	subMenu->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&mogedClipView::OnRemoveAnnotation, NULL, this);

	mnu.AppendSubMenu(addMenu, _("Apply Annotation"));
	mnu.AppendSubMenu(subMenu, _("Remove Annotation"));
	PopupMenu(&mnu);	
}

void mogedClipView::OnApplyAnnotation( wxCommandEvent& evt)
{
	size_t idx = evt.GetId();
	if(idx < m_annotations.size()) {
		long item = m_clips->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		while(item != -1) {
			m_annotations[idx].ApplyToClip( m_infos[m_clips->GetItemData(item)].id );
			item = m_clips->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		}
	}	
	
	Events::AnnotationsAddedEvent ev;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipView::OnRemoveAnnotation( wxCommandEvent& evt)
{
	size_t idx = evt.GetId();
	if(idx < m_annotations.size()) {
		long item = m_clips->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		while(item != -1) {
			m_annotations[idx].RemoveFromClip( m_infos[m_clips->GetItemData(item)].id );
			item = m_clips->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		}
	}	
	
	Events::AnnotationsAddedEvent ev;
	m_ctx->GetEventSystem()->Send(&ev);
}

void mogedClipView::OnShowTransitions( wxCommandEvent& event ) 
{
	(void)event;
	RefreshView();
}
 
void mogedClipView::OnShowOriginals( wxCommandEvent& event ) 
{
	(void)event;
	RefreshView();	
}


////////////////////////////////////////////////////////////////////////////////
void mogedClipView::GenerateNameList( const std::vector<int>& indices, wxString&result )
{
	const int num_indices = indices.size();
	for(int i = 0; i < num_indices; ++i) {
		result += wxString( m_infos[ indices[i] ].name.c_str(), wxConvUTF8 ) + _("\n");
	}
}
