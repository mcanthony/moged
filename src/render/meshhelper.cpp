#include <GL/gl.h>
#include "meshhelper.hh"
#include "mesh.hh"

MeshHelper::MeshHelper()  
	: m_mesh(0)
	
{
}

void MeshHelper::Draw()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	
	if(m_mesh)
	{
		glColor3f(0.6,0.6,0.6);
		glVertexPointer(3, GL_FLOAT, 0, m_mesh->GetPositionPtr());
		glNormalPointer(GL_FLOAT, 0, m_mesh->GetNormalPtr());
		glTexCoordPointer(2, GL_FLOAT, 0, m_mesh->GetTexCoordPtr());

		glDrawElements(GL_TRIANGLES, 3*m_mesh->GetNumTris(), GL_UNSIGNED_INT, m_mesh->GetTriIndexBuffer());
		glDrawElements(GL_QUADS, 4*m_mesh->GetNumQuads(), GL_UNSIGNED_INT, m_mesh->GetQuadIndexBuffer());
	}	

	glDisableClientState(GL_VERTEX_ARRAY);
}
