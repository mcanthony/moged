#include <cstdio>
#include "entity.hh"
#include "skeleton.hh"
#include "clipdb.hh"
#include "mesh.hh"
#include "lbfloader.hh"
#include "motiongraph.hh"
#include "mogedevents.hh"
#include "sql/sqlite3.h"

// Current file version. Increment when format is different enough that previous
// data cannot be loaded without a conversion step.
static const int kCurrentVersion = 1;

Entity::Entity(	Events::EventSystem* evsys)
	: m_evsys(evsys)
	, m_db(0)
	, m_skeleton(0)
	, m_clips(0)
	, m_mesh(0)
	, m_mg(0)
{
}

Entity::~Entity()
{
	delete m_skeleton;
	delete m_clips;
	delete m_mesh;
	delete m_mg;

	sqlite3_close(m_db);	
}

void Entity::SetFilename( const char* filename )
{
	Reset();
	sqlite3_close(m_db); m_db = 0;

	if( sqlite3_open(filename, &m_db) == SQLITE_OK)
	{
		sqlite3_int64 version = 0;
		if(CheckVersion(&version))
		{
			// TODO: check to see if I can load and convert an older version.
			Query enable_fk(m_db, "PRAGMA foreign_keys = ON");
			enable_fk.Step();
		
			sqlite3_busy_timeout(m_db, 1000);

			CreateMissingTables();
			sqlite3_int64 skelid;
			if(FindFirstSkeleton(&skelid)) {
				SetCurrentSkeleton(skelid);
			}
		
			EnsureVersion();
		}
		else
		{
			sqlite3_close(m_db); m_db = 0;

			Events::WrongFileVersion ev;
			ev.FileVersion = version;
			ev.RequiredVersion = kCurrentVersion;
			m_evsys->Send(&ev);
		}
	}
}

void Entity::Reset()
{
	delete m_skeleton;m_skeleton=0;
	delete m_clips;m_clips=0;
	delete m_mesh;m_mesh=0;
	delete m_mg;m_mg=0;

	Events::EntitySkeletonChangedEvent ev;
	m_evsys->Send(&ev);
}


sqlite3_int64 Entity::GetCurrentSkeleton() const 
{
	if(m_skeleton) return m_skeleton->GetID();
	else return 0;
}

void Entity::SetCurrentSkeleton(sqlite3_int64 id)
{
	Reset();
	
	Skeleton* newSkel = new Skeleton(m_db, id);
	if(newSkel->Valid())
	{
		m_skeleton = newSkel;
		m_clips = new ClipDB(m_db, id);

		sqlite3_int64 mg_id;
		if(FindFirstMotionGraph(id, &mg_id))
			SetCurrentMotionGraph(mg_id);

		sqlite3_int64 meshid;
		if(FindFirstMesh(id, &meshid)) {
			m_mesh = new Mesh(m_db, id, meshid);
		}
	} else {
		delete newSkel; 
	}
		
	Events::EntitySkeletonChangedEvent ev;
	m_evsys->Send(&ev);		
}

void Entity::SetCurrentMesh(sqlite3_int64 id)
{
	delete m_mesh; m_mesh = 0;
	if(m_skeleton) {
		m_mesh = new Mesh(m_db, m_skeleton->GetID(), id);
		if(!m_mesh->Valid()) {
			delete m_mesh; m_mesh = 0;
		}			
	}
	Events::EntitySkeletonChangedEvent ev;
	m_evsys->Send(&ev);		
}

void Entity::SetCurrentMotionGraph(sqlite3_int64 id)
{
	delete m_mg; m_mg = 0;
	if(m_skeleton) {
		m_mg = new MotionGraph(m_db, m_skeleton->GetID(), id);
		if(!m_mg->Valid()) {
			delete m_mg; m_mg = 0;
		}
	}

	Events::EntityMotionGraphChangedEvent ev;
	ev.MotionGraphID = m_mg ? m_mg->GetID() : 0;
	m_evsys->Send(&ev);
}

