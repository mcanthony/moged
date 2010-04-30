#include <cstdio>
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
	glEnableClientState(GL_NORMAL_ARRAY);
	
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if(m_mesh)
	{
		glPushMatrix();
		glMultMatrixf( transpose(m_mesh->GetTransform()).m );

		glColor3f(0.6,0.6,0.6);
		glVertexPointer(3, GL_FLOAT, 0, m_mesh->GetPositionPtr());
		glNormalPointer(GL_FLOAT, 0, m_mesh->GetNormalPtr());
//		glTexCoordPointer(2, GL_FLOAT, 0, m_mesh->GetTexCoordPtr());

		glDrawElements(GL_TRIANGLES, 3*m_mesh->GetNumTris(), GL_UNSIGNED_INT, m_mesh->GetTriIndexBuffer());
		glDrawElements(GL_QUADS, 4*m_mesh->GetNumQuads(), GL_UNSIGNED_INT, m_mesh->GetQuadIndexBuffer());

		glPopMatrix();
	}	

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}
