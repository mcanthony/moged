#include "mogedJointWeightEditor.h"
#include "appcontext.hh"
#include "entity.hh"
#include "skeleton.hh"

mogedJointWeightEditor::mogedJointWeightEditor( wxWindow* parent, AppContext* ctx )
  : JointWeightEditor( parent )
  , m_ctx(ctx)
  , m_selected(0)
{
	m_bone_grid->SetColFormatFloat(1);
}

mogedJointWeightEditor::~mogedJointWeightEditor()
{
	delete[] m_selected;
}

void mogedJointWeightEditor::HandleEvent(Events::Event* ev)
{
	using namespace Events;
	if(ev->GetType() == EventID_EntitySkeletonChangedEvent) {
		RefreshJointList();
	}
}

void mogedJointWeightEditor::RefreshJointList()
{
	m_bone_grid->ClearGrid();

	int num_rows = m_bone_grid->GetNumberRows();
	m_bone_grid->DeleteRows(0, num_rows);

	delete[] m_selected; m_selected = 0;

	const Skeleton* skel = m_ctx->GetEntity()->GetSkeleton();
	SkeletonWeights* weights = m_ctx->GetEntity()->GetSkeletonWeights();
	if(skel && weights)
	{
		num_rows = skel->GetNumJoints();
		m_selected = new int[num_rows];
		memset(m_selected,0,sizeof(int)*num_rows);

		m_bone_grid->InsertRows(0,num_rows);

		for(int i = 0; i < num_rows; ++i) {
			wxString name = wxString(skel->GetJointName(i), wxConvUTF8);
			m_bone_grid->SetRowLabelValue(i, name);
			float w = weights->GetJointWeight(i);
			wxString cellValue ;
			cellValue << w;
			m_bone_grid->SetCellValue(i,0, cellValue);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
void mogedJointWeightEditor::OnChangeCell( wxGridEvent& event )
{
	int idx = event.GetRow();

	float w = atof( m_bone_grid->GetCellValue(idx, 0).char_str());
	
	SkeletonWeights* weights = m_ctx->GetEntity()->GetSkeletonWeights();
	if(weights)
	{
		weights->SetJointWeight(idx, w);
	}
	event.Skip();
}

void mogedJointWeightEditor::OnSelectCell( wxGridEvent& event ) 
{
	int idx = event.GetRow();
	m_selected [ idx ] = 1;

	Events::SelectBoneEvent ev;
	ev.BoneIndex = idx;
	ev.Selected = 1;
	m_ctx->GetEventSystem()->Send(&ev);

	event.Skip();
}

void mogedJointWeightEditor::OnSelectRange( wxGridRangeSelectEvent& event ) 
{
	if(event.Selecting()) {
		for(int i = event.GetTopRow(); i <= event.GetBottomRow(); ++i)
		{
			if(!m_selected[i]) {
				m_selected[i] = 1;
				Events::SelectBoneEvent ev;
				ev.BoneIndex = i;
				ev.Selected = 1;
				m_ctx->GetEventSystem()->Send(&ev);
			}
		}
	} else {
		for(int i = event.GetTopRow(); i <= event.GetBottomRow(); ++i)
		{
			if(m_selected[i]) {
				m_selected[i] = 0;
				Events::SelectBoneEvent ev;
				ev.BoneIndex = i;
				ev.Selected = 0;
				m_ctx->GetEventSystem()->Send(&ev);
			}
		}
	}
	event.Skip();
}
