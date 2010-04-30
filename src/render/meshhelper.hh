#ifndef INCLUDED_render_meshhelper_HH
#define INCLUDED_render_meshhelper_HH

// for types...bleh
#include <GL/gl.h>

class Mesh;
class Pose;
struct program_data ;
class MeshHelper
{
	bool m_gpu_skinning;

	bool m_initialized;
	GLhandleARB m_vert_shader;
	GLhandleARB m_frag_shader;
	GLhandleARB m_program;

	program_data *m_data;
public:
	MeshHelper();
	~MeshHelper();

	void Init();

	void Draw(const Mesh* mesh, const Pose* pose);
private:
	void DrawCPU(const Mesh* mesh, const Pose* pose);
	void DrawGPU(const Mesh* mesh, const Pose* pose);
};

#endif
