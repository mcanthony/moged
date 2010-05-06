#include <ostream>
#include "mogedAnnotations.h"
#include "appcontext.hh"
#include "entity.hh"

using namespace std;

mogedAnnotations::mogedAnnotations( wxWindow* parent, AppContext* ctx )
:
Annotations( parent )
, m_ctx(ctx)
, m_current_clip(0)
{
	RefreshView();
}

void mogedAnnotations::HandleEvent(Events::Event* ev)
{
	using namespace Events;
	if(ev->GetType() == EventID_ActiveClipEvent) {
		ActiveClipEvent* ace = static_cast<ActiveClipEvent*>(ev);
		SetCurrentClip( ace->ClipID );
	}
}

void mogedAnnotations::SetCurrentClip( sqlite3_int64 clip )
{
	m_current_clip = clip;
	m_info->Clear();
	const ClipDB* clips = m_ctx->GetEntity()->GetClips();
	if(clips)
	{
		ostream o(m_info);
		if(clip == 0) {
			o << "Showing all annotations.";			
		} else {
			ClipInfoBrief info;
			clips->GetClipInfoBrief( clip, info );
			o << "Showing annotations for clip: " << info.name.c_str();				
		}
		RefreshView();
	}
}

void mogedAnnotations::RefreshView( ) 
{
	m_all_annos.clear();
	const ClipDB* clips = m_ctx->GetEntity()->GetClips();
	if(clips)
		clips->GetAnnotations(m_all_annos);

	// fill combo box
	m_anno_name->Clear();
	m_anno_name->SetSelection(wxNOT_FOUND);
	const int all_annos_size = m_all_annos.size();
	for(int i = 0; i < all_annos_size; ++i) {
		m_anno_name->Append( wxString(m_all_annos[i].GetName(),wxConvUTF8) );
	}

	m_list->ClearGrid();
	m_list->DeleteRows(0, m_list->GetNumberRows());

	std::vector<Annotation >* annos = &m_all_annos;
	if(m_current_clip) {		
		clips->GetAnnotations( m_annos , m_current_clip );
		annos = &m_annos;
	}

	const int num_annos = annos->size();
	m_list->InsertRows(0, num_annos);
	for(int i = 0; i < num_annos; ++i)
	{
		m_list->SetCellValue(i, 0, wxString( annos->at(i).GetName(), wxConvUTF8 ) );
		wxString fidelity ;
		fidelity << annos->at(i).GetFidelity();
		m_list->SetCellValue(i, 1, fidelity);
	}
}

void mogedAnnotations::OnAddAnnotation( wxCommandEvent& event )
{
	(void)event;
	if(m_anno_name->GetValue().Len() == 0)
		return;

	ClipDB* clips = m_ctx->GetEntity()->GetClips();
	if(clips == 0) return;

	int idx = FindAnnotation(m_all_annos, m_anno_name->GetValue().char_str());

	if(m_current_clip) { // adding to this clip
		int cur_idx = FindAnnotation(m_annos, m_anno_name->GetValue().char_str() );
		if(cur_idx != -1) {
			m_list->SelectRow(cur_idx);
			return;
		} else {
			if(idx == -1) { // it's not in the main list, so create it.
				Annotation anno = clips->AddAnnotation( m_anno_name->GetValue().char_str() );
				if(anno.Valid()) anno.ApplyToClip(m_current_clip);
			}
			else { // exists, but not in this clip, so add it
				m_all_annos[idx].ApplyToClip(m_current_clip);
			}
		}
	} else { // in global mode.
		if(idx != -1) {
			m_list->SelectRow(idx);
			return;
		} else {
			clips->AddAnnotation( m_anno_name->GetValue().char_str() );
		}
	}

	RefreshView();
}

void mogedAnnotations::OnRemoveAnnotation( wxCommandEvent& event )
{
	(void)event;
	ClipDB* clips = m_ctx->GetEntity()->GetClips();
	if(clips == 0) return;

	wxArrayInt selectedRows = m_list->GetSelectedRows();
	const size_t count = selectedRows.GetCount();
	for(size_t i = 0; i < count; ++i) {
		if(m_current_clip) {
			m_annos[ selectedRows.Item(i) ].RemoveFromClip(m_current_clip);
		} else {
			clips->RemoveAnnotation( m_all_annos[ selectedRows.Item(i) ].GetID() );
		}
	}
	
	if(count > 0)
		RefreshView();
}

void mogedAnnotations::OnEditCell( wxGridEvent& event ) 
{
	(void)event;
	
}


int mogedAnnotations::FindAnnotation(const std::vector< Annotation >& list, const char* name )
{
	const int count = list.size();
	for(int i = 0; i < count; ++i) {
		if(strcmp(name, list[i].GetName()) == 0) {
			return i;
		}
	}
	return -1;	
}




