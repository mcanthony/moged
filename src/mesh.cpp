#include <cstdio>
#include <vector>
#include "mesh.hh"
#include "lbfloader.hh"
#include "sql/sqlite3.h"
#include "dbhelpers.hh"

using namespace std;

Mesh::Mesh(sqlite3* db, sqlite3_int64 skel_id, sqlite3_int64 mesh_id)
	: m_db(db)
	, m_skel_id(skel_id)
	, m_mesh_id(mesh_id)

	, m_transform( Mat4::ident_t() )

	, m_num_quads(0)
	, m_num_tris(0)
	, m_num_verts(0)

	, m_quad_index_buffer(0)
	, m_tri_index_buffer(0)
	, m_positions(0)
	, m_normals(0)
	, m_texcoords(0)
	, m_skin_mats(0)
	, m_skin_weights(0)
{
	if(!LoadFromDB()) {
		mesh_id = 0;
	}
}

Mesh::~Mesh()
{
	delete[] m_quad_index_buffer;
	delete[] m_tri_index_buffer;
	delete[] m_positions;
	delete[] m_normals;
	delete[] m_texcoords;
	delete[] m_skin_mats;
	delete[] m_skin_weights;
}

struct mesh_save_info {
	Mat4 transform;
	unsigned int num_verts;
	char reserved[12];
};

LBF::WriteNode* Mesh::CreateMeshWriteNode( ) const
{
	mesh_save_info save_info;
	memset(&save_info,0,sizeof(save_info));
	save_info.transform = m_transform;
	save_info.num_verts = m_num_verts;
	
	LBF::WriteNode* meshNode = new LBF::WriteNode( LBF::GEOM3D, 0, sizeof(save_info) );
	meshNode->PutData(&save_info, sizeof(save_info));

	int nameLen = m_name.length();
	LBF::WriteNode* meshName = new LBF::WriteNode( LBF::GEOM3D_NAME, 0, sizeof(char)*nameLen);
	meshNode->AddChild(meshName);
	meshName->PutData(m_name.c_str(), sizeof(char)*nameLen);

	if(m_tri_index_buffer) {
		LBF::WriteNode* idxBuf = new LBF::WriteNode( LBF::TRIMESH_INDICES, 0, 3*m_num_tris*sizeof(unsigned int) );
		meshNode->AddChild(idxBuf);
		idxBuf->PutData(m_tri_index_buffer, idxBuf->GetDataLength());
	}

	if(m_quad_index_buffer) {
		LBF::WriteNode* idxBuf = new LBF::WriteNode( LBF::QUADMESH_INDICES, 0, 4*m_num_quads*sizeof(unsigned int) );
		meshNode->AddChild(idxBuf);
		idxBuf->PutData(m_quad_index_buffer, idxBuf->GetDataLength());
	}

	if(m_positions) {
		LBF::WriteNode* data = new LBF::WriteNode( LBF::POSITIONS, 0, 3*m_num_verts*sizeof(float));
		meshNode->AddChild(data);
		data->PutData(m_positions, data->GetDataLength());
	}

	if(m_normals) {
		LBF::WriteNode* data = new LBF::WriteNode( LBF::NORMALS, 0, 3*m_num_verts*sizeof(float));
		meshNode->AddChild(data);
		data->PutData(m_normals, data->GetDataLength());
	}

	if(m_texcoords) {
		LBF::WriteNode* data = new LBF::WriteNode( LBF::TEXCOORDS, 0, 2*m_num_verts*sizeof(float));
		meshNode->AddChild(data);
		data->PutData(m_texcoords, data->GetDataLength());
	}

	if(m_skin_mats) {
		LBF::WriteNode* data = new LBF::WriteNode( LBF::SKINMATS, 0, 4*m_num_verts*sizeof(char));
		meshNode->AddChild(data);
		data->PutData(m_skin_mats, data->GetDataLength());
	}

	if(m_skin_weights) {
		LBF::WriteNode* data = new LBF::WriteNode( LBF::WEIGHTS, 0, 4*m_num_verts*sizeof(float));
		meshNode->AddChild(data);
		data->PutData(m_skin_weights, data->GetDataLength());
	}
	return meshNode;
}

