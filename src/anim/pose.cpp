#include <cstdio>
#include "pose.hh"
#include "skeleton.hh"
#include "Mat4.hh"
#include "MathUtil.hh"

using namespace std;

Pose::Pose(const Skeleton* skel)
	: m_count(skel->GetNumJoints())
	, m_root_offset(0,0,0)
	, m_root_rotation(0,0,0,1)
	, m_offsets(0)
	, m_rotations(0)
	, m_mats(0)
{
	m_offsets = new Vec3[m_count];
	m_rotations = new Quaternion[m_count];	
	m_mats = new Mat4[m_count];
}

Pose::~Pose()
{
	delete[] m_offsets;
	delete[] m_rotations;
	delete[] m_mats;
}

void Pose::RestPose(const Skeleton* skel )
{
	if(skel->GetNumJoints() != m_count)
		return;

	const int num_joints = m_count;
	for(int i = 0; i < num_joints; ++i)
	{			
		m_rotations[i] = skel->GetJointToSkelRotation(i);
		m_offsets[i] = skel->GetJointToSkelOffset(i);
	}
}

void Pose::ComputeMatrices(const Skeleton* skel)
{
	const int num_joints = m_count;
	for(int i = 0; i < num_joints; ++i) {
		Mat4 anim_joint_to_model = translation( m_offsets[i] ) * m_rotations[i].to_matrix();
		Mat4 skel_model_to_joint = /* inv bind rotation * */ 
			skel->GetSkelToJointTransform(i);		
		Mat4 mat = anim_joint_to_model * skel_model_to_joint;
		m_mats[i] = mat;
	}
}
