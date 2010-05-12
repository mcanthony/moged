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
	, m_local_rotations(0)
	, m_mats(0)
{
	m_offsets = new Vec3[m_count];
	m_rotations = new Quaternion[m_count];	
	m_local_rotations = new Quaternion[m_count];
	m_mats = new Mat4[m_count];
	
	memset(m_offsets, 0, sizeof(Vec3)*m_count);
	memset(m_rotations, 0, sizeof(Quaternion)*m_count);
	memset(m_local_rotations, 0, sizeof(Quaternion)*m_count);
	memset(m_mats, 0, sizeof(Mat4)*m_count);
}

Pose::~Pose()
{
	delete[] m_offsets;
	delete[] m_rotations;
	delete[] m_local_rotations;
	delete[] m_mats;
}

void Pose::RestPose(const Skeleton* skel )
{
	if(skel->GetNumJoints() != m_count)
		return;

	m_root_offset.set(0,0,0);
	m_root_rotation.set(0,0,0,1);

	const int num_joints = m_count;
	for(int i = 0; i < num_joints; ++i)
	{			
		m_local_rotations[i] = Quaternion(0,0,0,1);
	}
}

void Pose::ComputeMatrices(const Skeleton* skel, Mat4_arg model_to_skel)
{
	const int num_joints = m_count;

	const int* parents = skel->GetParents();
	const Vec3 *skel_rest_offsets = skel->GetJointTranslations();

	// flatten hierarchy
	for(int i = 0; i < num_joints; ++i) {
		int parent = parents[i];
		if(parent == -1) {
			m_rotations[i] = m_root_rotation * m_local_rotations[i];
			m_offsets[i] = m_root_offset ;
		} else {
			m_rotations[i] = m_rotations[parent] * m_local_rotations[i];
			m_offsets[i] = m_offsets[parent] + rotate(skel_rest_offsets[parent], m_rotations[parent]);
		}
	}			

	// and build render friendly matrices
	for(int i = 0; i < num_joints; ++i) {
		Mat4 anim_joint_to_model = translation( m_offsets[i] ) * m_rotations[i].to_matrix();
		Mat4 skel_to_joint = skel->GetSkelToJointTransform(i);		
		Mat4 mat = anim_joint_to_model * skel_to_joint * model_to_skel;
		m_mats[i] = mat;
	}
}