bool Mesh::LoadFromDB()
{
	if(m_skel_id == 0)
		return false;
	
	// basic mesh info
	Query get_mesh(m_db, "SELECT name,transform FROM meshes WHERE skel_id = ? AND id = ?");
	get_mesh.BindInt64(1, m_skel_id).BindInt64(2, m_mesh_id);
	if(get_mesh.Step()) {
		m_name = get_mesh.ColText(0);
		Mat4* transform = (Mat4*)get_mesh.ColBlob(1);
		if(transform) {
			m_transform = *transform;
		}
	} else return false;

	Query count_verts(m_db, "SELECT count(*) FROM mesh_verts WHERE mesh_id = ? ");
	count_verts.BindInt64(1, m_mesh_id);
	m_num_verts = 0;
	if( count_verts.Step() ) {
		m_num_verts = count_verts.ColInt(0);
	} else return false;
	
	m_positions = new float[3*m_num_verts];
	m_normals = new float[3*m_num_verts];
	m_texcoords = new float[2*m_num_verts];
	m_skin_mats = new char[4*m_num_verts];
	m_skin_weights = new float[4*m_num_verts];

	memset(m_positions, 0, sizeof(float)*3*m_num_verts);
	memset(m_normals, 0, sizeof(float)*3*m_num_verts);
	memset(m_texcoords, 0, sizeof(float)*2*m_num_verts);
	memset(m_skin_mats, 0, sizeof(char)*4*m_num_verts);
	memset(m_skin_weights, 0, sizeof(float)*4*m_num_verts);

	// join and read all vert data
	Query get_verts(m_db, 
					"SELECT mesh_verts.offset,"
					"mesh_verts.x,mesh_verts.y,mesh_verts.z,"
					"mesh_normals.nx, mesh_normals.ny, mesh_normals.nz,"
					"mesh_texcoords.u, mesh_texcoords.v FROM mesh_verts "
					"LEFT JOIN mesh_normals ON mesh_verts.id = mesh_normals.vert_id "
					"LEFT JOIN mesh_texcoords ON mesh_verts.id = mesh_texcoords.vert_id "
					"WHERE mesh_verts.mesh_id = ? ORDER BY mesh_verts.offset ASC");
	get_verts.BindInt64(1, m_skel_id);
	while( get_verts.Step() ) {
		unsigned int offset = get_verts.ColInt(0);
		ASSERT(offset < m_num_verts);

		float x = get_verts.ColDouble(1);
		float y = get_verts.ColDouble(2);
		float z = get_verts.ColDouble(3);

		float nx = get_verts.ColDouble(4);
		float ny = get_verts.ColDouble(5);
		float nz = get_verts.ColDouble(6);

		float u = get_verts.ColDouble(7);
		float v = get_verts.ColDouble(8);

		m_positions[offset*3 + 0] = x;
		m_positions[offset*3 + 1] = y;
		m_positions[offset*3 + 2] = z;

		m_normals[offset*3 + 0] = nx;
		m_normals[offset*3 + 1] = ny;
		m_normals[offset*3 + 2] = nz;

		m_texcoords[offset*2 + 0] = u;
		m_texcoords[offset*2 + 1] = v;
	}	
	
	// need to read in the skins separately as they are specified in multiple rows.
	// use only 4 rows per mesh_offset specified. This depends on them being ordered by offset to work.
	Query get_skin(m_db,
				   "SELECT mesh_verts.offset,mesh_skin.weight,skeleton_joints.offset "
				   "FROM mesh_verts "
				   "LEFT JOIN mesh_skin ON mesh_verts.id = mesh_skin.vert_id "
				   "LEFT JOIN skeleton_joints ON mesh_skin.joint_id = skeleton_joints.id "
				   "WHERE mesh_verts.mesh_id = ? "
				   "ORDER BY mesh_verts.offset ASC");
	get_skin.BindInt64(1, m_skel_id);
	unsigned int last_offset = 0;
	int cur_space = 0;
	while( get_skin.Step() ) {
		unsigned int offset = get_skin.ColInt(0);
		ASSERT(offset < m_num_verts);

		float weight = get_skin.ColDouble(1);
		int joint_offset = get_skin.ColDouble(2);
		
		if(last_offset != offset) {
			cur_space = 0;
			last_offset = offset;
		}
		
		if(cur_space < 4) { // ignore counts of values > 4
			m_skin_mats[offset*4 + cur_space] = char(joint_offset);
			m_skin_weights[offset*4 + cur_space] = weight;
		}
		++cur_space;
	}

	// finally, populate index buffers
	// count quads
	Query count_quads(m_db, "SELECT count(*) FROM mesh_quads WHERE mesh_id = ? ");
	count_quads.BindInt64(1, m_mesh_id);
	m_num_quads = 0;
	if( count_quads.Step() ) {
		m_num_quads = count_quads.ColInt(0);
	} return false;

	// count tris
	Query count_tris(m_db, "SELECT count(*) FROM mesh_tris WHERE mesh_id = ? ");
	count_tris.BindInt64(1, m_mesh_id);
	m_num_tris = 0;
	if( count_tris.Step() ) { 
		m_num_tris = count_tris.ColInt(0);
	} return false;

	m_quad_index_buffer = new unsigned int [m_num_quads];
	m_tri_index_buffer = new unsigned int [m_num_tris];
	
	memset(m_quad_index_buffer, 0, sizeof(unsigned int)*m_num_quads);
	memset(m_tri_index_buffer, 0, sizeof(unsigned int)*m_num_tris);

	// read in quads
	Query get_quads(m_db, "SELECT t0.offset, t1.offset, t2.offset, t3.offset FROM mesh_quads "
					"LEFT JOIN mesh_verts as t0 ON t0.id=mesh_quads.idx0 "
					"LEFT JOIN mesh_verts as t1 ON t0.id=mesh_quads.idx1 "
					"LEFT JOIN mesh_verts as t2 ON t0.id=mesh_quads.idx2 "
					"LEFT JOIN mesh_verts as t3 ON t0.id=mesh_quads.idx3 "
					"WHERE mesh_id = ? ORDER BY id ASC ");
	get_quads.BindInt64(1, m_mesh_id);
	unsigned int cur = 0;
	while( get_quads.Step() ) 
	{
		unsigned int idx0 = get_quads.ColInt(0);
		unsigned int idx1 = get_quads.ColInt(1);
		unsigned int idx2 = get_quads.ColInt(2);
		unsigned int idx3 = get_quads.ColInt(3);
		
		ASSERT(idx0 < m_num_verts && 
			   idx1 < m_num_verts && 
			   idx2 < m_num_verts && 
			   idx3 < m_num_verts);
		
		ASSERT((cur+4) <= m_num_quads*4);
		m_quad_index_buffer[cur + 0] = idx0;
		m_quad_index_buffer[cur + 1] = idx1;
		m_quad_index_buffer[cur + 2] = idx2;
		m_quad_index_buffer[cur + 3] = idx3;
		cur += 4;
	}

	// read in tris
	Query get_tris(m_db, "SELECT t0.offset,t1.offset,t2.offset FROM mesh_tris "
				   "LEFT JOIN mesh_verts as t0 ON t0.id=mesh_quads.idx0 "
				   "LEFT JOIN mesh_verts as t1 ON t0.id=mesh_quads.idx1 "
				   "LEFT JOIN mesh_verts as t2 ON t0.id=mesh_quads.idx2 "
				   "WHERE mesh_id = ? ORDER BY id ASC ");
	get_tris.BindInt64(1, m_mesh_id);
	cur = 0;
	while( get_tris.Step()) 
	{
		unsigned int idx0 = get_tris.ColInt(0);
		unsigned int idx1 = get_tris.ColInt(1);
		unsigned int idx2 = get_tris.ColInt(2);

		ASSERT(idx0 < m_num_verts && 
			   idx1 < m_num_verts && 
			   idx2 < m_num_verts);
		
		ASSERT((cur+3) <= m_num_quads*3);

		m_tri_index_buffer[cur + 0] = idx0;
		m_tri_index_buffer[cur + 1] = idx1;
		m_tri_index_buffer[cur + 2] = idx2;
		cur += 3;
	}
	
	return true;
}

