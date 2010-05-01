#include "mogedMotionGraphEditor.h"

mogedMotionGraphEditor::mogedMotionGraphEditor( wxWindow* parent, AppContext* ctx )
:
MotionGraphEditor( parent )
, m_ctx(ctx)
{

}

void mogedMotionGraphEditor::OnIdle( wxIdleEvent& event )
{
	// TODO: Implement OnIdle
}

void mogedMotionGraphEditor::OnPageChanged( wxListbookEvent& event )
{
	// TODO: Implement OnPageChanged
}

void mogedMotionGraphEditor::OnPageChanging( wxListbookEvent& event )
{
	// TODO: Implement OnPageChanging
}

void mogedMotionGraphEditor::OnScrollErrorThreshold( wxScrollEvent& event )
{
	// TODO: Implement OnScrollErrorThreshold
}

void mogedMotionGraphEditor::OnEditErrorThreshold( wxCommandEvent& event )
{
	// TODO: Implement OnEditErrorThreshold
}

void mogedMotionGraphEditor::OnScrollCloudSampleRate( wxScrollEvent& event )
{
	// TODO: Implement OnScrollCloudSampleRate
}

void mogedMotionGraphEditor::OnEditCloudSampleRate( wxCommandEvent& event )
{
	// TODO: Implement OnEditCloudSampleRate
}

void mogedMotionGraphEditor::OnCreate( wxCommandEvent& event )
{
	// TODO: Implement OnCreate
}

void mogedMotionGraphEditor::OnCancel( wxCommandEvent& event )
{
	// TODO: Implement OnCancel
}

void mogedMotionGraphEditor::OnPause( wxCommandEvent& event )
{
	// TODO: Implement OnPause
}

void mogedMotionGraphEditor::OnNext( wxCommandEvent& event )
{
	// TODO: Implement OnNext
}

void mogedMotionGraphEditor::OnContinue( wxCommandEvent& event )
{
	// TODO: Implement OnContinue
}

void mogedMotionGraphEditor::OnNextStage( wxCommandEvent& event )
{
	// TODO: Implement OnNextStage
}
