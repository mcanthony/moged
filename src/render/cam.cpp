#include <cstdio>
#include <GL/gl.h>
#include <GL/glu.h>
#include "cam.hh"

Camera::Camera()               
	: m_fov(45.0f)
	, m_w(1.0f)
	, m_h(1.0f)
	, m_near(0.1f)
	, m_far(1000.0f)
	, m_pos(0.0f,0.0f,0.0f)
	, m_lookdir(1.0f,0.0f,0.0f)
{                       
}
                
void Camera::Draw() const
{
	// replace with computed matrix
	glViewport(0,0,(GLsizei)m_w,(GLsizei)m_h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(m_fov,(m_w/m_h),m_near,m_far);
	glMatrixMode(GL_MODELVIEW);              
	glLoadIdentity();

	Vec3 tgt = m_pos + m_lookdir;
	gluLookAt(m_pos.x,m_pos.y,m_pos.z,
			  tgt.x, tgt.y, tgt.z,
			  0.0f, 1.0f, 0.0f);
}

void Camera::SetFOV(float fov)
{
	m_fov = fov;
}

void Camera::SetDimensions(float w, float h)
{
	m_w = w; m_h = h;
}

void Camera::SetZClip(float znear, float zfar)
{
	m_near = znear;
	m_far = zfar;
}

void Camera::SetPosition(const Vec3& position)
{
	m_pos = position;
}

void Camera::SetLookDir(const Vec3& lookdir)
{
	m_lookdir = lookdir;
}

void Camera::LookAt(const Vec3& pos)
{
	m_lookdir = pos - m_pos;
}

Mat4 Camera::GetMatrix() const
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	Vec3 tgt = m_pos + m_lookdir;
	gluLookAt(m_pos.x,m_pos.y,m_pos.z,
			  tgt.x, tgt.y, tgt.z,
			  0.0f, 1.0f, 0.0f);
	Mat4 result;
	glGetFloatv( GL_MODELVIEW_MATRIX, result.m );
	glPopMatrix();
	return result;
}

Vec3 Camera::GetDirectionFromScreen(float x, float y) 
{
	Mat4 m = GetMatrix();

	float y_dim = tan(0.5f * m_fov * TO_RAD);
	float x_dim = y_dim * m_w/m_h;
	x_dim *= 2.0f; y_dim *= 2.0f;

	float xcomp = x/(m_w-1.0f) - 0.5f;
	float ycomp = 0.5f - y/(m_h-1.0f);
	
	return normalize(matrix_col(m, 0) * x_dim * xcomp + 
					 matrix_col(m, 1) * y_dim * ycomp +
					 -matrix_col(m, 2) );	
}
