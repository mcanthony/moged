#include <cstdio>
#include <GL/glew.h>
#include <GL/gl.h>
#include "cloudhelper.hh"
#include "Mat4.hh"
using namespace std;

CloudHelper::CloudHelper()
	: m_cloud(0)
	, m_count(0)
	, m_align_translation(0,0,0)
	, m_align_rotation(0)
	, m_vbo(0)
{
}

CloudHelper::~CloudHelper()
{
	if(m_vbo) {
		glDeleteBuffersARB(1, &m_vbo);
	}
}

void CloudHelper::SetCloud(const Vec3* cloud, int count)
{
	if(GLEW_ARB_vertex_buffer_object)
	{
		if(m_vbo) {
			glDeleteBuffersARB(1, &m_vbo);
			m_vbo = 0;
		}
	}

	m_cloud = cloud;
	m_count = count;
}

void CloudHelper::SetAlignment(float rotation, Vec3_arg trans)
{
	m_align_rotation = rotation;
	m_align_translation = trans;
}

void CloudHelper::Draw()
{
	if(m_cloud) {
		if(GLEW_ARB_vertex_buffer_object && m_vbo == 0) {
			glGenBuffersARB(1, &m_vbo);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vbo);
			glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_count*sizeof(float)*3, m_cloud, GL_STREAM_DRAW_ARB);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);			
		}
		Mat4 transform = transpose(translation(m_align_translation) * rotation_y(m_align_rotation));
		glPushMatrix();
		glMultMatrixf(transform.m);

		if(GLEW_ARB_vertex_buffer_object)
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vbo);
			glEnableClientState(GL_VERTEX_ARRAY); 
			glVertexPointer(3, GL_FLOAT, 0, 0);
			
			glDrawArrays(GL_POINTS, 0, m_count);
			glDisableClientState(GL_VERTEX_ARRAY); 			
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		}
		else
		{
			glEnableClientState(GL_VERTEX_ARRAY); 

			const int maxNumElements = 10000;
			glVertexPointer(3, GL_FLOAT, 0, (float*)m_cloud);

			int offset = 0;
			while(offset < m_count) {
				int thisCount = Min(m_count - offset, maxNumElements);
				glDrawArrays(GL_POINTS, offset, thisCount);
				break; // temporary?
				offset += thisCount;
			}
			glDisableClientState(GL_VERTEX_ARRAY); 
		}
		glPopMatrix();		
	}

}
