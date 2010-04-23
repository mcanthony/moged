#include "skeleton.hh"
#include "assert.hh"

Skeleton::Skeleton(int num_joints)
	: m_num_joints(num_joints)
	, m_root_offset(0,0,0)
	, m_root_rotation(0,0,0,1) // no rotation
	, m_translations(0)
	, m_rotations(0)
	, m_parents(0)
{
	m_joint_names = new std::string[num_joints];
	m_translations = new Vec3[num_joints];
	m_rotations = new Quaternion[num_joints];
	m_parents = new int[num_joints];
	
	memset(m_translations, 0, sizeof(Vec3)*num_joints);
	memset(m_rotations, 0, sizeof(Quaternion)*num_joints);
	memset(m_parents, -1, sizeof(int)*num_joints);
}

Skeleton::~Skeleton()
{
	delete[] m_joint_names;
	delete[] m_translations;
	delete[] m_rotations;
	delete[] m_parents;
}

Vec3& Skeleton::GetJointTranslation(int idx) 
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	return m_translations[idx];
}

const Vec3& Skeleton::GetJointTranslation(int idx) const 
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	return m_translations[idx];
}

Quaternion& Skeleton::GetJointOrientation(int idx) 
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	return m_rotations[idx];
}

const Quaternion& Skeleton::GetJointOrientation(int idx) const 
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	return m_rotations[idx];
}

const char* Skeleton::GetJointName(int idx)  const
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	return m_joint_names[idx].c_str();
}

void Skeleton::SetJointName(int idx, const char* name )
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	m_joint_names[idx] = name;
}

void Skeleton::SetJointParent(int idx, int parent) 
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	ASSERT(parent >= -1 && parent < m_num_joints);
	m_parents[idx] = parent;
}

int Skeleton::GetJointParent(int idx) const
{
	ASSERT(idx >= 0 && idx < m_num_joints);
	return m_parents[idx];
}
