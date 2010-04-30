#include <cstdio>
#include <algorithm>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "meshhelper.hh"
#include "mesh.hh"
#include "anim/pose.hh"

using namespace std;

static const char* gVertshader = 
	"#version 120\n"
	"uniform mat4 skin_mats[40];\n"
	"attribute vec4 skin_weights;\n"
	"attribute vec4 skin_indices;\n	"
	"varying vec3 normal;\n"

	"void main()\n"
	"{\n"
	"    ivec4 indices = ivec4(skin_indices);\n"
	"    vec4 w1Pos = skin_mats[indices.x] * gl_Vertex;\n"
	"    vec4 w2Pos = skin_mats[indices.y] * gl_Vertex;\n"
	"    vec4 w3Pos = skin_mats[indices.z] * gl_Vertex;\n"
	"    vec4 w4Pos = skin_mats[indices.w] * gl_Vertex;\n"
	"    vec4 skinned_vert = w1Pos * skin_weights.x + w2Pos * skin_weights.y + w3Pos * skin_weights.z + w4Pos * skin_weights.w; \n"
	"    gl_Position = gl_ModelViewProjectionMatrix * skinned_vert;\n"
	"    normal = gl_NormalMatrix * gl_Normal;\n"
	"    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
	"}\n"	;

static const char* gFragshader = 
	"#version 120\n"
	"varying vec3 normal;\n"
	
	"void main()\n"
	"{\n"
	"    gl_FragColor = vec4(gl_TexCoord[0].st,0.6,1.0);\n"
	"}\n";


static void CheckError(const char* place = 0)
{
	int err = glGetError();
	if(err) {
		const GLubyte* str = gluErrorString(err);
		fprintf(stderr, "%s%sGL Error: %s\n", (place ? place : "") , (place ? ": " : ""), str);
	}
}

void PrintLog(GLhandleARB ob)
{
	int len = 0;
	glGetObjectParameterivARB(ob, GL_OBJECT_INFO_LOG_LENGTH_ARB, &len);
	
	if(len>0) {
		int charsUsed = 0;
		char* log = new char[len];
		glGetInfoLogARB(ob, len, &charsUsed, log);
		printf("%s\n", log);
		delete[] log;
	}
}

void CheckShaderCompile(GLhandleARB shader) {
	int val = 0;
	glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &val);
	if(!val) {
		printf("Shader errors:\n");
		PrintLog(shader);
	}
}

void CheckProgramLink(GLhandleARB program) {
	int val = 0;
	glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &val);
	if(!val) {
		PrintLog(program);
	}
}

struct program_data
{
	GLint skin_mats;
	GLint skin_weights;
	GLint skin_indices;
};

MeshHelper::MeshHelper()  
	: m_gpu_skinning(false)
	, m_initialized(false)
	, m_vert_shader(0)
	, m_frag_shader(0)
	, m_program(0)
	, m_data(0)
{
}

MeshHelper::~MeshHelper()
{
	if(m_gpu_skinning)
	{
		glDetachObjectARB(m_program, m_vert_shader);
		glDetachObjectARB(m_program, m_frag_shader);
		glDeleteObjectARB(m_vert_shader);
		glDeleteObjectARB(m_frag_shader);
		glDeleteProgramsARB(1, &m_program);
	}
	delete m_data;
}