bool Entity::CheckVersion(sqlite3_int64* out_version)
{
	Query existsQuery(m_db, "SELECT count(type) FROM sqlite_master WHERE type='table' AND name='dbschema'");

	if(!existsQuery.Step() ||
		existsQuery.ColInt64(0) < 1)
	{
		// if no tables exist, then this is a new file and it's ok to return true so 
		// the version will be ensured.
		Query anyExistsQuery(m_db, "SELECT count(type) FROM sqlite_master WHERE type='table'");
		if(anyExistsQuery.Step() && anyExistsQuery.ColInt64(0) == 0)
			return true;
		return false;		
	}

	Query versionQuery(m_db, "SELECT version FROM dbschema ORDER BY version ASC LIMIT 1");
	if(versionQuery.Step())
	{
		sqlite3_int64 version = versionQuery.ColInt64(0);
		if(out_version) *out_version = version;
		return (int)version == kCurrentVersion;
	}
	if(out_version) *out_version = 0;
	return false;
}

void Entity::EnsureVersion()
{
	Query currentVersion(m_db, "SELECT version FROM dbschema ORDER BY version ASC");
	if(!currentVersion.Step()) {
		Query setVersion(m_db, "INSERT INTO dbschema(version) VALUES (?)");
		setVersion.BindInt64(1, kCurrentVersion);
		setVersion.Step();
	} else {
		Query updateVersion(m_db, "UPDATE dbschema SET version = ?");
		updateVersion.BindInt64(1, kCurrentVersion);
		updateVersion.Step();
	}
}

bool Entity::FindFirstSkeleton(sqlite3_int64 *skel_id)
{
	if(skel_id == 0) return false;
	*skel_id = 0;

	Query find(m_db, "SELECT id FROM skeleton ORDER BY id ASC LIMIT 1");
	if( find.Step() )
		*skel_id = find.ColInt64(0);
	return *skel_id != 0;
}

bool Entity::FindFirstMesh (sqlite3_int64 skel_id, sqlite3_int64* mesh_id)
{
	if(mesh_id == 0) return false;
	*mesh_id = 0;

	Query find(m_db, "SELECT id FROM meshes WHERE skel_id=? ORDER BY id ASC LIMIT 1");
	find.BindInt64(1, skel_id);
	if( find.Step()) 
		*mesh_id = find.ColInt64(0);
	return *mesh_id != 0;
}

bool Entity::FindFirstMotionGraph(sqlite3_int64 skel_id, sqlite3_int64* mg_id)
{
	if(mg_id == 0) return false;
	*mg_id = 0;
   
	Query find(m_db, "SELECT id FROM motion_graphs WHERE skel_id = ? ORDER BY id ASC LIMIT 1");
	find.BindInt64(1, skel_id);
	if(find.Step())
		*mg_id = find.ColInt64(0);
	return *mg_id != 0;
}

void Entity::GetSkeletonInfos(std::vector<SkeletonInfo>& out) const
{
	out.clear();
	Query query(m_db, "SELECT id,name FROM skeleton");
	while(query.Step()) {
		SkeletonInfo info;
		info.id = query.ColInt64(0);
		info.name = query.ColText(1);
		out.push_back(info);
	}
}

void Entity::GetMotionGraphInfos(std::vector<MotionGraphInfo>& out, sqlite3_int64 skel_id) const
{
	out.clear();
	Query query(m_db, "SELECT id,name FROM motion_graphs WHERE skel_id = ?");
	query.BindInt64(1, skel_id);
	while(query.Step()) {
		MotionGraphInfo info;
		info.id = query.ColInt64(0);
		info.name = query.ColText(1);
		out.push_back(info);
	}
}

void Entity::GetMeshInfos(std::vector<MeshInfo>& out, sqlite3_int64 skel_id) const
{
	out.clear();
	Query query(m_db, "SELECT id,name FROM meshes WHERE skel_id = ?");
	query.BindInt64(1, skel_id);
	while(query.Step()) {
		MeshInfo info;
		info.id = query.ColInt64(0);
		info.name = query.ColText(1);
		out.push_back(info);
	}
}

void Entity::DeleteSkeleton(sqlite3_int64 skel_id)
{
	sql_begin_transaction(m_db);
	// delete everything using a skeleton (mesh, motion_graphs)
	Query del_meshes(m_db, "DELETE FROM meshes WHERE skel_id = ?");
	del_meshes.BindInt64(1, skel_id);
	del_meshes.Step();

	Query del_mg(m_db, "DELETE FROM motion_graphs WHERE skel_id = ?");
	del_mg.BindInt64(1, skel_id);
	del_mg.Step();
	
	Query del_clips(m_db, "DELETE FROM clips WHERE skel_id = ?");
	del_clips.BindInt64(1, skel_id);
	del_clips.Step();

	Query del_skel(m_db, "DELETE FROM skeleton WHERE id = ?");
	del_skel.BindInt64(1, skel_id);
	del_skel.Step();

	sql_end_transaction(m_db);

	Query query_vacuum(m_db, "VACUUM");
	query_vacuum.Step();
}

