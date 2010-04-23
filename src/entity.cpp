#include "entity.hh"
#include "skeleton.hh"
#include "clipdb.hh"
#include "weightedmesh.hh"

Entity::Entity()
	: m_skeleton(0)
	, m_clips(0)
	, m_mesh(0)
	, m_name()
{
}

Entity::~Entity()
{
	delete m_skeleton;
	delete m_clips;
	delete m_mesh;
}

void Entity::SetSkeleton( const Skeleton* skel, ClipDB* clips )
{
	if(m_skeleton) {
		delete m_skeleton;
		delete m_clips;
		delete m_mesh; m_mesh = 0;
	}

	m_skeleton = skel;
	m_clips = clips;
}

bool Entity::SetMesh(const WeightedMesh* mesh )
{
	if(m_skeleton == 0) {
		return false;
	}

	// TODO: find range of mesh skinning mat indices, see if it will fit with the current skeleton.
	
	if(m_mesh) {
		delete m_mesh;
		m_mesh = 0;
	}
	m_mesh = mesh;
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// serialization funcs
////////////////////////////////////////////////////////////////////////////////

bool saveEntity(const Entity* entity)
{
	return false;
}

Entity* loadEntity(const char* filename)
{
	return new Entity;
}
