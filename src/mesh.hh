#ifndef INCLUDED_moged_weightedmesh_HH
#define INCLUDED_moged_weightedmesh_HH

#include <string>
#include "Mat4.hh"

namespace LBF { class ReadNode; class WriteNode; }

enum MeshFmt {
	MeshFmt_Position = 0,
	MeshFmt_Normal,
	MeshFmt_Texcoord,
	MeshFmt_Weight, 
	MeshFmt_SkinMat, 
};

// mesh container... doesn't really do anything
class Mesh
{
	std::string m_name;

	int m_format_size;
	int *m_format;
	
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
	Mesh();
	~Mesh();

	const char* GetName() const { return m_name.c_str(); }
	int GetFormatSize() const { return m_format_size; }
	int GetFormatAt(int idx) const { return m_format[idx]; }

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
	
	// copies rn and returns a new mesh - does NOT fixup in place.
	LBF::WriteNode* CreateMeshWriteNode( ) const;
	static Mesh* CreateMeshFromReadNode( const LBF::ReadNode& rn );
};

Mesh* loadMesh(const char* filename);

#endif
