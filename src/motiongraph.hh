#ifndef INCLUDED_motiongraph_HH
#define INCLUDED_motiongraph_HH

#include <ostream>
#include <list>
#include <vector>
#include "Vector.hh"
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

	ClipHandle GetClip();
	const ClipHandle GetClip() const ;
private:
	void CacheHandle() const;
};

typedef reference<MGEdge> MGEdgeHandle;

sqlite3_int64 NewMotionGraph( sqlite3* db, sqlite3_int64 skel );

class MotionGraph
{
	sqlite3* m_db;
	sqlite3_int64 m_skel_id;
	sqlite3_int64 m_id;

	mutable Query m_stmt_count_edges;
	mutable Query m_stmt_count_nodes;
	mutable Query m_stmt_insert_edge;
	mutable Query m_stmt_insert_node;
	mutable Query m_stmt_get_edges;
	mutable Query m_stmt_get_edge;

	void PrepareStatements();
public:
	MotionGraph(sqlite3* db, sqlite3_int64 skel_id, sqlite3_int64 id);
	~MotionGraph();

	bool Valid() const { return m_id != 0; }
	sqlite3_int64 GetID() const { return m_id; }

	sqlite3_int64 AddEdge(sqlite3_int64 start, sqlite3_int64 finish, sqlite3_int64 clip_id);
	sqlite3_int64 AddNode();
	void GetEdgeIDs(std::vector<sqlite3_int64>& out) const;

	MGEdgeHandle GetEdge(sqlite3_int64 id) ;
	const MGEdgeHandle GetEdge(sqlite3_int64 id) const;

	int GetNumEdges() const ;
	int GetNumNodes() const ;
};


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
