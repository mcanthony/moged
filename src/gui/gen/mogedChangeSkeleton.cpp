#include "mogedChangeSkeleton.h"
#include "appcontext.hh"

mogedChangeSkeleton::mogedChangeSkeleton( wxWindow* parent, AppContext *ctx )
:
ChangeSkeleton( parent )
, m_ctx(ctx)
{
	// get list of skeletons from entity
}

void mogedChangeSkeleton::OnChange( wxCommandEvent& event )
{
	(void)event;
	// set current skeleton.
}

void mogedChangeSkeleton::OnCancel( wxCommandEvent& event )
{
	(void)event;
	// TODO: Implement OnCancel
}
