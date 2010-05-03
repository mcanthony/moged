#include <cstdio>
#include "entity.hh"
#include "skeleton.hh"
#include "clipdb.hh"
#include "mesh.hh"
#include "lbfloader.hh"
#include "motiongraph.hh"

Entity::Entity()
	: m_skeleton(0)
	, m_skeleton_weights(0)
	, m_clips(0)
	, m_mesh(0)
	, m_name()
	, m_mg(0)
{
}

Entity::~Entity()
{
	delete m_skeleton;
	delete m_skeleton_weights;
	delete m_clips;
	delete m_mesh;
	delete m_mg;
}

void Entity::SetSkeleton( const Skeleton* skel, ClipDB* clips, SkeletonWeights* weights )
{
	if(m_skeleton) {
		delete m_skeleton_weights;
		delete m_skeleton;
		delete m_clips;
//		delete m_mesh; m_mesh = 0;
	}

	m_skeleton = skel;
	m_clips = clips;
	m_skeleton_weights = weights;
	if(m_skeleton_weights == 0) {
		m_skeleton_weights = new SkeletonWeights(skel->GetNumJoints());
	}
}

bool Entity::SetMesh(const Mesh* mesh )
{
	// TODO: find range of mesh skinning mat indices, see if it will fit with the current skeleton.	
	if(m_mesh) {
		delete m_mesh; m_mesh = 0;
	}
	m_mesh = mesh;
	return true;
}

void Entity::SetMotionGraph(MotionGraph *g)
{
	if(m_mg) {
		delete m_mg; m_mg = 0;
	}
	m_mg = g;
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
	if(entity->GetMesh())
	{
		LBF::WriteNode* geom3d = entity->GetMesh()->CreateMeshWriteNode();
		if(geom3d) {
			objSectionNode->AddChild(geom3d);
		}
	}

	if(entity->GetSkeleton())
	{
		LBF::WriteNode* skelNode = entity->GetSkeleton()->CreateSkeletonWriteNode();
		if(skelNode) {
			LBF::WriteNode* skelWeightsNode = entity->GetSkeletonWeights()->CreateWriteNode();
			if(skelWeightsNode) {
				skelNode->AddChild(skelWeightsNode);
			}
			animSectionNode->AddChild(skelNode);
		}
	}

	if(entity->GetClips())
	{
		LBF::WriteNode* clipsNode = createClipsWriteNode( entity->GetClips() );
		if(clipsNode) {
			animSectionNode->AddChild(clipsNode);
		}
	}

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
		return 0;
	}
	
	LBF::ReadNode rnObj = file->GetFirstNode(LBF::OBJECT_SECTION);
	LBF::ReadNode rnAnim = file->GetFirstNode(LBF::ANIM_SECTION);


	Entity* entity = new Entity;
	entity->SetName(filename);

	if(rnObj.Valid()) {
		LBF::ReadNode rnGeom = rnObj.GetFirstChild(LBF::GEOM3D);
		if(rnGeom.Valid()) {
			Mesh* mesh = Mesh::CreateMeshFromReadNode( rnGeom );
			entity->SetMesh(mesh);
		}
	}

	if(rnAnim.Valid()) {
		LBF::ReadNode rnSkel = rnAnim.GetFirstChild(LBF::SKELETON);
		LBF::ReadNode rnSkelWeights = rnSkel.GetFirstChild(LBF::SKELETON_JOINT_WEIGHTS);		
		LBF::ReadNode rnFirstClip = rnAnim.GetFirstChild(LBF::ANIMATION);
		Skeleton* skel = Skeleton::CreateSkeletonFromReadNode(rnSkel);
		SkeletonWeights* weights = SkeletonWeights::CreateFromReadNode(rnSkelWeights);
		ClipDB* clips = createClipsFromReadNode(rnFirstClip);
		entity->SetSkeleton(skel, clips, weights);
	}
	delete file;

	return entity;	
}
