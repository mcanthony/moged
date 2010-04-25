#include <cstdio>
#include "entity.hh"
#include "skeleton.hh"
#include "clipdb.hh"
#include "weightedmesh.hh"
#include "lbfloader.hh"

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
	if(entity->GetName()[0] == '\0') {
		return false;
	}
	
	LBF::WriteNode* objSectionNode = new LBF::WriteNode(LBF::OBJECT_SECTION, 0, 0);
	LBF::WriteNode* animSectionNode = new LBF::WriteNode(LBF::ANIM_SECTION, 0, 0);
	
	objSectionNode->AddSibling(animSectionNode);

	int err = LBF::saveLBF( entity->GetName(), objSectionNode, true );
	delete objSectionNode;
	if(err == 0) {
		return true;
	} else {
		fprintf(stderr, "Failed with error code %d\n", err);
		// error report?
		return false;
	}
}

Entity* loadEntity(const char* filename)
{
	LBF::LBFData* file;
	int err = LBF::openLBF( filename, file );
	if(err != 0) {
		fprintf(stderr, "Failed with error code %d\n", err);
		return new Entity;
	}
	
	LBF::ReadNode rnObj = file->GetFirstNode(LBF::OBJECT_SECTION);
	LBF::ReadNode rnAnim = file->GetFirstNode(LBF::ANIM_SECTION);

	if(rnObj.Valid()) {
		printf("Found object section!\n");
	}

	if(rnAnim.Valid()) {
		printf("Found animation section!\n");
	}

	delete file;
	return new Entity;
	
}
