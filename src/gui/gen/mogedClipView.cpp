#include <cstdio>
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
	RefreshView();
}

mogedClipView::~mogedClipView( ) 
{
	
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
	Events::ActiveClipEvent ev;
	m_current_clip = ev.ClipID = 0;
	m_ctx->GetEventSystem()->Send(&ev);			

	m_infos.clear();
	const ClipDB* db = m_ctx->GetEntity()->GetClips();
	if(db)
	{
		db->GetAllClipInfoBrief( m_infos );
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
		int idx = item;
		toRemove.push_back(idx);
		sqlite3_int64 clip_id = m_infos[idx].id;
		if(clip_id == m_current_clip) {
			Events::ActiveClipEvent ev;
			m_current_clip = ev.ClipID = 0;
			m_ctx->GetEventSystem()->Send(&ev);			
		}
		db->RemoveClip( clip_id );

		item = m_clips->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	}

	// will be in decreasing order because they are added in increasing order.
	for(int i = toRemove.size()-1; i>=0; --i) {
		m_infos.erase(m_infos.begin() + toRemove[i]);
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
