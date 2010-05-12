#include "animcontroller.hh"
#include "pose.hh"

AnimController::AnimController(const Skeleton* skel) 
  : m_skel(skel)
  , m_pose(0)
{
	m_pose = new Pose(skel);
}

AnimController::~AnimController() 
{
	delete m_pose;
}

void AnimController::ComputeMatrices( Mat4_arg model_to_skel )
{
	m_pose->ComputeMatrices(m_skel, model_to_skel);
}
