#ifndef INCLUDED_skeleton_HH
#define INCLUDED_skeleton_HH

#include <string>
#include "Vector.hh"
#include "Quaternion.hh"
#include "NonCopyable.hh"
#include "dbhelpers.hh"

namespace LBF { class WriteNode; class ReadNode; }

class SkeletonWeights : non_copyable
{
	struct sqlite3* m_db;
	sqlite3_int64 m_skel_id;

	Query m_set_statement;

	////////////////////////////////////////
	// cached values
	int m_num_weights;
	float *m_cached_weights;
public:
	SkeletonWeights(struct sqlite3* db, sqlite3_int64 skel_id);
	~SkeletonWeights();

	void SetJointWeight(int idx, float weight) ;
	float GetJointWeight(int idx) const ;

	LBF::WriteNode* CreateWriteNode() const ;
	void ImportFromReadNode( const LBF::ReadNode& rn );
};

class Skeleton : non_copyable
{
	sqlite3* m_db;
	sqlite3_int64 m_id;

	////////////////////////////////////////
	// the following data is read only so is cached on creation.
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

	mutable SkeletonWeights m_weights;
public:

	explicit Skeleton(sqlite3* db, sqlite3_int64 id);
	~Skeleton();

	bool Valid() const { return m_id != 0; }
	sqlite3_int64 GetID() const { return m_id; }

	int GetNumJoints() const { return m_num_joints; }
	
	void SetName(const char* name) { m_name = name; }
	void SetName( const std::string& name) { m_name = name; }
	const char* GetName() const { return m_name.c_str(); }

	void SetRootOffset(Vec3_arg v) { m_root_offset = v; }
	const Vec3& GetRootOffset() const { return m_root_offset; }

	void SetRootRotation(Quaternion_arg q) { m_root_rotation = q; }
	const Quaternion& GetRootRotation() const { return m_root_rotation; }

	const Vec3* GetJointTranslations() const { return m_translations; }
	const int* GetParents() const { return m_parents; }

	const Vec3& GetJointTranslation(int idx) const ;
	const char* GetJointName(int idx) const ;
	int GetJointParent(int idx) const;

	Mat4 GetSkelToJointTransform(int idx) const;
	const Vec3& GetJointToSkelOffset(int idx) const;
	const Quaternion& GetJointToSkelRotation(int idx) const;
	
	// sort of strange api: returning a mutable portion of the skeleton, so is available to a const skel
	SkeletonWeights& GetSkeletonWeights() const { return m_weights; }

	LBF::WriteNode* CreateSkeletonWriteNode( ) const;
	static sqlite3_int64 ImportFromReadNode( sqlite3* db, const LBF::ReadNode& rn );

private:
	void ComputeTransforms(); // call once after structure is initialized
	bool LoadFromDB();
};

#endif
