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
		m_mesh_id = 0;
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
	Query get_mesh(m_db, "SELECT name,transform,num_verts FROM meshes WHERE skel_id = ? AND id = ?");
	get_mesh.BindInt64(1, m_skel_id).BindInt64(2, m_mesh_id);
	if(get_mesh.Step()) {
		m_name = get_mesh.ColText(0);
		Mat4* transform = (Mat4*)get_mesh.ColBlob(1);
		if(transform) {
			m_transform = *transform;
		}
		m_num_verts = get_mesh.ColInt(2);
	} else return false;

	m_positions = new float[3*m_num_verts];
	const int positionsSize = sizeof(float)*3*m_num_verts;

	m_normals = new float[3*m_num_verts];
	const int normalsSize = sizeof(float)*3*m_num_verts;

	m_texcoords = new float[2*m_num_verts];
	const int texcoordsSize = sizeof(float)*2*m_num_verts;

	m_skin_mats = new char[4*m_num_verts];
	const int skin_matsSize = sizeof(char)*4*m_num_verts;

	m_skin_weights = new float[4*m_num_verts];
	const int skin_weightsSize = sizeof(float)*4*m_num_verts;

	memset(m_positions, 0, positionsSize);
	memset(m_normals, 0, normalsSize);
	memset(m_texcoords, 0, texcoordsSize);
	memset(m_skin_mats, 0, skin_matsSize);
	memset(m_skin_weights, 0, skin_weightsSize);

	// Read conveniently shaped binary dump of data!
	Blob readVerts(m_db, "meshes", "vertices", m_mesh_id, false);
	readVerts.Read(m_positions, positionsSize, 0);

	Blob readNormals(m_db, "meshes", "normals", m_mesh_id, false);
	readNormals.Read(m_normals, normalsSize, 0);

	Blob readTexcoords(m_db, "meshes", "texcoords", m_mesh_id, false);
	readTexcoords.Read(m_texcoords, texcoordsSize, 0);

	Blob readSkinWeights(m_db, "meshes", "weights", m_mesh_id, false);
	readSkinWeights.Read(m_skin_weights, skin_weightsSize, 0);

	Blob readSkinMats(m_db, "meshes", "skinmats", m_mesh_id, false);
	readSkinMats.Read(m_skin_mats, skin_matsSize, 0);

	Blob readQuads(m_db, "meshes", "quads", m_mesh_id, false);
	Blob readTris(m_db, "meshes", "triangles", m_mesh_id, false);

	m_num_quads = readQuads.GetSize() / (sizeof(unsigned int) * 4);
	m_num_tris = readTris.GetSize() / (sizeof(unsigned int) * 3);

	m_quad_index_buffer = new unsigned int [m_num_quads*4];
	const int quadsSize = m_num_quads * 4 * sizeof(unsigned int);
	m_tri_index_buffer = new unsigned int [m_num_tris*3];
	const int trisSize = m_num_tris * 3 * sizeof(unsigned int);
	
	memset(m_quad_index_buffer, 0, quadsSize);
	memset(m_tri_index_buffer, 0, trisSize);

	readQuads.Read(m_quad_index_buffer, quadsSize, 0);
	readTris.Read(m_tri_index_buffer, trisSize, 0);
	
	return true;
}

