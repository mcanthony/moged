#ifndef INCLUDED_motiongraph_HH
#define INCLUDED_motiongraph_HH

#include <ostream>
#include <list>
#include <vector>
#include "Vector.hh"
#include "Quaternion.hh"
#include "dbhelpers.hh"
#include "intrusive_ptr.hh"
#include "clipdb.hh"

class MGEdge;
class MGNode;

class Mesh;
class Skeleton;
class ClipController;
class Pose;
class Vec3;

class MGEdge : public refcounted_type<MGEdge>
{
	sqlite3* m_db;
	sqlite3_int64 m_id;
	sqlite3_int64 m_clip_id;
	sqlite3_int64 m_start_id;
	sqlite3_int64 m_finish_id;

	mutable bool m_error_mode; // true if we tried to get a clip before and it failed.
	mutable ClipHandle m_cached_clip;
public:
	MGEdge( sqlite3* db, sqlite3_int64 id, sqlite3_int64 clip, sqlite3_int64 start, sqlite3_int64 finish );
	bool Valid() const { return m_id != 0 && m_clip_id != 0 && m_start_id != 0 && m_finish_id != 0; }
	void Invalidate() { m_id = 0; } // used after splitting an edge - still want the clip, but don't want the id

	ClipHandle GetClip();
	const ClipHandle GetClip() const ;

	sqlite3_int64 GetID() const { return m_id; }
	sqlite3_int64 GetStartNodeID() const { return m_start_id; }
	sqlite3_int64 GetEndNodeID() const { return m_finish_id; }
	sqlite3_int64 GetClipID() const { return m_clip_id; }
private:
	void CacheHandle() const;
};
typedef reference<MGEdge> MGEdgeHandle;

struct MGNodeInfo
{
	sqlite3_int64 id;
	sqlite3_int64 clip_id;
	int frame_num;
};

struct MGClipInfo
{
	sqlite3_int64 id;
	float fps;
	int is_transition;
	int num_frames;
};

sqlite3_int64 NewMotionGraph( sqlite3* db, sqlite3_int64 skel, const char* name );
int CountMotionGraphs( sqlite3* db, sqlite3_int64 skel );

// This really should be called MotionGraphDB
class MotionGraph
{
	sqlite3* m_db;
	sqlite3_int64 m_skel_id;
	sqlite3_int64 m_id;
	std::string m_name;

	mutable Query m_stmt_count_edges;
	mutable Query m_stmt_count_nodes;
	mutable Query m_stmt_insert_edge;
	mutable Query m_stmt_insert_node;
	mutable Query m_stmt_find_node;
	mutable Query m_stmt_get_edges;
	mutable Query m_stmt_get_edge;
	mutable Query m_stmt_get_node;
	mutable Query m_stmt_delete_edge;
	mutable Query m_stmt_add_t_edge;

	void PrepareStatements();
public:
	MotionGraph(sqlite3* db, sqlite3_int64 skel_id, sqlite3_int64 id);
	~MotionGraph();

	bool Valid() const { return m_id != 0; }
	sqlite3_int64 GetID() const { return m_id; }

	const char* GetName() const { return m_name.c_str(); }

	sqlite3_int64 AddEdge(sqlite3_int64 start, sqlite3_int64 finish, sqlite3_int64 clip_id);
	sqlite3_int64 AddTransitionEdge(sqlite3_int64 start, sqlite3_int64 finish, sqlite3_int64 clip_id,
									Vec3_arg align_offset, Quaternion_arg align_rot);
	sqlite3_int64 AddNode(sqlite3_int64 clip_id, int frame_num);
	sqlite3_int64 FindNode(sqlite3_int64 clip_id, int frame_num) const;

	bool GetNodeInfo(sqlite3_int64 node_id, MGNodeInfo& out) const;

	// returns node subdividing an edge_id. edge is DELETED and two edges are added.
	// if left or right is not null, then they are filled witht he new edge ids on success.
	sqlite3_int64 SplitEdge(sqlite3_int64 edge_id, int frame_num, sqlite3_int64* left, sqlite3_int64* right);

	bool DeleteEdge(sqlite3_int64 id);

	void GetEdgeIDs(std::vector<sqlite3_int64>& out) const;

	MGEdgeHandle GetEdge(sqlite3_int64 id) ;
	const MGEdgeHandle GetEdge(sqlite3_int64 id) const;

	int GetNumEdges() const ;
	int GetNumNodes() const ;
};

bool exportMotionGraphToGraphViz(sqlite3* db, sqlite3_int64 graph_id, const char* filename );

void populateInitialMotionGraph(MotionGraph* graph, const ClipDB* clips, 
								std::ostream& out);

void selectMotionGraphSampleVerts(const Mesh* mesh, int num, std::vector<int> &out);

// samples must be of length sample_indices.size() * num_samples
// Samples controller num_samples times at sample_interval increments.
void getPointCloudSamples(Vec3* samples, 
						  const Mesh* mesh,
						  const Skeleton* skel, 
						  const std::vector<int>& sample_indices, 
						  const Clip* clip, 
						  int num_samples, 
						  float sample_interval, 
						  int numThreads);

void computeCloudAlignment(const Vec3* from_cloud,
						   const Vec3* to_cloud,
						   int points_per_frame,
						   int num_frames,
						   const float *weights,
						   float inv_total_weights,
						   Vec3& align_translation,
						   float& align_rotation, 
						   int numThreads);

float computeCloudDifference(const Vec3* from_cloud,
							 const Vec3* to_cloud,
							 const float *weights,
							 int points_per_frame,
							 int num_frames,
							 Vec3_arg align_translation,
							 float align_rotation,
							 int numThreads);

void findErrorFunctionMinima(const float* error_values, 
							 int width, 
							 int height, 
							 std::vector<int>& out_mimima_indices);


sqlite3_int64 createTransitionClip(sqlite3* db, 
								   const Skeleton *skel,
								   const Clip* from, 
								   const Clip* to, 
								   float from_start,
								   float to_start,
								   int num_frames, 
								   float sample_interval, 
								   Vec3_arg align_translation, 
								   float align_rotation);


#endif
