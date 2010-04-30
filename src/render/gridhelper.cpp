#include <GL/gl.h>
#include "render/gridhelper.hh"

GridHelper::GridHelper(float dim, float subdiv)
	: m_dlist(0)
	, m_dim(dim)
	, m_subdiv(subdiv)
{
}

GridHelper::~GridHelper()
{
	if(m_dlist)
		glDeleteLists(m_dlist, 1);
}

void GridHelper::Draw()
{
	if(m_dlist) { 
		glCallList(m_dlist);
	} else {
		m_dlist = glGenLists(1);
		glNewList(m_dlist, GL_COMPILE);

		float min_x = -m_dim*0.5f;
		float max_x = -min_x;

		float min_z = -m_dim*0.5f;
		float max_z = -min_z;
	
		int count = m_dim/m_subdiv;
		float cur_x = min_x;
		float cur_z = min_z;

		glColor3f(0.2, 0.2, 0.2);
		glBegin(GL_LINES);
	
		for(int i = 0; i <= count; ++i) {
			glVertex3f(cur_x, 0.f, max_z);
			glVertex3f(cur_x, 0.f, min_z);

			glVertex3f(max_x, 0.f, cur_z);
			glVertex3f(min_x, 0.f, cur_z);

			cur_x += m_subdiv;
			cur_z += m_subdiv;
		}

		glEnd();
	
		glEndList();
	}
}

