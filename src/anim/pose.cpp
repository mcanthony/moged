#include "pose.hh"
#include "skeleton.hh"

Pose::Pose(const Skeleton* skel)
	: m_count(skel->GetNumJoints())
	, m_root_offset(0,0,0)
	, m_root_rotation(0,0,0,1)
	, m_offsets(0)
	, m_rotations(0)
{
	m_offsets = new Vec3[m_count];
	m_rotations = new Quaternion[m_count];	
}

Pose::~Pose()
{
	delete[] m_offsets;
	delete[] m_rotations;
}

void Pose::RestPose(const Skeleton* skel /*, Mat4_arg matModelToWorld */) 
{
	if(skel->GetNumJoints() != m_count)
		return;

	Vec3 root_offset = skel->GetRootOffset();
	Quaternion root_rotation = skel->GetRootRotation();
	m_root_offset = root_offset;
	m_root_rotation = root_rotation;

	int num_joints = m_count;

	for(int i = 0; i < num_joints; ++i)
	{			
		int parent = skel->GetJointParent(i);
		Vec3 offset = skel->GetJointTranslation(i);
		Quaternion rot = skel->GetJointOrientation(i);
			
		if(parent == -1) { 
			m_rotations[i] = root_rotation * rot;
			m_offsets[i] = root_offset + rotate(offset, m_rotations[i]);; 
		} else {
			m_rotations[i] = m_rotations[parent] * rot;
			m_offsets[i] = m_offsets[parent] + rotate(offset,m_rotations[parent]*rot);
		}
	}
}
