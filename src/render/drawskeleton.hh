#ifndef INCLUDED_render_drawskeleton_HH
#define INCLUDED_render_drawskeleton_HH

#include <GL/glu.h>

class Skeleton;
class SkeletonWeights;
class Vec3;
class Quaternion;

class DrawSkeletonHelper
{
	const Skeleton* m_skel;
	const SkeletonWeights* m_weights;

	Vec3 *m_offsets;
	int *m_selected;

	bool m_initialized;
	GLUquadric *m_quadric;
public:
	DrawSkeletonHelper();
	~DrawSkeletonHelper();

	void Init();

	void Draw();
	void SetSkeleton(const Skeleton* skel, const SkeletonWeights* weights);
	void SetSelected(int idx, bool selected);
};

#endif
