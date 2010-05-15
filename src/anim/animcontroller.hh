#ifndef INCLUDED_anim_animcontroller_HH
#define INCLUDED_anim_animcontroller_HH

#include "Mat4.hh"

class Pose;
class Skeleton;

class AnimController
{
protected:
	const Skeleton* m_skel;
	Pose* m_pose;
public:
	AnimController(const Skeleton* skel) ;
	virtual ~AnimController() ;
	virtual void ComputePose() = 0 ;

	void ComputeMatrices( Mat4_arg model_to_skel );

	Pose* GetPose() { return m_pose; }
	const Pose* GetPose() const { return m_pose; }	
	const Skeleton* GetSkeleton() const { return m_skel; }
};

#endif
