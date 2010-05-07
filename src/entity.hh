#ifndef INCLUDED_moged_entity_HH
#define INCLUDED_moged_entity_HH

#include <string>
#include <vector>
#include "NonCopyable.hh"

class Skeleton;
class SkeletonWeights;
class Mesh;
class ClipDB;
class MotionGraph;

struct sqlite3 ;
typedef long long int sqlite3_int64;

namespace Events { class EventSystem; }

////////////////////////////////////////////////////////////////////////////////
// DB helper classes 
////////////////////////////////////////////////////////////////////////////////
struct MeshInfo {
	std::string name;
	sqlite3_int64 id;
};

struct MotionGraphInfo {
	std::string name;
	sqlite3_int64 id;
};

struct SkeletonInfo {
	std::string name;
	sqlite3_int64 id;
};
	
////////////////////////////////////////////////////////////////////////////////
// entity - holder/owner of all working data we care about!
////////////////////////////////////////////////////////////////////////////////
class Entity : non_copyable
{
	Events::EventSystem* m_evsys;
	sqlite3* m_db;
	std::string m_filename;

	Skeleton* m_skeleton;
	ClipDB* m_clips;
	const Mesh* m_mesh;
	MotionGraph *m_mg;

public:
	Entity(Events::EventSystem* evsys) ;
	~Entity() ;

	void SetFilename(const char* filename);
	const char *GetFilename() { return m_filename.c_str(); };

	sqlite3_int64 GetCurrentSkeleton() const ;
	void SetCurrentSkeleton(sqlite3_int64 id);	
	void SetCurrentMesh(sqlite3_int64 id);
	void SetCurrentMotionGraph(sqlite3_int64 id);
	void Reset();

	// HasDB() must be true or no other operations will have any effect.
	bool HasDB() const { return m_db != 0; }
	sqlite3* GetDB() { return m_db; }

	const Skeleton* GetSkeleton() const { return m_skeleton; }
	ClipDB *GetClips() { return m_clips; }
	const ClipDB *GetClips() const { return m_clips; }
	const Mesh* GetMesh() const { return m_mesh; }
	MotionGraph* GetMotionGraph() { return m_mg; }
	const MotionGraph* GetMotionGraph() const { return m_mg; }

	void GetSkeletonInfos(std::vector<SkeletonInfo>& out) const;
	void GetMotionGraphInfos(std::vector<MotionGraphInfo>& out, sqlite3_int64 skel_id) const;
	void GetMeshInfos(std::vector<MeshInfo>& out, sqlite3_int64 skel_id) const;
	void DeleteSkeleton(sqlite3_int64 skel_id);
	void DeleteMesh(sqlite3_int64 mesh_id);
	void DeleteMotionGraph(sqlite3_int64 mg_id);

private:
	void CreateMissingTables();
	bool FindFirstSkeleton(sqlite3_int64 *skel_id);
	bool FindFirstMesh(sqlite3_int64 skel_id, sqlite3_int64* mesh_id);
	bool FindFirstMotionGraph(sqlite3_int64 skel_id, sqlite3_int64* mg_id);

};

////////////////////////////////////////////////////////////////////////////////
// serialization funcs
////////////////////////////////////////////////////////////////////////////////

bool exportEntityLBF(const Entity* entity, const char* filename); 
bool importEntityLBF(Entity* target, const char* filename);

#endif
