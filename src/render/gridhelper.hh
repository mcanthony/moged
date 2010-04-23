#ifndef INCLUDED_render_gridhelper_HH
#define INCLUDED_render_gridhelper_HH

#include "Vector.hh"

class GridHelper
{
	GLuint m_dlist; // opengl display list
	float m_dim, m_subdiv;
public:
	// dim sized grid at origin along x and z.
	GridHelper(float dim, float subdiv);
	~GridHelper();
	void Draw();
};

#endif
