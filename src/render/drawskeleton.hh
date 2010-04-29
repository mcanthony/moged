#ifndef INCLUDED_render_drawskeleton_HH
#define INCLUDED_render_drawskeleton_HH

class Skeleton;
class Vec3;
class Quaternion;

class DrawSkeletonHelper
{
	const Skeleton* m_skel;
	Vec3 *m_offsets;

public:
	DrawSkeletonHelper();
	~DrawSkeletonHelper();
	void Draw();
	void SetSkeleton(const Skeleton* skel);
};

#endif
