#include <cstdio>
#include "mogedClipView.h"
#include "clipdb.hh"
#include "clip.hh"
#include "appcontext.hh"
#include "entity.hh"

int wxCALLBACK clipListCompare(long item1, long item2, long sortData)
{
	wxListCtrl* ctrl = *reinterpret_cast<wxListCtrl**>(&sortData);
	return -ctrl->GetItemText(item1).Cmp(ctrl->GetItemText(2));
}

mogedClipView::mogedClipView( wxWindow* parent, AppContext* ctx )
:
ClipView( parent )
, m_ctx(ctx)
, m_current_clip(0)
{
	RefreshView();
}

void mogedClipView::HandleEvent( Events::Event* ev)
{
	using namespace Events;
	if(ev->GetType() == EventID_EntitySkeletonChangedEvent)
		RefreshView();
	else if(ev->GetType() == EventID_ClipAddedEvent) {
		ClipAddedEvent* cae = static_cast<ClipAddedEvent*>(ev);
		wxListItem newItem;
		newItem.SetText( wxString(cae->ClipPtr->GetName(), wxConvUTF8) );
		newItem.SetData( cae->ClipPtr );
		m_clips->InsertItem( newItem );
		m_clips->SortItems( clipListCompare, (long)(m_clips));
	}
}

void mogedClipView::RefreshView()
{
	Events::ActiveClipEvent ev;
	m_current_clip = ev.ClipPtr = 0;
	m_ctx->GetEventSystem()->Send(&ev);			

	m_clips->ClearAll();
	const ClipDB* db = m_ctx->GetEntity()->GetClips();
	if(db)
	{
		int num_clips = db->GetNumClips();
		for(int i = 0; i < num_clips; ++i) {
			const Clip* clip = db->GetClip(i);

			wxString name = _("no-name clip");
			if(clip->GetName()[0] != '\0') {
				name = wxString(clip->GetName(), wxConvUTF8);
			}
			
			wxListItem newItem ;
			newItem.SetId(0);
			newItem.SetText( name );
			newItem.SetData( (void*)clip );			
			m_clips->InsertItem( newItem );
		}
		m_clips->SortItems(clipListCompare, (long)(m_clips));
	}
}

void mogedClipView::OnDelete( wxCommandEvent& event) 
{
	ClipDB* db = m_ctx->GetEntity()->GetClips();

	long item = m_clips->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	while(item != -1) {
		Clip* itemData = reinterpret_cast<Clip*>(m_clips->GetItemData(item));
		if(itemData == m_current_clip) {
			Events::ActiveClipEvent ev;
			m_current_clip = ev.ClipPtr = 0;
			m_ctx->GetEventSystem()->Send(&ev);			
		}	
		db->RemoveClip( itemData );		
		m_clips->DeleteItem(item);
		item = m_clips->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	}
}

void mogedClipView::OnRenameClip( wxListEvent& event )
{
	if( event.GetLabel().empty() )
		event.Veto();
	else 
	{
		Clip* clip = reinterpret_cast<Clip*>(event.GetItem().GetData());	
		clip->SetName( event.GetLabel().char_str() );
		
		Events::ClipModifiedEvent ev;
		m_ctx->GetEventSystem()->Send(&ev);
	}
}

void mogedClipView::OnActivateClip( wxListEvent& event)
{
	const wxListItem& item = event.GetItem();
	Events::ActiveClipEvent ev;
	m_current_clip = ev.ClipPtr = reinterpret_cast<Clip*>(item.GetData());	
	m_ctx->GetEventSystem()->Send(&ev);
}
