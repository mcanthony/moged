#ifndef INCLUDED_skeleton_HH
#define INCLUDED_skeleton_HH

#include <string>
#include "Vector.hh"
#include "Quaternion.hh"
#include "NonCopyable.hh"

namespace LBF { class WriteNode; class ReadNode; }

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

	// parent info for posing
	int* m_parents;
		
	Vec3* m_full_translations;
	Quaternion* m_full_rotations;
public:

	explicit Skeleton(int num_joints);
	~Skeleton();

	int GetNumJoints() const { return m_num_joints; }
	void ComputeTransforms(); // call once after structure is initialized
	
	void SetName(const char* name) { m_name = name; }
	void SetName( const std::string& name) { m_name = name; }
	const char* GetName() const { return m_name.c_str(); }

	void SetRootOffset(Vec3_arg v) { m_root_offset = v; }
	const Vec3& GetRootOffset() const { return m_root_offset; }

	void SetRootRotation(Quaternion_arg q) { m_root_rotation = q; }
	const Quaternion& GetRootRotation() const { return m_root_rotation; }

	const Vec3* GetJointTranslations() const { return m_translations; }
	const int* GetParents() const { return m_parents; }

	Vec3& GetJointTranslation(int idx) ;
	const Vec3& GetJointTranslation(int idx) const ;

	const char* GetJointName(int idx) const ;
	void SetJointName(int idx, const char* name );

	void SetJointParent(int idx, int parent) ;
	int GetJointParent(int idx) const;

	Mat4 GetSkelToJointTransform(int idx) const;
	const Vec3& GetJointToSkelOffset(int idx) const;
	const Quaternion& GetJointToSkelRotation(int idx) const;

	LBF::WriteNode* CreateSkeletonWriteNode( ) const;
	// 'factory' method for loading a skeleton. needs to be a member so string table loading isn't too stupid
	static Skeleton* CreateSkeletonFromReadNode( const LBF::ReadNode& rn );
};

// changeable metadata for the skeleton - needs to not be constant, so it is in a separate class
class SkeletonWeights : non_copyable
{
	int m_num_joints;
	float *m_weights;
public:
	SkeletonWeights(int num_joints);
	~SkeletonWeights();
	void SetJointWeight(int idx, float weight) ;
	float GetJointWeight(int idx) const ;

	LBF::WriteNode* CreateWriteNode() const ;
	static SkeletonWeights* CreateFromReadNode( const LBF::ReadNode& rn );

};

#endif
