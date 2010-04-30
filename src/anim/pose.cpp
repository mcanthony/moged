#include "pose.hh"
#include "skeleton.hh"
#include "Mat4.hh"

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

	Quaternion root_rotation = skel->GetRootRotation();
	Vec3 root_offset = skel->GetRootOffset();
	m_root_offset = root_offset;
	m_root_rotation = root_rotation;

	const int num_joints = m_count;

	for(int i = 0; i < num_joints; ++i)
	{			
		int parent = skel->GetJointParent(i);
		// local_rot = skelrestRot * anim * invSkelRestRot
		// but anim is identity, so no rotation is applied.
			
		if(parent == -1) { 
			m_rotations[i] = root_rotation ;
			m_offsets[i] = root_offset ;
		} else {
			Vec3 local_offset = skel->GetJointTranslation(parent);

			m_rotations[i] = m_rotations[parent] ;
			m_offsets[i] = m_offsets[parent] + rotate(local_offset, m_rotations[parent]);
		}
	}
}

void Pose::ComputeMatrices()
{
	const int num_joints = m_count;
	for(int i = 0; i < num_joints; ++i) {
		Mat4 mat = translation(m_offsets[i]) * transpose(m_rotations[i].to_matrix());// * translation( -m_offsets[i] );
		m_mats[i] = mat;
	}
}
