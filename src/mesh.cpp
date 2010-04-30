#include <cstdio>
#include "mesh.hh"
#include "lbfloader.hh"

using namespace std;

Mesh::Mesh()
	: m_format_size(0)
	, m_format(0)
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
	
}

Mesh::~Mesh()
{
	delete[] m_format;
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

int MapLBFType(int type) {
	switch(type) {
	case LBF::POSITIONS: return MeshFmt_Position;
	case LBF::NORMALS: return MeshFmt_Normal;
	case LBF::TEXCOORDS: return MeshFmt_Texcoord;
	case LBF::SKINMATS: return MeshFmt_SkinMat;
	case LBF::WEIGHTS: return MeshFmt_Weight;
	default: return -1;
	}
}

int MapFmtToLBF(int fmt) {
	switch(fmt) {
	case MeshFmt_Position: return LBF::POSITIONS ;
	case MeshFmt_Normal: return LBF::NORMALS ;
	case MeshFmt_Texcoord: return LBF::TEXCOORDS ;
	case MeshFmt_SkinMat: return LBF::SKINMATS ;
	case MeshFmt_Weight: return LBF::WEIGHTS ;
	default: return -1;
	}
}

LBF::WriteNode* Mesh::CreateMeshWriteNode( ) const
{
	if(m_format == 0)
		return 0;
	if(m_format_size == 0)
		return 0;

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

	LBF::WriteNode* fmtNode = new LBF::WriteNode( LBF::VTXFMT, 0, sizeof(int) + sizeof(int)*m_format_size);
	meshNode->AddChild(fmtNode);
	BufferWriter writer = fmtNode->GetWriter();
	writer.Put(&m_format_size, sizeof(m_format_size));
	for (int i = 0; i < m_format_size; ++i) {
		int save_type = MapFmtToLBF(m_format[i]);
		writer.Put(&save_type, sizeof(save_type));
	}

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

Mesh* Mesh::CreateMeshFromReadNode( const LBF::ReadNode& rn )
{
	if(!rn.Valid())
		return 0;

	Mesh *result = new Mesh;

	mesh_save_info save_info;
	memset(&save_info,0,sizeof(save_info));
	rn.GetData(&save_info, sizeof(save_info));	
	result->m_transform = save_info.transform;
	result->m_num_verts = save_info.num_verts;

	LBF::ReadNode rnName = rn.GetFirstChild(LBF::GEOM3D_NAME);
	if(rnName.Valid()) {
		result->m_name = std::string(rnName.GetNodeData(), rnName.GetNodeDataLength());
	} else {
		result->m_name = "noname";
	}

	LBF::ReadNode rnFmt = rn.GetFirstChild(LBF::VTXFMT);
	if(rnFmt.Valid()) {
		BufferReader reader = rnFmt.GetReader();
		int fmt_size = 0;
		reader.Get(&fmt_size,sizeof(fmt_size));
		if(fmt_size > 0) {
			result->m_format_size = fmt_size;
			long fmtDataSize = sizeof(int)*fmt_size;
			result->m_format = new int[fmt_size];
			reader.Get(result->m_format, fmtDataSize);
		} else {
			delete result; return 0;
		}
	} else {
		delete result; return 0;
	}

	LBF::ReadNode rnTri = rn.GetFirstChild(LBF::TRIMESH_INDICES);
	if(rnTri.Valid()) {
		int numIndices = rnTri.GetNodeDataLength() / sizeof(unsigned int); 
		if( (numIndices % 3) != 0 ) {
			fprintf(stderr, "mesh load error: Number of trimesh indices (%d) must be divisible by 3\n", numIndices);
			delete result; return 0;
		}

		result->m_num_tris = numIndices / 3;
		
		result->m_tri_index_buffer = new unsigned int[ numIndices ];
		rnTri.GetData(result->m_tri_index_buffer, rnTri.GetNodeDataLength());
	}

	LBF::ReadNode rnQuad = rn.GetFirstChild(LBF::QUADMESH_INDICES);
	if(rnQuad.Valid()) {
		int numIndices = rnQuad.GetNodeDataLength() / sizeof(unsigned int); 
		if( (numIndices % 4) != 0 ) {
			fprintf(stderr, "mesh load error: Number of quadmesh indices (%d) must be divisible by 4\n", numIndices);
			delete result; return 0;
		}

		result->m_num_quads = numIndices / 4;
		
		result->m_quad_index_buffer = new unsigned int[ numIndices ];
		rnTri.GetData(result->m_quad_index_buffer, rnTri.GetNodeDataLength());
	}

	// only load what is specified in the format.
	for(int i = 0; i < result->m_format_size; ++i) {
		int fmt = result->m_format[i];
		LBF::ReadNode rnChunk = rn.GetFirstChild(fmt);
		if(rnChunk.Valid()) {
			if(fmt == LBF::POSITIONS && result->m_positions == 0) {
				result->m_positions = new float[3*result->m_num_verts];
				rnChunk.GetData(result->m_positions, rnChunk.GetNodeDataLength());
			} else if(fmt == LBF::NORMALS && result->m_normals == 0) {
				result->m_normals = new float[3*result->m_num_verts];
				rnChunk.GetData(result->m_normals, rnChunk.GetNodeDataLength());
			} else if(fmt == LBF::WEIGHTS && result->m_skin_weights == 0) {
				result->m_skin_weights = new float[4*result->m_num_verts];
				rnChunk.GetData(result->m_skin_weights, rnChunk.GetNodeDataLength());
			} else if(fmt == LBF::SKINMATS && result->m_skin_mats == 0) {
				result->m_skin_mats = new char[4*result->m_num_verts];
				rnChunk.GetData(result->m_skin_mats, rnChunk.GetNodeDataLength());
			} else if(fmt == LBF::TEXCOORDS && result->m_texcoords == 0) {
				result->m_texcoords = new float[2*result->m_num_verts];
				rnChunk.GetData(result->m_texcoords, rnChunk.GetNodeDataLength());
			}
		}
	}
	
	for(int i = 0; i < result->m_format_size; ++i) 
		result->m_format[i] = MapLBFType(result->m_format[i]);
	return result;
}


Mesh* loadMesh(const char* filename)
{
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
			return Mesh::CreateMeshFromReadNode( rnGeom );
		}
	}

	return 0;
}
