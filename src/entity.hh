#ifndef INCLUDED_moged_entity_HH
#define INCLUDED_moged_entity_HH

#include <string>

class Skeleton;
class Mesh;
class ClipDB;
class MotionGraph;

////////////////////////////////////////////////////////////////////////////////
// entity - holder/owner of all working data we care about!
////////////////////////////////////////////////////////////////////////////////
class Entity
{
	const Skeleton* m_skeleton;
	ClipDB* m_clips;
	const Mesh* m_mesh;
	// std::vector< MgNode* > m_nodes;
	std::string m_name;
	MotionGraph *m_mg;
public:
	Entity() ;
	~Entity() ;

	// Filename of entity
	void SetName(const char* name) { m_name = name; }
	const char* GetName() const { return m_name.c_str(); }

	void SetSkeleton( const Skeleton* skel, ClipDB* clips );
	bool SetMesh(const Mesh* mesh );

	const Skeleton* GetSkeleton() const { return m_skeleton; }
	ClipDB* GetClips() { return m_clips; }
	const ClipDB* GetClips() const { return m_clips; }

	const Mesh* GetMesh() const { return m_mesh; }

	void SetMotionGraph(MotionGraph *g);
	MotionGraph* GetMotionGraph() { return m_mg; }
	const MotionGraph* GetMotionGraph() const { return m_mg; }
};

////////////////////////////////////////////////////////////////////////////////
// serialization funcs
////////////////////////////////////////////////////////////////////////////////

bool saveEntity(const Entity* entity); 
Entity* loadEntity(const char* filename);

#endif