void Entity::DeleteMesh(sqlite3_int64 mesh_id)
{
	sql_begin_transaction(m_db);
	Query del_meshes(m_db, "DELETE FROM meshes WHERE id = ?");
	del_meshes.BindInt64(1, mesh_id);
	del_meshes.Step();
	sql_end_transaction(m_db);

	Query query_vacuum(m_db, "VACUUM");
	query_vacuum.Step();
}

void Entity::DeleteMotionGraph(sqlite3_int64 mg_id)
{
	Transaction t(m_db);
	std::vector<sqlite3_int64> transitions ;

	// need to delete these after the graph is deleted due to fk constraints
	Query get_transitions(m_db, 
						  "SELECT id FROM clips WHERE is_transition = 1 AND "
						  "(id IN (SELECT clip_id FROM motion_graph_edges WHERE motion_graph_id = ?) OR "
						  "id IN (SELECT clip_id FROM motion_graph_nodes WHERE motion_graph_id = ?))");
	get_transitions.BindInt64(1, mg_id).BindInt64(2, mg_id);
	while(get_transitions.Step()) {
		transitions.push_back(get_transitions.ColInt64(0));
	}

	Query del(m_db, "DELETE FROM motion_graphs WHERE id = ?");
	del.BindInt64(1, mg_id);
	del.Step();
	
	Query del_transition(m_db, "DELETE FROM clips WHERE id = ? AND is_transition = 1");
	for(int i = 0; i < (int)transitions.size(); ++i) {
		del_transition.Reset();
		del_transition.BindInt64(1, transitions[i] );
		del_transition.Step();
	}

	Query query_vacuum(m_db, "VACUUM");
	query_vacuum.Step();

	Events::MassClipRemoveEvent ev;
	m_evsys->Send(&ev);
}

