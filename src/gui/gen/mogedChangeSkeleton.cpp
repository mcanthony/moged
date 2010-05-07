#include <wx/wx.h>
#include "mogedChangeSkeleton.h"
#include "appcontext.hh"
#include "entity.hh"
#include "skeleton.hh"
#include "mesh.hh"
#include "motiongraph.hh"

mogedChangeSkeleton::mogedChangeSkeleton( wxWindow* parent, AppContext *ctx )
:
ChangeSkeleton( parent )
, m_ctx(ctx)
{
	RefreshView();
}

void mogedChangeSkeleton::RefreshView()
{
	m_skeletons->Clear();
	m_meshes->Clear();
	m_mgs->Clear();

	// get list of skeletons from entity
	m_ctx->GetEntity()->GetSkeletonInfos( m_skel_infos );
	m_mesh_infos.clear();
	m_mg_infos.clear();
	
	int selSkel = wxNOT_FOUND;
	for(int i = 0; i < (int)m_skel_infos.size(); ++i)
	{
		int result = m_skeletons->Append( wxString(m_skel_infos[i].name.c_str(), wxConvUTF8), (void*)i);		
		if( m_ctx->GetEntity()->GetSkeleton() && m_ctx->GetEntity()->GetSkeleton()->GetID() == 
			m_skel_infos[i].id)
		{
			RefreshViewFromSkel( m_skel_infos[i].id );
			selSkel = result;
		}
	}

	m_skeletons->SetSelection(selSkel);
}

void mogedChangeSkeleton::RefreshViewFromSkel(sqlite3_int64 skel)
{
	int selMesh = wxNOT_FOUND;
	int selMg = wxNOT_FOUND;

	m_meshes->Clear();
	m_mgs->Clear();

	if(skel == 0) {
		m_mesh_infos.clear();
		m_mg_infos.clear();
	} else {
		m_ctx->GetEntity()->GetMeshInfos( m_mesh_infos, skel );
		m_ctx->GetEntity()->GetMotionGraphInfos( m_mg_infos, skel );
	}

	for(int i = 0; i < (int)m_mesh_infos.size(); ++i)
	{
		int result = m_meshes->Append( wxString(m_mesh_infos[i].name.c_str(), wxConvUTF8), (void*)i);
		if( m_ctx->GetEntity()->GetMesh() && m_ctx->GetEntity()->GetMesh()->GetID() == 
			m_mesh_infos[i].id)
			selMesh = result;
	}

	for(int i = 0; i < (int)m_mg_infos.size(); ++i)
	{
		int result = m_mgs->Append( wxString(m_mg_infos[i].name.c_str(), wxConvUTF8), (void*)i);
		if( m_ctx->GetEntity()->GetMotionGraph() && m_ctx->GetEntity()->GetMotionGraph()->GetID() == 
			m_mg_infos[i].id)
			selMg = result;
	}

	m_meshes->SetSelection(selMesh);
	m_mgs->SetSelection(selMg);
}


void mogedChangeSkeleton::OnChooseSkeleton( wxCommandEvent& event )
{
	(void)event;
	int sel_idx = m_skeletons->GetSelection();
	if(sel_idx == wxNOT_FOUND) RefreshViewFromSkel(0);
	int info_idx = (int)m_skeletons->GetClientData(sel_idx);
	RefreshViewFromSkel(m_skel_infos[info_idx].id);
}

void mogedChangeSkeleton::OnDeleteSkeleton( wxCommandEvent& event ) 
{
	(void)event;
	int sel_idx = m_skeletons->GetSelection();
	if(sel_idx == wxNOT_FOUND) return;
	int info_idx = (int)m_skeletons->GetClientData(sel_idx);
	
	wxString str = _("Are you sure you want to delete the skeleton \"") ;
	str << wxString( m_skel_infos[info_idx].name.c_str(), wxConvUTF8 ) ;
	str << _("\"? There's no going back.");

	wxMessageDialog dlg(this, str, _("Confirm"), wxYES_NO|wxICON_QUESTION);
	if(dlg.ShowModal() == wxID_YES) {
		m_ctx->GetEntity()->DeleteSkeleton( m_skel_infos[info_idx].id );
		RefreshView();
	}
}

void mogedChangeSkeleton::OnDeleteMotionGraph( wxCommandEvent& event )
{
	(void)event;
	int sel_idx = m_mgs->GetSelection();
	if(sel_idx == wxNOT_FOUND) return;
	int info_idx = (int)m_mgs->GetClientData(sel_idx);

	wxString str = _("Are you sure you want to delete the motion graph \"") ;
	str << wxString( m_mg_infos[info_idx].name.c_str(), wxConvUTF8 ) ;
	str << _("\"? There's no going back.");

	wxMessageDialog dlg(this, str, _("Confirm"), wxYES_NO|wxICON_QUESTION);
	if(dlg.ShowModal() == wxID_YES) {
		m_ctx->GetEntity()->DeleteMotionGraph( m_mg_infos[info_idx].id );
		RefreshView();
	}
}

void mogedChangeSkeleton::OnDeleteMesh( wxCommandEvent& event )
{
	(void)event;
	int sel_idx = m_meshes->GetSelection();
	if(sel_idx == wxNOT_FOUND) return;
	int info_idx = (int)m_meshes->GetClientData(sel_idx);

	wxString str = _("Are you sure you want to delete the mesh \"") ;
	str << wxString( m_mesh_infos[info_idx].name.c_str(), wxConvUTF8 ) ;
	str << _("\"? There's no going back.");

	wxMessageDialog dlg(this, str, _("Confirm"), wxYES_NO|wxICON_QUESTION);
	if(dlg.ShowModal() == wxID_YES) {
		m_ctx->GetEntity()->DeleteMotionGraph( m_mesh_infos[info_idx].id );
		RefreshView();
	}
}

void mogedChangeSkeleton::OnApply( wxCommandEvent& event )
{
	(void)event;
	int sel_idx = m_skeletons->GetSelection();
	if(sel_idx != wxNOT_FOUND) 
	{
		int info_idx = (int)m_skeletons->GetClientData(sel_idx);
		m_ctx->GetEntity()->SetCurrentSkeleton( m_skel_infos[info_idx].id );
	}

	sel_idx = m_meshes->GetSelection();
	if(sel_idx != wxNOT_FOUND) 
	{
		int info_idx = (int)m_meshes->GetClientData(sel_idx);
		m_ctx->GetEntity()->SetCurrentMesh( m_mesh_infos[info_idx].id );
	}

	sel_idx = m_mgs->GetSelection();
	if(sel_idx != wxNOT_FOUND) 
	{
		int info_idx = (int)m_mgs->GetClientData(sel_idx);
		m_ctx->GetEntity()->SetCurrentMotionGraph( m_mg_infos[info_idx].id );
	}
	
	EndModal(wxID_OK);
}

