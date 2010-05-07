#ifndef __mogedChangeSkeleton__
#define __mogedChangeSkeleton__

/**
@file
Subclass of ChangeSkeleton, which is generated by wxFormBuilder.
*/

#include <vector>
#include "MogedFrames.h"
#include "entity.hh"

class AppContext ;

/** Implementing ChangeSkeleton */
class mogedChangeSkeleton : public ChangeSkeleton
{
	AppContext* m_ctx;
	
	std::vector<SkeletonInfo> m_skel_infos;
	std::vector<MeshInfo> m_mesh_infos;
	std::vector<MotionGraphInfo> m_mg_infos;

	void RefreshView();
	void RefreshViewFromSkel(sqlite3_int64 skel);
protected:
	// Handlers for ChangeSkeleton events.
	void OnDeleteSkeleton( wxCommandEvent& event ) ;
	void OnDeleteMotionGraph( wxCommandEvent& event );
	void OnDeleteMesh( wxCommandEvent& event );
	void OnChooseSkeleton( wxCommandEvent& event );

	void OnApply( wxCommandEvent& event );
	
public:
	/** Constructor */
	mogedChangeSkeleton( wxWindow* parent, AppContext* ctx );
};

#endif // __mogedChangeSkeleton__
