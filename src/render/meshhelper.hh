#ifndef INCLUDED_render_meshhelper_HH
#define INCLUDED_render_meshhelper_HH

class Mesh;

// class for holding glsl programs later.
class MeshHelper
{
	const Mesh* m_mesh;
public:
	MeshHelper();

	// set before render!
	void SetMesh(const Mesh* mesh) { m_mesh = mesh; }
	
	void Draw();
};

#endif
