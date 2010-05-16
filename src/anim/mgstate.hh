#ifndef INCLUDED_anim_motion_graph_state_HH
#define INCLUDED_anim_motion_graph_state_HH

#include <vector>
#include "Vector.hh"
#include "Quaternion.hh"
#include "clipdb.hh"
#include "motiongraph.hh"

class ClipController;
class Skeleton;

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
	void Draw() const;
	bool Empty() const { return m_path_points.empty(); }
	bool Full() const { return (int)m_path_points.size() == m_max_size; }
	
	const Vec3& Front() { return m_path_points.front(); }
	const Vec3& Back() { return m_path_points.back(); }
};

struct SearchNode;

class MotionGraphState
{
	ClipController *m_clip_controller;

	AlgorithmMotionGraphHandle m_algo_graph;
	std::vector< ClipHandle > m_cached_clips;

	MGPath m_requested_path;
	MGPath m_path_so_far;

	AlgorithmMotionGraph::Node* m_last_node;
	AlgorithmMotionGraph::Edge* m_cur_edge;
	
	Vec3 m_cur_offset;
	Quaternion m_cur_rotation;

	std::list< AlgorithmMotionGraph::Edge* > m_edges_to_walk;
	bool m_walking;

	void FindBestGraphWalk();
	void CreateSearchNode(SearchNode& out, 
						  AlgorithmMotionGraph::Edge* edge,
						  SearchNode* parent);
	float ComputeError(const SearchNode& info);
	void NextEdge();

	std::vector< Vec3 > m_cur_search_nodes;
	std::vector< Vec3 > m_cur_path;
public:
	MotionGraphState();
	~MotionGraphState();

	void SetGraph( sqlite3*db, const Skeleton* skel, AlgorithmMotionGraphHandle handle );

	void ResetPaths();
	void SetRequestedPath( const MGPath& path );
	
	void DebugDraw();

	const MGPath& GetCurrentPath() const { return m_path_so_far; }

	void Update(float dt );

	const Pose* GetPose() const ;
	const Skeleton* GetSkeleton() const;
	void ComputeMatrices( Mat4_arg m ) ;
};

#endif
