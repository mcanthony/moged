#ifndef INCLUDED_moged_weightedmesh_HH
#define INCLUDED_moged_weightedmesh_HH

#include <string>
#include "Mat4.hh"
#include "NonCopyable.hh"

struct sqlite3;
struct sqlite3_stmt;
typedef long long int sqlite3_int64;

namespace LBF { class ReadNode; class WriteNode; }

// mesh container... doesn't really do anything
class Mesh : non_copyable
{
	sqlite3* m_db;
	sqlite3_int64 m_skel_id;
	sqlite3_int64 m_mesh_id;

	////////////////////////////////////////
	// Cached data that does not change after loading.
	std::string m_name;

	Mat4 m_transform;
	
	unsigned int m_num_quads;
	unsigned int m_num_tris;
	unsigned int m_num_verts;

	unsigned int *m_quad_index_buffer;
	unsigned int *m_tri_index_buffer;

	float *m_positions;
	float *m_normals;
	float* m_texcoords;	
	char *m_skin_mats;
	float *m_skin_weights;
public:
	Mesh(sqlite3* db, sqlite3_int64 skel_id, sqlite3_int64 mesh_id);
	~Mesh();

	bool Valid() const { return m_skel_id != 0 && m_mesh_id != 0; }

	const char* GetName() const { return m_name.c_str(); }

	const Mat4& GetTransform() const { return m_transform; }
	const unsigned int *GetQuadIndexBuffer() const { return m_quad_index_buffer; }
	const unsigned int *GetTriIndexBuffer() const { return m_tri_index_buffer; }

	unsigned int GetNumQuads() const { return m_num_quads; }
	unsigned int GetNumTris() const { return m_num_tris; }
	unsigned int GetNumVerts() const { return m_num_verts; }
	
	const float* GetPositionPtr() const { return m_positions; }
	const float* GetNormalPtr() const { return m_normals; }
	const float* GetTexCoordPtr() const { return m_texcoords; }
	const char* GetSkinMatricesPtr() const { return m_skin_mats; }
	const float* GetSkinWeightsPtr() const { return m_skin_weights; }

	static Mesh* LoadFromDB(sqlite3*db, int skelid);
	
	// copies rn and returns a new mesh - does NOT fixup in place.
	LBF::WriteNode* CreateMeshWriteNode( ) const;
	static sqlite3_int64 ImportFromReadNode( sqlite3 *db, sqlite3_int64 skel_id, const LBF::ReadNode& rn );
private:
	bool LoadFromDB( );
};

sqlite3_int64 importMesh(const char* filename, sqlite3* db, sqlite3_int64 skel);

#endif
