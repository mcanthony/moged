#ifndef INCLUDED_render_cloudhelper_HH
#define INCLUDED_render_cloudhelper_HH

#include <GL/gl.h>
#include "Vector.hh"

class CloudHelper
{
	const Vec3* m_cloud;
	int m_count;
	Vec3 m_align_translation;
	float m_align_rotation;

	GLuint m_vbo;
public:
	CloudHelper();
	~CloudHelper();
	void SetCloud(const Vec3* cloud, int count);
	void SetAlignment(float rotation, Vec3_arg trans);
	void Draw();
};

#endif
