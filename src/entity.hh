#ifndef INCLUDED_moged_entity_HH
#define INCLUDED_moged_entity_HH

#include <string>

class Skeleton;
class WeightedMesh;
class ClipDB;

////////////////////////////////////////////////////////////////////////////////
// entity - holder/owner of all working data we care about!
////////////////////////////////////////////////////////////////////////////////
class Entity
{
	const Skeleton* m_skeleton;
	ClipDB* m_clips;
	const WeightedMesh* m_mesh;
	// std::vector< MgNode* > m_nodes;
	std::string m_name;
	
public:
	Entity() ;
	~Entity() ;

	// Filename of entity
	void SetName(const char* name) { m_name = name; }
	const char* GetName() const { return m_name.c_str(); }

	void SetSkeleton( const Skeleton* skel, ClipDB* clips );
	bool SetMesh(const WeightedMesh* mesh );

	const Skeleton* GetSkeleton() const { return m_skeleton; }
	ClipDB* GetClips() { return m_clips; }
	const WeightedMesh* GetMesh() { return m_mesh; }
};

////////////////////////////////////////////////////////////////////////////////
// serialization funcs
////////////////////////////////////////////////////////////////////////////////

bool saveEntity(const Entity* entity); 
Entity* loadEntity(const char* filename);

#endif