void Entity::CreateMissingTables()
{
	ASSERT(m_db);

	static const char* beginTransaction = 
		"BEGIN EXCLUSIVE TRANSACTION";
	static const char* endTransaction = 
		"END TRANSACTION";

	static const char* createSchemaStmt = 
		"CREATE TABLE IF NOT EXISTS dbschema ("
		"version INTEGER"
		")";

	static const char* createSkelStmt = 
		"CREATE TABLE IF NOT EXISTS skeleton ("
		"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
		"name TEXT,"
		"root_offset BLOB,"
		"root_rotation BLOB,"
		"num_joints INTEGER NOT NULL,"
		"translations BLOB,"
		"parents BLOB,"
		"weights BLOB"
		")";

	static const char* indexSkel = 
		"CREATE UNIQUE INDEX IF NOT EXISTS idx_skeleton ON skeleton (id)";
	  
	static const char* createJointsStmt = 
		"CREATE TABLE IF NOT EXISTS skeleton_joints ("
		"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
		"skel_id INTEGER NOT NULL,"
		"offset INTEGER NOT NULL," // index in skeleton
		"name TEXT,"
		"UNIQUE(skel_id,offset),"
		"CONSTRAINT joints_parent_u FOREIGN KEY(skel_id) REFERENCES skeleton(id) ON DELETE CASCADE)";

	static const char *indexJoint1 =
		"CREATE UNIQUE INDEX IF NOT EXISTS idx_skeleton_joints ON skeleton_joints (id)";

	static const char *indexJoint2 =
		"CREATE UNIQUE INDEX IF NOT EXISTS idx_skeleton_joints2 ON skeleton_joints (skel_id,offset)";	

	static const char* createClipsStmt =
		"CREATE TABLE IF NOT EXISTS clips ("
		"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
		"skel_id INTEGER NOT NULL,"
		"name TEXT,"
		"fps REAL,"
		"is_transition INTEGER DEFAULT 0,"
		"num_frames INTEGER NOT NULL,"
		"frames BLOB,"
		"CONSTRAINT clips_parent_d FOREIGN KEY(skel_id) REFERENCES skeleton(id) ON UPDATE RESTRICT ON DELETE RESTRICT)";

	static const char *indexClips =
		"CREATE UNIQUE INDEX IF NOT EXISTS idx_clips ON clips (id)";	

	static const char* createAnnotationsStmt = 
		"CREATE TABLE IF NOT EXISTS annotations ("
		"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
		"name TEXT,"
		"fidelity REAL,"
		"UNIQUE(name))";

	static const char *indexAnno =
		"CREATE UNIQUE INDEX IF NOT EXISTS idx_annotations ON annotations (id)";	

	static const char* createClipAnnotationsStmt = 
		"CREATE TABLE IF NOT EXISTS clip_annotations ("
		"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
		"annotation_id INTEGER NOT NULL,"
		"clip_id INTEGER NOT NULL,"
		"UNIQUE(annotation_id,clip_id),"
		"CONSTRAINT anno_parent_d FOREIGN KEY (annotation_id) REFERENCES annotations(id) ON DELETE CASCADE,"
		"CONSTRAINT anno_clip_parent_d FOREIGN KEY (clip_id) REFERENCES clips(id) ON DELETE CASCADE)";

	static const char *indexClipAnno1 =
		"CREATE UNIQUE INDEX IF NOT EXISTS idx_clip_annotations ON clip_annotations (id)";	

	static const char *indexClipAnno2 =
		"CREATE UNIQUE INDEX IF NOT EXISTS idx_clip_annotations2 ON clip_annotations (annotation_id,clip_id)";	

	static const char* createMeshStmt =
		"CREATE TABLE IF NOT EXISTS meshes ("
		"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
		"skel_id INTEGER NOT NULL,"
		"name TEXT,"
		"transform BLOB," // mat4
		"num_verts INTEGER NOT NULL,"
		"vertices BLOB," // vec3s
		"normals BLOB," // vec3s
		"texcoords BLOB," // float2s
		"triangles BLOB," // integer triplets
		"quads BLOB," // integer 4-tuples
		"skinmats BLOB," // char4s (same length as vertices, indexes into skeleton)
		"weights BLOB," // float4s (same length as vertices)
		"CONSTRAINT mesh_owner_d FOREIGN KEY (skel_id) REFERENCES skeleton(id) ON UPDATE CASCADE ON DELETE RESTRICT)";

	static const char *indexMeshes =
		"CREATE UNIQUE INDEX IF NOT EXISTS idx_meshes ON meshes (id)";	

	static const char* createMotionGraphContainerStmt = 
		"CREATE TABLE IF NOT EXISTS motion_graphs ("
		"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
		"skel_id INTEGER NOT NULL,"
		"name TEXT,"
		"CONSTRAINT mg_owner_2 FOREIGN KEY (skel_id) REFERENCES skeleton(id) ON UPDATE CASCADE ON DELETE RESTRICT)";

	static const char *indexMg =
		"CREATE UNIQUE INDEX IF NOT EXISTS idx_mg ON motion_graphs (id)";		

	static const char* createMotionGraphStmt =
		"CREATE TABLE IF NOT EXISTS motion_graph_edges ("
		"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
		"motion_graph_id INTEGER NOT NULL,"
		"clip_id INTEGER NOT NULL,"
		"start_id INTEGER NOT NULL,"
		"finish_id INTEGER NOT NULL,"
		"align_t_x REAL DEFAULT 0.0,"
		"align_t_y REAL DEFAULT 0.0,"
		"align_t_z REAL DEFAULT 0.0,"
		"align_q_a REAL DEFAULT 0.0,"
		"align_q_b REAL DEFAULT 0.0,"
		"align_q_c REAL DEFAULT 0.0,"
		"align_q_r REAL DEFAULT 1.0,"
		"CONSTRAINT edge_owner_2 FOREIGN KEY (motion_graph_id) REFERENCES motion_graphs(id) ON DELETE CASCADE ON UPDATE CASCADE,"
		"CONSTRAINT edge_owner_3 FOREIGN KEY (clip_id) REFERENCES clips(id) ON UPDATE CASCADE ON DELETE RESTRICT,"
		"CONSTRAINT edge_owner_5 FOREIGN KEY (start_id) REFERENCES motion_graph_nodes(id) ON DELETE CASCADE,"
		"CONSTRAINT edge_owner_6 FOREIGN KEY (finish_id) REFERENCES motion_graph_nodes(id) ON DELETE CASCADE)";

	static const char *indexMgEdges1 =
		"CREATE UNIQUE INDEX IF NOT EXISTS idx_mg_edges ON motion_graph_edges (id)";		

//	static const char *indexMgEdges2 =
//		"CREATE INDEX IF NOT EXISTS idx_mg_edges2 ON motion_graph_edges (start_id)";		

	static const char* createMotionGraphNodesStmt =
		"CREATE TABLE IF NOT EXISTS motion_graph_nodes ("
		"id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
		"motion_graph_id INTEGER NOT NULL,"
		"clip_id INTEGER NOT NULL,"
		"frame_num INTEGER NOT NULL,"
		"CONSTRAINT node_owner_2 FOREIGN KEY (motion_graph_id) REFERENCES motion_graphs(id) ON UPDATE CASCADE ON DELETE CASCADE)" ;

	static const char *indexMgNodes1 =
		"CREATE UNIQUE INDEX IF NOT EXISTS idx_mg_nodes ON motion_graph_nodes (id)";		
	const char* toCreate[] = 
	{
		beginTransaction,
		createSchemaStmt,
		createSkelStmt,
		indexSkel,
		createJointsStmt,
		indexJoint1,
		indexJoint2,
		createClipsStmt,
		indexClips,
		createAnnotationsStmt,
		indexAnno,
		createClipAnnotationsStmt,
		indexClipAnno1,
		indexClipAnno2,
		createMeshStmt, // TODO binary representation
		indexMeshes,
		createMotionGraphContainerStmt,
		indexMg,
		createMotionGraphStmt,
		indexMgEdges1,
		createMotionGraphNodesStmt,
		indexMgNodes1,
		endTransaction,
	};
		
	const int count = sizeof(toCreate)/sizeof(const char*);
	for(int i = 0; i < count; ++i) {
		Query create(m_db, toCreate[i]);
		create.Step();
	}
}

