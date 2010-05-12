#ifndef INCLUDED_anim_motion_graph_state_HH
#define INCLUDED_anim_motion_graph_state_HH

#include <vector>
#include "Vector.hh"
#include "blendcontroller.hh"
#include "clipdb.hh"
#include "motiongraph.hh"

class MGPath
{
	int m_max_size;
	std::vector< Vec3 > m_path_points;
	float m_len;
public:
	MGPath(int maxSize);

	int GetMaxSize() const { return m_max_size; }
	void SetMaxSize(int max_size) ;

	float TotalLength() const ;
	Vec3 PointAtLength(float length);

	bool AddPoint( Vec3_arg newPoint );
	void SmoothPath();
	void Clear();
	void Draw();
	bool Empty() { return m_path_points.empty(); }
	
	const Vec3& Back() { return m_path_points.back(); }
};

class MotionGraphState
{
	BlendController *m_blender;
	ClipController *m_clips[2];

	AlgorithmMotionGraphHandle m_algo_graph;
	std::vector< ClipHandle > m_cached_clips;

	MGPath m_requested_path;
	MGPath m_path_so_far;
	
public:
	MotionGraphState();

	void Reset();
	void SetRequestedPath( const MGPath& path );
};

#endif