void MeshHelper::Init()
{
	if(!m_initialized) {
		if(GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader) {
			printf("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
			m_vert_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
			glShaderSourceARB( m_vert_shader, 1, &gVertshader, NULL);
			glCompileShader(m_vert_shader);
			CheckShaderCompile(m_vert_shader);
		
			m_frag_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
			glShaderSourceARB( m_frag_shader, 1, &gFragshader, NULL);
			glCompileShaderARB(m_frag_shader);
			CheckShaderCompile(m_frag_shader);
		
			m_program = glCreateProgramObjectARB();
			glAttachObjectARB(m_program, m_vert_shader);
			glAttachObjectARB(m_program, m_frag_shader);
			glLinkProgramARB(m_program);	
			CheckProgramLink(m_program);
			glUseProgramObjectARB(m_program);
			CheckError();

			m_data = new program_data;
			// find attributes
			m_data->skin_mats = glGetUniformLocationARB(m_program, "skin_mats"); CheckError();
			m_data->skin_weights = glGetAttribLocationARB(m_program, "skin_weights"); CheckError();
			m_data->skin_indices = glGetAttribLocationARB(m_program, "skin_indices"); CheckError();

			glUseProgramObjectARB(0);
			std::printf("Using shaders for mesh rendering.\n");
			m_initialized = true;
			m_gpu_skinning = true;
		} else {
			std::printf("No vertex/fragment shader support!\n");
		}	
	}
}

void MeshHelper::Draw(const Mesh* mesh, const Pose* pose)
{
	if(!m_gpu_skinning) { // only one to render, so this branch isn't so bad :)
		DrawCPU(mesh, pose);
	} else {
		DrawGPU(mesh, pose);
	}
}

void MeshHelper::DrawCPU(const Mesh* mesh, const Pose* pose)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if(mesh)
	{
		glPushMatrix();
		glMultMatrixf( transpose(mesh->GetTransform()).m );

		glColor3f(0.6,0.6,0.6);
		glVertexPointer(3, GL_FLOAT, 0, mesh->GetPositionPtr());
		glNormalPointer(GL_FLOAT, 0, mesh->GetNormalPtr());
//		glTexCoordPointer(2, GL_FLOAT, 0, mesh->GetTexCoordPtr());

		glDrawElements(GL_TRIANGLES, 3*mesh->GetNumTris(), GL_UNSIGNED_INT, mesh->GetTriIndexBuffer());
		glDrawElements(GL_QUADS, 4*mesh->GetNumQuads(), GL_UNSIGNED_INT, mesh->GetQuadIndexBuffer());

		glPopMatrix();
	}	

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

void MeshHelper::DrawGPU(const Mesh* mesh, const Pose* pose)
{
	ASSERT(m_data);
	if(mesh && pose) {
		glUseProgramObjectARB(m_program); 

		glEnableClientState(GL_VERTEX_ARRAY); 
		glEnableClientState(GL_NORMAL_ARRAY); 
		glEnableClientState(GL_TEXTURE_COORD_ARRAY); 
		glEnableVertexAttribArrayARB(m_data->skin_weights); 
		glEnableVertexAttribArrayARB(m_data->skin_indices); 
		glUniformMatrix4fvARB(m_data->skin_mats, std::min(40,pose->GetNumJoints()), 1, &pose->GetMatricesPtr()[0].m[0]); 

		glDisable(GL_CULL_FACE);
		glPushMatrix();
		glMultMatrixf( transpose(mesh->GetTransform()).m );

		glVertexPointer(3, GL_FLOAT, 0, mesh->GetPositionPtr());
		glNormalPointer(GL_FLOAT, 0, mesh->GetNormalPtr());
		glTexCoordPointer(2, GL_FLOAT, 0, mesh->GetTexCoordPtr());
		glVertexAttribPointerARB(m_data->skin_weights, 4, GL_FLOAT, 0, 0, mesh->GetSkinWeightsPtr()); 
		glVertexAttribPointerARB(m_data->skin_indices, 4, GL_BYTE, 0, 0, mesh->GetSkinMatricesPtr()); 

		glDrawElements(GL_TRIANGLES, 3*mesh->GetNumTris(), GL_UNSIGNED_INT, mesh->GetTriIndexBuffer());
		glDrawElements(GL_QUADS, 4*mesh->GetNumQuads(), GL_UNSIGNED_INT, mesh->GetQuadIndexBuffer());

		glPopMatrix();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableVertexAttribArrayARB(m_data->skin_weights);
		glDisableVertexAttribArrayARB(m_data->skin_indices);

		glUseProgramObjectARB(0); CheckError("use program 0 ");
	}
}