sqlite3_int64 Mesh::ImportFromReadNode( sqlite3* db, sqlite3_int64 skel_id, 
					const LBF::ReadNode& rn )
{
	if(!rn.Valid())
		return 0;

	mesh_save_info save_info;
	memset(&save_info, 0, sizeof(save_info));
	rn.GetData(&save_info, sizeof(save_info));	

	std::string meshName;
	LBF::ReadNode rnName = rn.GetFirstChild(LBF::GEOM3D_NAME);
	if(rnName.Valid()) 
		meshName = std::string(rnName.GetNodeData(), rnName.GetNodeDataLength());
	else 
		meshName = "noname";

	sql_begin_transaction(db);

	Query insert_mesh(db, "INSERT INTO meshes (skel_id, name, transform, num_verts) VALUES(?,?,?,?)");
	insert_mesh.BindInt64(1, skel_id)
		.BindText(2, meshName.c_str())
		.BindBlob(3, &save_info.transform, sizeof(save_info.transform))
		.BindInt64(4, save_info.num_verts);
	sqlite3_int64 new_mesh_id = 0;
	insert_mesh.Step();
	if(!insert_mesh.IsError())
		new_mesh_id = insert_mesh.LastRowID();
	else {
		sql_rollback_transaction(db);
		return 0;
	}

	{ 
		LBF::ReadNode rnChunk = rn.GetFirstChild(LBF::POSITIONS);
		if(rnChunk.Valid()) {
			const int blobSize = save_info.num_verts * sizeof(Vec3);
			if( blobSize != rnChunk.GetNodeDataLength ())
			{
				fprintf(stderr, "mesh load error: wrong size for vert block. expected %d, have %d\n", blobSize, rnChunk.GetNodeDataLength());
			}
			else
			{
				Query query_resize_verts(db, "UPDATE meshes SET vertices = ? WHERE id = ?");
				query_resize_verts.BindBlob(1, rnChunk.GetNodeData(), blobSize );
				query_resize_verts.BindInt64(2, new_mesh_id);
				query_resize_verts.Step();
			}
		}
	}

	{ 
		LBF::ReadNode rnChunk = rn.GetFirstChild(LBF::NORMALS);
		if(rnChunk.Valid()) {
			const int blobSize = save_info.num_verts * sizeof(Vec3);
			if( blobSize != rnChunk.GetNodeDataLength ())
			{
				fprintf(stderr, "mesh load error: wrong size for normals block. expected %d, have %d\n", blobSize, rnChunk.GetNodeDataLength());
			}
			else
			{
				Query query_resize_normals(db, "UPDATE meshes SET normals = ? WHERE id = ?");
				query_resize_normals.BindBlob(1, rnChunk.GetNodeData(), blobSize);
				query_resize_normals.BindInt64(2, new_mesh_id);
				query_resize_normals.Step();
			}
		}
	}

	{ 
		LBF::ReadNode wtChunk = rn.GetFirstChild(LBF::WEIGHTS);
		LBF::ReadNode skinChunk = rn.GetFirstChild(LBF::SKINMATS);

		if(wtChunk.Valid() && skinChunk.Valid()) {
			const int wtBlobSize = save_info.num_verts * sizeof(float) * 4;
			const int matBlobSize = save_info.num_verts * sizeof(char) * 4;
			if( wtBlobSize != wtChunk.GetNodeDataLength() ||
				matBlobSize != skinChunk.GetNodeDataLength() )
			{
				fprintf(stderr, "mesh load error: wrong size for skinning info. Expected weight & mats to be of size %d and %d, and got %d and %d.\n", wtBlobSize, matBlobSize, wtChunk.GetNodeDataLength(), skinChunk.GetNodeDataLength());
			}
			else
			{
				Query query_resize_weights(db, "UPDATE meshes SET weights = ? WHERE id = ?");
				query_resize_weights.BindBlob(1, wtChunk.GetNodeData(), wtBlobSize);
				query_resize_weights.BindInt64(2, new_mesh_id);
				query_resize_weights.Step();

				Query query_resize_mats(db, "UPDATE meshes SET skinmats = ? WHERE id = ?");
				query_resize_mats.BindBlob(1, skinChunk.GetNodeData(), matBlobSize);
				query_resize_mats.BindInt64(2, new_mesh_id);
				query_resize_mats.Step();
			}
		}
	}

	{ 
		LBF::ReadNode rnChunk = rn.GetFirstChild(LBF::TEXCOORDS);
		if(rnChunk.Valid()) {
			const int blobSize = save_info.num_verts * sizeof(float) * 2;
			if( blobSize != rnChunk.GetNodeDataLength())
			{
				fprintf(stderr, "mesh load error: wrong size for texcoords. Expected %d, got %d\n", blobSize, rnChunk.GetNodeDataLength());
			}
			else
			{
				Query query_resize_texcoords(db, "UPDATE meshes SET texcoords = ? WHERE id = ?");
				query_resize_texcoords.BindBlob(1, rnChunk.GetNodeData(), blobSize);
				query_resize_texcoords.BindInt64(2, new_mesh_id);
				query_resize_texcoords.Step();
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
				Query query_resize_tris(db, "UPDATE meshes SET triangles = ? WHERE id = ?");
				query_resize_tris.BindBlob(1, rnTri.GetNodeData(), rnTri.GetNodeDataLength());
				query_resize_tris.BindInt64(2, new_mesh_id);
				query_resize_tris.Step();
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
				Query query_resize_quads(db, "UPDATE meshes SET quads = ? WHERE id = ?");
				query_resize_quads.BindBlob(1, rnQuad.GetNodeData(), rnQuad.GetNodeDataLength());
				query_resize_quads.BindInt64(2, new_mesh_id);
				query_resize_quads.Step();
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