sqlite3_int64 Mesh::ImportFromReadNode( sqlite3* db, sqlite3_int64 skel_id, 
										const LBF::ReadNode& rn )
{
	if(!rn.Valid())
		return 0;

	mesh_save_info save_info;
	memset(&save_info,0,sizeof(save_info));
	rn.GetData(&save_info, sizeof(save_info));	

	std::string meshName;
	LBF::ReadNode rnName = rn.GetFirstChild(LBF::GEOM3D_NAME);
	if(rnName.Valid()) 
		meshName = std::string(rnName.GetNodeData(), rnName.GetNodeDataLength());
	else 
		meshName = "noname";

	sql_begin_transaction(db);

	Query insert_mesh(db, "INSERT INTO meshes (skel_id, transform, name) VALUES(?,?,?)");
	insert_mesh.BindInt64(1, skel_id).BindBlob(2, &save_info.transform, sizeof(save_info.transform))
		.BindText(3, meshName.c_str());
	sqlite3_int64 new_mesh_id = 0;
	insert_mesh.Step();
	if(!insert_mesh.IsError())
		new_mesh_id = insert_mesh.LastRowID();
	else {
		sql_rollback_transaction(db);
		return 0;
	}

	std::vector< sqlite3_int64 > vert_ids;
	vert_ids.resize(save_info.num_verts, 0);

	{ 
		LBF::ReadNode rnChunk = rn.GetFirstChild(LBF::POSITIONS);
		if(rnChunk.Valid()) {
			Query insert(db, "INSERT INTO mesh_verts (mesh_id,offset,x,y,z) VALUES (?,?,?,?,?)");
			insert.BindInt64(1, new_mesh_id);
			BufferReader reader = rnChunk.GetReader();
			for(unsigned int offset = 0; offset < save_info.num_verts; ++offset)
			{
				float x = 0, y = 0, z = 0;
				reader.Get(&x, sizeof(float));
				reader.Get(&y, sizeof(float));
				reader.Get(&z, sizeof(float));

				insert.Reset();
				insert.BindInt(2, offset).BindVec3(3, Vec3(x,y,z));
				insert.Step();
				vert_ids[offset] = insert.LastRowID();
			}
		}
	}

	{ 
		LBF::ReadNode rnChunk = rn.GetFirstChild(LBF::NORMALS);
		if(rnChunk.Valid()) {
			Query insert(db, "INSERT INTO mesh_normals (vert_id,nx,ny,nz) VALUES (?,?,?,?)");
			BufferReader reader = rnChunk.GetReader();
			for(unsigned int offset = 0; offset < save_info.num_verts; ++offset)
			{
				float nx = 0, ny = 0, nz = 0;
				reader.Get(&nx, sizeof(float));
				reader.Get(&ny, sizeof(float));
				reader.Get(&nz, sizeof(float));

				insert.Reset();
				insert.BindInt64(1, vert_ids[offset]).BindVec3(2, Vec3(nx,ny,nz));
				insert.Step();
			}
		}
	}

	int num_joint_ids = 0;
	sqlite3_int64* joint_ids = 0;
	if(getJointIdMap( db, skel_id , &num_joint_ids, &joint_ids ))
	{ 
		LBF::ReadNode rnChunk = rn.GetFirstChild(LBF::WEIGHTS);
		LBF::ReadNode skinChunk = rn.GetFirstChild(LBF::SKINMATS);

		if(rnChunk.Valid() && skinChunk.Valid()) {
			Query insert(db, "INSERT INTO mesh_skin (vert_id,joint_id,weight) VALUES (?,?,?)");
			BufferReader weightReader = rnChunk.GetReader();
			BufferReader skinReader = rnChunk.GetReader();
			float weight = 0.f;
			char skin = 0;

			for(unsigned int offset = 0; offset < save_info.num_verts; ++offset)
			{
				insert.BindInt64(1, vert_ids[offset]);
				for(int i = 0; i < 4; ++i)
				{
					skinReader.Get(&skin, sizeof(char));
					weightReader.Get(&weight, sizeof(float));

					if(skin < 0 || skin >= num_joint_ids) {
						weight = 0.f; skin = 0;
					}

					sqlite3_int64 joint_id = joint_ids[(int)skin];
					insert.Reset();
					insert.BindInt64(2, joint_id).BindDouble(3, weight);
					insert.Step();
				}
			}
		}
		delete[] joint_ids;
	}

	{ 
		LBF::ReadNode rnChunk = rn.GetFirstChild(LBF::TEXCOORDS);
		if(rnChunk.Valid()) {
			Query insert(db, "INSERT INTO mesh_texcoords (vert_id,u,v) VALUES (?,?,?)");
			BufferReader reader = rnChunk.GetReader();
			for(unsigned int offset = 0; offset < save_info.num_verts; ++offset)
			{
				float u = 0, v = 0;
				reader.Get(&u, sizeof(float));
				reader.Get(&v, sizeof(float));

				insert.Reset();
				insert.BindInt64(1, vert_ids[offset]).BindDouble(2,u).BindDouble(3,v);
				insert.Step();
			}
		}
	}

	{
		LBF::ReadNode rnTri = rn.GetFirstChild(LBF::TRIMESH_INDICES);
		if(rnTri.Valid()) {
			int numIndices = rnTri.GetNodeDataLength() / sizeof(unsigned int); 
			if( (numIndices % 3) != 0 ) {
				fprintf(stderr, "mesh load error: Number of trimesh indices (%d) must be divisible by 3\n", numIndices);
			}
			else {
				BufferReader reader = rnTri.GetReader();
				Query insert(db, "INSERT INTO mesh_tris (mesh_id,idx0,idx1,idx2) VALUES (?,?,?,?)");

				insert.BindInt64(1, new_mesh_id);

				const int numTris = numIndices / 3;
				unsigned int idx = 0;
				for(int i = 0; i < numTris; ++i) {
					reader.Get(&idx, sizeof(unsigned int));
					insert.BindInt64(2, vert_ids[idx]);
					reader.Get(&idx, sizeof(unsigned int));
					insert.BindInt64(3, vert_ids[idx]);
					reader.Get(&idx, sizeof(unsigned int));
					insert.BindInt64(4, vert_ids[idx]);

					insert.Reset(); 
					insert.Step();
				}
			}
		}
	}

	{
		LBF::ReadNode rnQuad = rn.GetFirstChild(LBF::QUADMESH_INDICES);
		if(rnQuad.Valid()) {
			int numIndices = rnQuad.GetNodeDataLength() / sizeof(unsigned int); 
			if( (numIndices % 4) != 0 ) {
				fprintf(stderr, "mesh load error: Number of quadmesh indices (%d) must be divisible by 4\n", numIndices);
			
			} else {
				BufferReader reader = rnQuad.GetReader();

				Query insert(db, "INSERT INTO mesh_quads (mesh_id,idx0,idx1,idx2,idx3) VALUES (?,?,?,?,?)");
				insert.BindInt64(1, new_mesh_id);
				const int numQuads  = numIndices / 4;
				unsigned int idx = 0;
				for(int i = 0; i < numQuads; ++i) {

					reader.Get(&idx, sizeof(unsigned int));
					insert.BindInt64(2, vert_ids[idx]);
					reader.Get(&idx, sizeof(unsigned int));
					insert.BindInt64(3, vert_ids[idx]);
					reader.Get(&idx, sizeof(unsigned int));
					insert.BindInt64(4, vert_ids[idx]);
					reader.Get(&idx, sizeof(unsigned int));
					insert.BindInt64(5, vert_ids[idx]);

					insert.Reset(); 
					insert.Step();
				}
			}
		}
	}

	sql_end_transaction(db);
	
	return new_mesh_id;
}


sqlite3_int64 importMesh(const char* filename, sqlite3* db, sqlite3_int64 skel)
{
	if(skel == 0) return 0;

	sqlite3_int64 result = 0;
	LBF::LBFData* file ;
	int err = LBF::openLBF(filename, file );
	if(err != 0) {
		fprintf(stderr, "Failed with err code %d\n", err);
		return 0;
	}

	LBF::ReadNode rnObj = file->GetFirstNode(LBF::OBJECT_SECTION);
	if(rnObj.Valid()) {
		LBF::ReadNode rnGeom = rnObj.GetFirstChild(LBF::GEOM3D);
		if(rnGeom.Valid()) {
			
			result = Mesh::ImportFromReadNode(db, skel, rnGeom );
		}
	}

	delete file;
	return result;
}
