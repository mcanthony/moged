#ifndef INCLUDED_moged_pose_HH
#define INCLUDED_moged_pose_HH

#include "Vector.hh"
#include "Quaternion.hh"

class Skeleton;

class Pose
{
	int m_count;
	Vec3 m_root_offset;
	Quaternion m_root_rotation;
	Vec3 *m_offsets;
	Quaternion* m_rotations;
public:	
	Pose(const Skeleton* skel);
	~Pose();

	void RestPose(const Skeleton* skel) ;

	int GetNumJoints() const { return m_count; }

	void SetRootOffset(Vec3_arg v) { m_root_offset = v; }
	const Vec3& GetRootOffset() const { return m_root_offset; }
	
	void SetRootRotation(Quaternion_arg q) { m_root_rotation = q; }
	const Quaternion& GetRootRotation() const { return m_root_rotation; }

	Vec3* GetOffsets() { return m_offsets; }
	const Vec3* GetOffsets() const { return m_offsets; }

	Quaternion* GetRotations() { return m_rotations; }
	const Quaternion* GetRotations() const { return m_rotations; }
};

#endif
