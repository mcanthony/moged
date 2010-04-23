#ifndef INCLUDED_skeleton_HH
#define INCLUDED_skeleton_HH

#include <string>
#include "Vector.hh"
#include "Quaternion.hh"
#include "NonCopyable.hh"

class Skeleton : non_copyable
{
	std::string m_name;
	int m_num_joints;
	std::string *m_joint_names;

	// local root transform.
	Vec3 m_root_offset;
	Quaternion m_root_rotation;

	// local transforms that define the structure of the skeleton
	Vec3* m_translations;
	Quaternion* m_rotations;

	// parent info for posing
	int* m_parents;
		
public:
	explicit Skeleton(int num_joints);
	~Skeleton();

	int GetNumJoints() const { return m_num_joints; }
	
	void SetName(const char* name) { m_name = name; }
	const char* GetName() const { return m_name.c_str(); }

	void SetRootOffset(Vec3_arg v) { m_root_offset = v; }
	const Vec3& GetRootOffset() const { return m_root_offset; }

	void SetRootRotation(Quaternion_arg q) { m_root_rotation = q; }
	const Quaternion& GetRootRotation() const { return m_root_rotation; }

	Vec3& GetJointTranslation(int idx) ;
	const Vec3& GetJointTranslation(int idx) const ;

	Quaternion& GetJointOrientation(int idx) ;
	const Quaternion& GetJointOrientation(int idx) const ;

	const char* GetJointName(int idx) const ;
	void SetJointName(int idx, const char* name );

	void SetJointParent(int idx, int parent) ;
	int GetJointParent(int idx) const;
};

#endif