////////////////////////////////////////////////////////////////////////////////
// serialization funcs
////////////////////////////////////////////////////////////////////////////////

bool exportEntityLBF(const Entity* entity, const char* filename)
{
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
			LBF::WriteNode* skelWeightsNode = entity->GetSkeleton()->GetSkeletonWeights().CreateWriteNode();
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

	int err = LBF::saveLBF( filename, objSectionNode, true );
	delete objSectionNode;
	if(err == 0) {
		return true;
	} else {
		fprintf(stderr, "Failed with error code %d\n", err);
		// error report?
		return false;
	}
}

bool importEntityLBF(Entity *target, const char* filename)
{
	if(!target->HasDB()) return false;

	LBF::LBFData* file = 0;
	int err = LBF::openLBF( filename, file );
	if(err != 0) {
		fprintf(stderr, "Failed with error code %d\n", err);
		return false;
	}

	LBF::ReadNode rnObj = file->GetFirstNode(LBF::OBJECT_SECTION);
	LBF::ReadNode rnAnim = file->GetFirstNode(LBF::ANIM_SECTION);

	if(rnAnim.Valid()) {
		LBF::ReadNode rnSkel = rnAnim.GetFirstChild(LBF::SKELETON);
		LBF::ReadNode rnSkelWeights = rnSkel.GetFirstChild(LBF::SKELETON_JOINT_WEIGHTS);		
		LBF::ReadNode rnFirstClip = rnAnim.GetFirstChild(LBF::ANIMATION);
		
		sqlite3_int64 skelid = Skeleton::ImportFromReadNode(target->GetDB(), rnSkel);
		if(skelid > 0)
		{
			SkeletonWeights weights(target->GetDB(), skelid);
			weights.ImportFromReadNode(rnSkelWeights);

			importClipsFromReadNode(target->GetDB(), skelid, rnFirstClip);	

			target->SetCurrentSkeleton(skelid);
		}
	}

	if(rnObj.Valid() ) {
		LBF::ReadNode rnGeom = rnObj.GetFirstChild(LBF::GEOM3D);
		if(rnGeom.Valid()) {
			sqlite3_int64 mesh_id =  Mesh::ImportFromReadNode( target->GetDB(), target->GetCurrentSkeleton(), rnGeom );
			if(mesh_id > 0) 
				target->SetCurrentMesh(mesh_id);
		}
	}
	delete file;
	return true;
}

