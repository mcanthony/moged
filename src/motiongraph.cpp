#include <ostream>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <cstdio>
#include <fstream>
#include <omp.h>
#include "sql/sqlite3.h"
#include "motiongraph.hh"
#include "assert.hh"
#include "clip.hh"
#include "clipdb.hh"
#include "mesh.hh"
#include "Vector.hh"
#include "anim/clipcontroller.hh"
#include "anim/pose.hh"
#include "Mat4.hh"
#include "skeleton.hh"
#include "assert.hh"

using namespace std;

sqlite3_int64 NewMotionGraph( sqlite3* db, sqlite3_int64 skel, const char* name )
{
	Query new_mg(db, "INSERT INTO motion_graphs (skel_id, name) VALUES(?,?)");
	new_mg.BindInt64(1, skel);
	new_mg.BindText(2, name);
	new_mg.Step();
	return new_mg.LastRowID();
}

int CountMotionGraphs( sqlite3* db, sqlite3_int64 skel )
{
	Query count_mg(db, "SELECT count(*) FROM motion_graphs WHERE skel_id = ?");
	count_mg.BindInt64(1, skel);
	count_mg.Step();
	return count_mg.ColInt(0);
}

MotionGraph::MotionGraph(sqlite3* db, sqlite3_int64 skel_id, sqlite3_int64 id)
	: m_db(db)
	, m_skel_id(skel_id)
	, m_id(id)
	, m_stmt_count_edges (db)
	, m_stmt_count_nodes (db)
	, m_stmt_insert_edge (db)
	, m_stmt_insert_node (db)
	, m_stmt_find_node (db)
	, m_stmt_get_edges (db)
	, m_stmt_get_edge(db)
	, m_stmt_get_node(db)
	, m_stmt_delete_edge(db)
	, m_stmt_add_t_edge(db)
{

	Query check_id(m_db, "SELECT id,name FROM motion_graphs WHERE id = ? and skel_id = ?");
	check_id.BindInt64(1, id);
	check_id.BindInt64(2, skel_id);
	m_id = 0;
	if( check_id.Step()) {
		m_id = check_id.ColInt64(0);
		m_name = check_id.ColText(1);
	}
	if(Valid())
		PrepareStatements();
}

void MotionGraph::PrepareStatements()
{
	m_stmt_count_edges.Init("SELECT count(*) FROM motion_graph_edges WHERE motion_graph_id = ?");
	m_stmt_count_edges.BindInt64(1, m_id);

	m_stmt_count_nodes.Init("SELECT count(*) FROM motion_graph_nodes WHERE motion_graph_id = ?");
	m_stmt_count_nodes.BindInt64(1, m_id);
	
	m_stmt_insert_edge.Init("INSERT INTO motion_graph_edges "
							"(motion_graph_id, clip_id, start_id, finish_id) "
							"VALUES (?,?,?,?)");
	m_stmt_insert_edge.BindInt64(1, m_id);

	m_stmt_insert_node.Init("INSERT INTO motion_graph_nodes (motion_graph_id, clip_id, frame_num) VALUES ( ?,?,? )");
	m_stmt_insert_node.BindInt64(1, m_id);

	m_stmt_find_node.Init("SELECT id FROM motion_graph_nodes WHERE motion_graph_id = ? AND clip_id = ? AND frame_num = ?");
	m_stmt_find_node.BindInt64(1, m_id);

	m_stmt_get_edges.Init("SELECT id FROM motion_graph_edges WHERE motion_graph_id = ?");
	m_stmt_get_edges.BindInt64(1, m_id);

	m_stmt_get_edge.Init("SELECT clip_id,start_id,finish_id FROM motion_graph_edges WHERE motion_graph_id = ? AND id = ?");
	m_stmt_get_edge.BindInt64(1, m_id);

	m_stmt_get_node.Init("SELECT id,clip_id,frame_num FROM motion_graph_nodes WHERE motion_graph_id = ? AND id = ?");
	m_stmt_get_node.BindInt64(1, m_id);

	m_stmt_delete_edge.Init("DELETE FROM motion_graph_edges WHERE id = ? and motion_graph_id = ?");
	m_stmt_delete_edge.BindInt64(2, m_id);

	m_stmt_add_t_edge.Init("INSERT INTO motion_graph_edges(motion_graph_id, clip_id, start_id, finish_id, "
						   "align_t_x, align_t_y, align_t_z, "
						   "align_q_a, align_q_b, align_q_c, align_q_r) "
						   "VALUES (?, ?, ?, ?, ?,?,?, ?,?,?,?)");
	m_stmt_add_t_edge.BindInt64(1, m_id);
}

MotionGraph::~MotionGraph()
{
}

sqlite3_int64 MotionGraph::AddEdge(sqlite3_int64 start, sqlite3_int64 finish, sqlite3_int64 clip_id)
{
	m_stmt_insert_edge.Reset();
	m_stmt_insert_edge.BindInt64(2, clip_id);
	m_stmt_insert_edge.BindInt64(3, start);
	m_stmt_insert_edge.BindInt64(4, finish);
	m_stmt_insert_edge.Step();
	return m_stmt_insert_edge.LastRowID();
}

sqlite3_int64 MotionGraph::AddTransitionEdge(sqlite3_int64 start, sqlite3_int64 finish, sqlite3_int64 clip_id,
											 Vec3_arg align_offset, Quaternion_arg align_rot)
{
	m_stmt_add_t_edge.Reset();
	m_stmt_add_t_edge.BindInt64(2, clip_id).BindInt64(3, start).BindInt64(4, finish)
		.BindVec3(5, align_offset).BindQuaternion(8, align_rot);
	m_stmt_add_t_edge.Step();
	return m_stmt_add_t_edge.LastRowID();
}

sqlite3_int64 MotionGraph::AddNode(sqlite3_int64 clip_id, int frame_num)
{
	m_stmt_insert_node.Reset();
	m_stmt_insert_node.BindInt64(2, clip_id);
	m_stmt_insert_node.BindInt(3, frame_num);
	m_stmt_insert_node.Step();
	return m_stmt_insert_node.LastRowID();
}

sqlite3_int64 MotionGraph::FindNode(sqlite3_int64 clip_id, int frame_num) const
{
	m_stmt_find_node.Reset();
	m_stmt_find_node.BindInt64(2, clip_id);
	m_stmt_find_node.BindInt(3, frame_num);
	if( m_stmt_find_node.Step() ) {
		return m_stmt_find_node.ColInt64(0);
	} 
	return 0;
}

bool MotionGraph::GetNodeInfo(sqlite3_int64 node_id, MGNodeInfo& out) const
{
	out.id = 0;
	m_stmt_get_node.Reset();
	m_stmt_get_node.BindInt64(2, node_id);
	if(m_stmt_get_node.Step()) {
		out.id = m_stmt_get_node.ColInt64(0);
		out.clip_id = m_stmt_get_node.ColInt64(1);
		out.frame_num = m_stmt_get_node.ColInt(2);
		return true;
	}
	return false;
}

sqlite3_int64 MotionGraph::SplitEdge(sqlite3_int64 edge_id, int frame_num, sqlite3_int64* left, sqlite3_int64* right)
{
	if(left) *left = 0;
	if(right) *right = 0;

	MGEdgeHandle edgeData = GetEdge( edge_id );
	if(!edgeData) {
		fprintf(stderr, "SplitEdge: Missing edge data\n");
		return 0;
	}

	sqlite3_int64 original_start_id = edgeData->GetStartNodeID();
	sqlite3_int64 original_end_id = edgeData->GetEndNodeID();

	MGNodeInfo start_info, end_info;
	if(!GetNodeInfo(original_start_id, start_info)) return 0;
	if(!GetNodeInfo(original_end_id, end_info)) return 0;

	if(start_info.clip_id != end_info.clip_id ||
	   start_info.clip_id != edgeData->GetClipID()) {
		ASSERT(false);
		fprintf(stderr, "WEIRD ERROR: Attempting to split what looks like a position. Don't split transitions!\n");
		return 0;
	}

	if(frame_num <= start_info.frame_num || 
	   frame_num >= end_info.frame_num) {
		fprintf(stderr, "SplitEdge: requested split is out of range (split at %d between %d and %d\n",
				frame_num, start_info.frame_num, end_info.frame_num);
		return 0;
	}

	sqlite3_int64 mid_point_node = 0;
	{
		SavePoint save(m_db, "splitEdge");
		mid_point_node = AddNode(edgeData->GetClipID(), frame_num);
		if(mid_point_node == 0) { 
			fprintf(stderr, "SplitEdge: failed to add midpoint node\n");
			save.Rollback(); return 0; 
		}

		sqlite3_int64 new_edge_left = AddEdge(original_start_id, mid_point_node, edgeData->GetClipID());
		if(new_edge_left == 0) { 
			fprintf(stderr, "SplitEdge: failed to add new left edge\n");
			save.Rollback(); return 0; 
		}
		
		sqlite3_int64 new_edge_right = AddEdge(mid_point_node, original_end_id, edgeData->GetClipID());
		if(new_edge_right == 0) { 
			fprintf(stderr, "SplitEdge: failed to add new right edge\n");
			save.Rollback(); return 0; 
		}
		
		if(!DeleteEdge(edgeData->GetID())) { 
			fprintf(stderr, "SplitEdge: failed to delete old edge\n");
			save.Rollback(); return 0; 
		}

		if(left) *left = new_edge_left ;
		if(right) *right = new_edge_right;
	}	
	return mid_point_node;
}

bool MotionGraph::DeleteEdge(sqlite3_int64 id)
{
	m_stmt_delete_edge.Reset();
	m_stmt_delete_edge.BindInt64(1, id);
	m_stmt_delete_edge.Step();
	
	return !m_stmt_delete_edge.IsError();
}

void MotionGraph::GetEdgeIDs(std::vector<sqlite3_int64>& out) const
{
	out.clear();
	m_stmt_get_edges.Reset();
	while(m_stmt_get_edges.Step()) {
		out.push_back( m_stmt_get_edges.ColInt64(0) );
	}
}

MGEdgeHandle MotionGraph::GetEdge(sqlite3_int64 id) 
{
	m_stmt_get_edge.Reset();
	m_stmt_get_edge.BindInt64(2, id);

	if(m_stmt_get_edge.Step()) 
	{
		sqlite3_int64 clip_id = m_stmt_get_edge.ColInt64(0);
		sqlite3_int64 start_id = m_stmt_get_edge.ColInt64(1);
		sqlite3_int64 finish_id = m_stmt_get_edge.ColInt64(2);

		MGEdgeHandle result = new MGEdge(m_db, id, clip_id, start_id, finish_id);
		return result;		
	} else 
		return MGEdgeHandle();
}

const MGEdgeHandle MotionGraph::GetEdge(sqlite3_int64 id) const
{
	m_stmt_get_edge.Reset();
	m_stmt_get_edge.BindInt64(2, id);

	if(m_stmt_get_edge.Step()) 
	{
		sqlite3_int64 clip_id = m_stmt_get_edge.ColInt64(0);
		sqlite3_int64 start_id = m_stmt_get_edge.ColInt64(1);
		sqlite3_int64 finish_id = m_stmt_get_edge.ColInt64(2);

		MGEdgeHandle result = new MGEdge(m_db, id, clip_id, start_id, finish_id);
		return result;		
	} else 
		return MGEdgeHandle();
}


int MotionGraph::GetNumEdges() const 
{
	m_stmt_count_edges.Reset();
	if(m_stmt_count_edges.Step()) {
		return m_stmt_count_edges.ColInt(0);
	}
	return 0;
}

int MotionGraph::GetNumNodes() const 
{
	m_stmt_count_nodes.Reset();
	if (m_stmt_count_nodes.Step()) {
		return m_stmt_count_nodes.ColInt(0);
	}
	return 0;
}

AlgorithmMotionGraphHandle MotionGraph::GetAlgorithmGraph() const
{
	AlgorithmMotionGraphHandle handle = new AlgorithmMotionGraph(m_db, m_id);
	Query get_nodes(m_db, "SELECT id FROM motion_graph_nodes WHERE motion_graph_id = ?");
	get_nodes.BindInt64(1, m_id);

	Query get_edges(m_db, "SELECT id,start_id,finish_id,clip_id FROM motion_graph_edges WHERE motion_graph_id = ?");
	get_edges.BindInt64(1, m_id);

	Query get_annos(m_db, "SELECT annotation_id FROM clip_annotations WHERE clip_id = ?");

	while(get_nodes.Step()) {
		handle->AddNode( get_nodes.ColInt64(0) );
	}

	while(get_edges.Step()) {
		AlgorithmMotionGraph::Node* start = handle->FindNode( get_edges.ColInt64(1) );
		AlgorithmMotionGraph::Node* end = handle->FindNode( get_edges.ColInt64(2) );
		sqlite3_int64 clip_id = get_edges.ColInt64(3);
		ASSERT(start && end);
		AlgorithmMotionGraph::Edge* edge = handle->AddEdge(start, end, get_edges.ColInt64(0));

		get_annos.Reset();
		get_annos.BindInt64(1, clip_id);
		while( get_annos.Step()) {
			edge->annotations.push_back( get_annos.ColInt64(0) );
		}
	}

	return handle;
}

float MotionGraph::CountClipTimeWithAnno(sqlite3_int64 anno) const
{
	Query count_clip_time(m_db);
	if(anno == 0) {
		count_clip_time.Init(
			"SELECT sum(time) FROM ("
			"SELECT count(*)/clips.fps as time FROM clips "
			"LEFT JOIN frames ON frames.clip_id = clips.id "
			"WHERE clips.id IN "
			"(SELECT motion_graph_edges.clip_id FROM motion_graph_edges "
			"LEFT JOIN clip_annotations ON "
			"motion_graph_edges.clip_id = clip_annotations.clip_id "
			"WHERE motion_graph_edges.motion_graph_id = ? "
			") " // this should be the only difference from the chunk below.. 
			"GROUP BY frames.clip_id"
			")");
	} else {
		count_clip_time.Init(
			"SELECT sum(time) FROM ("
			"SELECT count(*)/clips.fps as time FROM clips "
			"LEFT JOIN frames ON frames.clip_id = clips.id "
			"WHERE clips.id IN "
			"(SELECT motion_graph_edges.clip_id FROM motion_graph_edges "
			"LEFT JOIN clip_annotations ON "
			"motion_graph_edges.clip_id = clip_annotations.clip_id "
			"WHERE motion_graph_edges.motion_graph_id = ? "
			"AND clip_annotations.annotation_id = ?) "
			"GROUP BY frames.clip_id"
			")");
		count_clip_time.BindInt64(2, anno);
	}
	count_clip_time.BindInt64(1, m_id);

	if(count_clip_time.Step()) {
		return count_clip_time.ColDouble(0);
	}
	return 0.f;
}

////////////////////////////////////////////////////////////////////////////////
MGEdge::MGEdge( sqlite3* db, sqlite3_int64 id, sqlite3_int64 clip, sqlite3_int64 start, sqlite3_int64 finish )
	: m_db(db)
	, m_id(id)
	, m_clip_id(clip)
	, m_start_id(start)
	, m_finish_id(finish)
	, m_error_mode(false)
{
}

ClipHandle MGEdge::GetClip()
{
	CacheHandle();
	return m_cached_clip;
}

const ClipHandle MGEdge::GetClip() const
{
	CacheHandle();
	return m_cached_clip;
}

void MGEdge::CacheHandle() const
{
	if(!m_error_mode && !m_cached_clip) {
		m_cached_clip = new Clip(m_db, m_clip_id);
		if(!m_cached_clip->Valid())
		{
			m_error_mode = true;
			m_cached_clip = 0;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
void populateInitialMotionGraph(MotionGraph* graph, 
								const ClipDB* clips,
								std::ostream& out)
{
	std::vector<ClipInfoBrief> clip_infos;
	clips->GetAllClipInfoBrief(clip_infos);

	const int num_clips = clip_infos.size();
	for(int i = 0; i < num_clips; ++i)
	{
		sqlite3_int64 start = graph->AddNode( clip_infos[i].id , 0 );
		sqlite3_int64 end = graph->AddNode( clip_infos[i].id , clip_infos[i].num_frames - 1);
		graph->AddEdge( start, end, clip_infos[i].id);
	}
	
	out << "Using " << num_clips << " clips." << endl <<
		"Created graph with " << graph->GetNumEdges() << " edges and " << graph->GetNumNodes() << " nodes." << endl;
}

// rand index over vertex buffer. 
// This may not give the best distribution over the character necessarily since
// it doesn't take into account density of mesh (where we probably want number of points
// in an area to be roughly the same).
// can try a stratified sampling approach if this is bad.
void selectMotionGraphSampleVerts(const Mesh* mesh, int num, std::vector<int> &out)
{
	std::vector<int> candidates ;
	const int num_verts = mesh->GetNumVerts();
	candidates.reserve(num_verts);
	for(int i = 0; i < num_verts; ++i) 
		candidates.push_back(i);
	
	out.clear();
	out.reserve(num);

	for(int i = 0; i < num; ++i) 
	{
		int rand_index = rand() % candidates.size();

		out.push_back(candidates[rand_index]);
		candidates.erase( candidates.begin() + rand_index );
	}

	// sort the list so we at least access data in a predictable way
	std::sort( out.begin(), out.end());
}

// Get point from a clip controller. 
// 'samples' must be of at least num_samples * sample_indicies.size() in length.
// It begins by starting at a particular frame, and then sampling every sample_interval for num_samples iterations.
// This allows for clips that are running at variable FPS to be used.
// Pose must be initialized with the same skeleton as the clip_controller.

static void poseSamples(Vec3* out, int num_samples, const std::vector<int>& sample_indices, 
						const Mesh* mesh, const Pose* pose)
{
	const float *vert_data = mesh->GetPositionPtr();
	const char *mat_indices = mesh->GetSkinMatricesPtr();
	const float *weights = mesh->GetSkinWeightsPtr();
	
	const Mat4* mats = pose->GetMatricesPtr();

	Vec3 v0,v1,v2,v3;
	int i = 0;

	for(i = 0; i < num_samples; ++i) {
		int sample = sample_indices[i];
		int vert_sample = sample*3;
		int mat_sample = sample*4;
		Vec3 mesh_vert(vert_data[vert_sample],vert_data[vert_sample+1],vert_data[vert_sample+2]);
		const float *w = &weights[mat_sample];
		const char* indices = &mat_indices[mat_sample];
		ASSERT(indices[0] < pose->GetNumJoints() && 
			   indices[1] < pose->GetNumJoints() && 
			   indices[2] < pose->GetNumJoints() && 
			   indices[3] < pose->GetNumJoints());
		const Mat4& m1 = mats[ int(indices[0]) ];
		const Mat4& m2 = mats[ int(indices[1]) ];
		const Mat4& m3 = mats[ int(indices[2]) ];
		const Mat4& m4 = mats[ int(indices[3]) ];
	
		v0 = transform_point( m1, mesh_vert );
		v1 = transform_point( m2, mesh_vert );
		v2 = transform_point( m3, mesh_vert );
		v3 = transform_point( m4, mesh_vert );

		(void)w;
		out[i] = w[0] * v0 + 
			w[1] * v1 + 
			w[2] * v2 + 
			w[3] * v3;
	}
}

void getPointCloudSamples(Vec3* samples, 
						  const Mesh* mesh,
						  const Skeleton* skel, 
						  const std::vector<int>& sample_indices, 
						  const Clip* clip,
						  int num_samples, 
						  float sample_interval,
						  int numThreads)
{
	ASSERT(samples);
	ASSERT(numThreads > 0);
	int i;
	int frame_offset, tid;
	// init
	const int frame_length_in_samples = sample_indices.size();
	Pose** poses = new Pose*[numThreads];
	for(i = 0; i < numThreads; ++i) {
		poses[i] = new Pose(skel);
	}
	ClipController *controllers = new ClipController[numThreads];
	for(int i = 0; i < numThreads; ++i) {
		controllers[i].SetSkeleton(skel);
		controllers[i].SetClip(clip);
	}

	memset(samples, 0, sizeof(Vec3)*num_samples*frame_length_in_samples);

	omp_set_num_threads(numThreads);

	// processing
#pragma omp parallel for												\
	shared(poses,controllers,mesh,skel,sample_indices,samples,sample_interval,num_samples) \
	private(i,tid,frame_offset)
	for(i = 0; i < num_samples; ++i)
	{
		tid = omp_get_thread_num();
		ASSERT(tid < numThreads);
		frame_offset = i * frame_length_in_samples;

		controllers[tid].SetTime( i * sample_interval );
		controllers[tid].ComputePose(poses[tid]);

		poses[tid]->ComputeMatrices(skel, mesh->GetTransform());

		poseSamples(&samples[frame_offset],
					frame_length_in_samples,
					sample_indices, 
					mesh, 
					poses[tid]);
	}

	// clean up
	for(int i = 0; i < numThreads; ++i) {
		delete poses[i];
	}
	delete[] poses;
	delete[] controllers;
}

void computeCloudAlignment(const Vec3* from_cloud,
						   const Vec3* to_cloud,
						   int points_per_frame,
						   int num_frames,
						   const float *weights,
						   float inv_total_weights,
						   Vec3& align_translation,
						   float& align_rotation,
						   int numThreads)
{
	// compute according to minimization solution in kovar paper
	omp_set_num_threads(numThreads);

	const int total_num_samples = num_frames * points_per_frame;
	double total_from_x = 0.f, total_from_z = 0.f;
	double total_to_x = 0.f, total_to_z = 0.f;
	int i = 0;
	// a = w * (x * z' - x' * z) sum over i
	double a = 0.f;
	// b = w * (x * x' + z * z') sum over i
	double b = 0.f;
	double w = 0.f;

#pragma omp parallel for private(i,w) shared(from_cloud,to_cloud,weights) reduction(+:total_from_x,total_from_z,total_to_x,total_to_z,a,b)
	for(i = 0; i < total_num_samples; ++i)
	{
		w = weights[i];
		total_from_x += from_cloud[i].x * w;
		total_from_z += from_cloud[i].z * w;
		total_to_x += to_cloud[i].x * w;
		total_to_z += to_cloud[i].z * w;		
		a += w * (from_cloud[i].x * to_cloud[i].z - to_cloud[i].x * from_cloud[i].z);
		b += w * (from_cloud[i].x * to_cloud[i].x + from_cloud[i].z * to_cloud[i].z);
	}

	double angle = atan2( a - double(inv_total_weights) * (total_from_x * total_to_z - total_to_x * total_from_z),
						  b - double(inv_total_weights) * (total_from_x * total_to_x + total_from_z * total_to_z) );

	double cos_a = cos(angle);
	double sin_a = sin(angle);

	float x = double(inv_total_weights) * (total_from_x - total_to_x * cos_a - total_to_z * sin_a);
	float z = double(inv_total_weights) * (total_from_z + total_to_x * sin_a - total_to_z * cos_a);
	
	align_translation.set(x,0,z);
	align_rotation = float(angle);
}


float computeCloudDifference(const Vec3* from_cloud,
							 const Vec3* to_cloud,
							 const float* weights,
							 int points_per_frame,
							 int num_frames,
							 Vec3_arg align_translation,
							 float align_rotation,
							 int numThreads)
{
	omp_set_num_threads(numThreads);

	Mat4 transform = translation(align_translation) * rotation_y(align_rotation);
	const int total_num_samples = points_per_frame * num_frames;
	int i = 0;

	float diff = 0.f;

#pragma omp parallel for private(i) shared(transform, weights, from_cloud, to_cloud) reduction(+:diff)
	for(i = 0; i < total_num_samples; ++i)
	{
		float w = weights[i];
		Vec3 from = from_cloud[i];
		Vec3 to = transform_point(transform,to_cloud[i]);
		float magsq = w * magnitude_squared(from-to);
		diff += magsq;
	}

	return diff;
}

void findErrorFunctionMinima(const float* error_values, int width, int height, std::vector<int>& out_minima_indices)
{
	out_minima_indices.clear();

	const int window_size = 3;
	const int total = width*height;
	int i = 0;

	int* is_minima = new int[total];
	for(i = 0; i < total; ++i) is_minima[i] = 1;
	
#pragma omp parallel for private(i) shared(is_minima, width, height, error_values)
	for(i = 0; i < total; ++i) 
	{
		int x = i % width;
		int y = i / width;

		float middle = error_values[i];
		
		int min_x = Max(0, x - window_size);
		int max_x = Min(width, x + window_size);
		int min_y = Max(0, y - window_size);
		int max_y = Min(height, y + window_size);
		
		for(int cur_y = min_y; cur_y < max_y; ++cur_y)
		{			
			int offset = cur_y * width + min_x;
			for(int cur_x = min_x; cur_x < max_x; ++cur_x)
			{
				if(offset != i) {
					float val = error_values[offset];
					if(val < middle) {
						is_minima[i] = 0;
						break;
					}				
				}
				++offset;
			}
		}
	}

	for(i = 0; i < total; ++i) 
		if(is_minima[i]) 
			out_minima_indices.push_back(i);

	delete[] is_minima;
}

static inline float compute_blend_param(int p, int k)
{
	// interpolation scheme from Kovar paper - basically just a 
	// cubic with f(0) = 1, f(1) = 0, f'(0) = f'(1) = 0
	float term = (p+1)/float(k);
	float term2 = term*term;
	float term3 = term2*term;	
	float param = 2.f * term3 - 3.f * term2 + 1.f;
	return Clamp(param, 0.f, 1.f);
}

static void blendClips(const Skeleton *skel,
					   const Clip* from, const Clip* to, 
					   float from_start, float to_start,
					   int num_frames, float sample_interval,
					   Vec3_arg align_translation, 
					   float align_rotation,
					   std::vector< Vec3 >& root_translations,
					   std::vector< Quaternion >& root_rotations,
					   std::vector< Quaternion >& frame_rotations )
{
	root_translations.clear();
	root_rotations.clear();
	frame_rotations.clear();

	const int num_joints = skel->GetNumJoints();
	root_translations.resize(num_frames);
	root_rotations.resize(num_frames);
	frame_rotations.resize(num_frames * num_joints);

	// use controllers to sample the clips at the right intervals.
	ClipController* from_controller = new ClipController;
	ClipController* to_controller = new ClipController;
	Pose* from_pose = new Pose(skel);
	Pose* to_pose = new Pose(skel);

	from_controller->SetSkeleton(skel);
	from_controller->SetClip(from);

	to_controller->SetSkeleton(skel);
	to_controller->SetClip(to);

	const Quaternion* from_rotations = from_pose->GetRotations();
	const Quaternion* to_rotations = to_pose->GetRotations();

	Quaternion align_q = make_rotation(align_rotation, Vec3(0,1,0));
	from_controller->SetTime( from_start );
	to_controller->SetTime( to_start );
	int out_joint_offset = 0;
	for(int i = 0; i < num_frames; ++i)
	{
		from_controller->ComputePose(from_pose);
		to_controller->ComputePose(to_pose);

		float blend = compute_blend_param(i, num_frames);
		float one_minus_blend = 1.f - blend;

		// transform the target pose with the alignment
		Vec3 target_root_off = align_translation + rotate(to_pose->GetRootOffset(), align_q);
		Quaternion target_root_q = align_q * to_pose->GetRootRotation();

		root_translations[i] = blend * from_pose->GetRootOffset() + one_minus_blend * target_root_off;
		slerp_rotation(root_rotations[i], from_pose->GetRootRotation(), target_root_q, blend);
					
		for(int joint = 0; joint < num_joints; ++joint)
		{
			slerp_rotation(frame_rotations[out_joint_offset], from_rotations[joint],
				  to_rotations[joint], blend);
			++out_joint_offset;
		}

		from_controller->UpdateTime( sample_interval );
		to_controller->UpdateTime( sample_interval );
	}
	
	delete from_controller;
	delete to_controller;
	delete from_pose;
	delete to_pose;
}

sqlite3_int64 createTransitionClip(sqlite3* db, 
								   const Skeleton* skel,
								   const Clip* from, 
								   const Clip* to, 
								   float from_start, 
								   float to_start,
								   int num_frames, float sample_interval,
								   Vec3_arg align_translation, 
								   float align_rotation)
{
	std::string transition_name = "blend_from_";
	transition_name += from->GetName();
	transition_name += "_to_";
	transition_name += to->GetName();

	std::vector< Vec3 > root_translations;
	std::vector< Quaternion > root_rotations;
	std::vector< Quaternion > frame_rotations;
	const int num_joints = skel->GetNumJoints();
	blendClips(skel, from, to, from_start, to_start,
			   num_frames, sample_interval, align_translation, align_rotation,
			   root_translations, root_rotations, frame_rotations);
	ASSERT( (int)root_translations.size() == num_frames );
	ASSERT( (int)root_rotations.size() == num_frames );
	ASSERT( (int)frame_rotations.size() == num_frames * num_joints );

	SavePoint save(db, "createTransitionClip");

	Query insert_clip(db, "INSERT INTO clips(skel_id,name,fps,is_transition) "
					  "VALUES (?, ?, ?, 1)");	
	insert_clip.BindInt64(1, skel->GetID())
		.BindText(2, transition_name.c_str())
		.BindDouble(3, 1.f/sample_interval);
	insert_clip.Step();
	if(insert_clip.IsError()) {
		save.Rollback();
		return 0;
	}
	sqlite3_int64 clip_id = insert_clip.LastRowID();

	Query insert_frame(db, "INSERT INTO frames(clip_id,num,"
					   "root_offset_x, root_offset_y, root_offset_z,"
					   "root_rotation_a, root_rotation_b, root_rotation_c, root_rotation_r) "
					   "VALUES (?,?, ?,?,?, ?,?,?,?)");
	insert_frame.BindInt64(1, clip_id);
	Query insert_rots(db, "INSERT INTO frame_rotations "
					  "(frame_id, skel_id, joint_offset, q_a, q_b, q_c, q_r) "
					  "VALUES (?, ?,?, ?,?,?,?)");
	insert_rots.BindInt64(2, skel->GetID());
	int joint_index = 0;
	for(int i = 0; i < num_frames; ++i)
	{
		insert_frame.Reset();
		insert_frame.BindInt(2, i);
		insert_frame.BindVec3(3, root_translations[i]);
		insert_frame.BindQuaternion(6, root_rotations[i]);
		insert_frame.Step();

		if(insert_frame.IsError()) {
			save.Rollback();
			return 0;
		}

		sqlite3_int64 frame_id = insert_frame.LastRowID();
		insert_rots.Reset();
		insert_rots.BindInt64(1, frame_id);

		for(int joint = 0; joint < num_joints; ++joint)
		{
			insert_rots.Reset();
			insert_rots.BindInt(3, joint);
			insert_rots.BindQuaternion(4, frame_rotations[joint_index++]);
			insert_rots.Step();

			if(insert_rots.IsError()) {
				save.Rollback();
				return 0;
			}				
		}
	}

	// finally, annotate the clip with a union of the parent clip annotations 
	Query get_annos(db, "SELECT DISTINCT annotation_id FROM clip_annotations WHERE clip_id = ? OR clip_id = ?");
	get_annos.BindInt64(1, from->GetID()).BindInt64(2, to->GetID());
	Query insert_anno(db, "INSERT INTO clip_annotations(annotation_id, clip_id) VALUES (?,?)");
	while(get_annos.Step()) {
		insert_anno.Reset();
		insert_anno.BindInt64(1, get_annos.ColInt64(0)).BindInt64(2,clip_id);
		insert_anno.Step();
	}

	return clip_id;
}

bool exportMotionGraphToGraphViz(sqlite3* db, sqlite3_int64 graph_id, const char* filename )
{
	ofstream out(filename, ios_base::out|ios_base::trunc);
	if(!out) 
		return false;
	
	out << "digraph G {" << endl
		<< "\trankdir=LR;" << endl;
	
	Query get_edges(db, "SELECT motion_graph_edges.start_id, motion_graph_edges.finish_id, "
					"clips.name, clips.is_transition "
					"FROM motion_graph_edges JOIN clips ON motion_graph_edges.clip_id = clips.id "
					"WHERE motion_graph_edges.motion_graph_id = ?");
	get_edges.BindInt64(1, graph_id);

	while(get_edges.Step())
	{
		sqlite3_int64 startNode = get_edges.ColInt64(0);
		sqlite3_int64 endNode = get_edges.ColInt64(1);
		const char* name = get_edges.ColText(2);
		int is_transition = get_edges.ColInt(3);

		out << "\tn" << startNode << " -> n" << endNode ;
		if(is_transition == 1) 
			out << "[label=\"" << name << "\" style=dotted];" ;
		out << endl;
	}

	Query get_nodes(db, "SELECT motion_graph_nodes.id,motion_graph_nodes.frame_num, clips.name "
					"FROM motion_graph_nodes JOIN clips ON motion_graph_nodes.clip_id = clips.id "
					"WHERE motion_graph_nodes.motion_graph_id = ?");
	get_nodes.BindInt64(1, graph_id);

	while(get_nodes.Step()) {
		sqlite3_int64 nodeId = get_nodes.ColInt64(0);
		int frame = get_nodes.ColInt(1);
		const char* name = get_nodes.ColText(2);

		out << "\tn" << nodeId << " [label=\"" << name << " at frame " << frame << "\"];" << endl;
	}
	out << "}" << endl;
	return true;
}

////////////////////////////////////////////////////////////////////////////////
AlgorithmMotionGraph::AlgorithmMotionGraph(sqlite3* db, sqlite3_int64 id)
	: m_db(db), m_db_id(id)
{
	
}

AlgorithmMotionGraph::~AlgorithmMotionGraph()
{
	{
		const int count = m_nodes.size();
		for(int i = 0; i < count; ++i) {
			delete m_nodes[i];
		}
	}

	{
		const int count = m_edges.size();
		for(int i = 0; i < count; ++i) {
			delete m_edges[i];
		}
	}
}

AlgorithmMotionGraph::Node* AlgorithmMotionGraph::AddNode(sqlite3_int64 id)
{
	Node* newNode = new Node();
	newNode->db_id = id;
	m_nodes.push_back(newNode);
	return newNode;
}

AlgorithmMotionGraph::Edge* AlgorithmMotionGraph::AddEdge(Node* start, Node* finish, sqlite3_int64 id)
{
	Edge* newEdge = new Edge();
	m_edges.push_back(newEdge);
	newEdge->db_id = id;
	newEdge->start = start;
	newEdge->end = finish;

	start->outgoing.push_back(newEdge);
	return newEdge;
}

AlgorithmMotionGraph::Node* AlgorithmMotionGraph::FindNode(sqlite3_int64 id) const
{
	const int count = m_nodes.size();
	for(int i = 0; i < count; ++i) {
		if(m_nodes[i]->db_id == id) {
			return m_nodes[i];
		}
	}
	return 0;
}

void AlgorithmMotionGraph::ComputeStronglyConnectedComponents( SCCList & sccs, sqlite3_int64 anno )
{
	sccs.clear();

	// initialize all nodes
	const int count = m_nodes.size();
	for(int i = 0; i < count; ++i) {
		m_nodes[i]->tarjan_index = -1;
		m_nodes[i]->tarjan_lowlink = -1;
		m_nodes[i]->tarjan_in_stack = false;
	}

	std::vector<Node*> current; 
	int cur_index = 0;
	for(int i = 0; i < count; ++i) {
		if(m_nodes[i]->tarjan_index == -1)
			Tarjan(sccs, current, m_nodes[i], cur_index, anno);
	}
}

bool HasAnnotation(AlgorithmMotionGraph::Edge* edge, sqlite3_int64 anno)
{
	const int num_annos = edge->annotations.size();
	for(int j = 0; j < num_annos; ++j) {
		if( edge->annotations[j] == anno) {
			return true;
		}
	}
	return false;
}

void AlgorithmMotionGraph::Tarjan( SCCList & sccs, std::vector<Node*>& current, Node* curNode, 
								   int &index, sqlite3_int64 anno)
{
	curNode->tarjan_index = index;
	curNode->tarjan_lowlink = index;
	curNode->tarjan_in_stack = true;
	index += 1;

	current.push_back(curNode);
	const int num_neighbors = curNode->outgoing.size();

	for(int i = 0; i < num_neighbors; ++i) {
		Node* neighbor = curNode->outgoing[i]->end;

		if(curNode->outgoing[i]->keep_flag && (anno == 0 || HasAnnotation(curNode->outgoing[i], anno)))
		{
			if(neighbor->tarjan_index == -1) {
				Tarjan(sccs, current, neighbor, index, anno);
				curNode->tarjan_lowlink = Min(curNode->tarjan_lowlink, neighbor->tarjan_lowlink);
			} else if(neighbor->tarjan_in_stack) {
				curNode->tarjan_lowlink = Min(curNode->tarjan_lowlink, neighbor->tarjan_index);
			}
		}
	}

	if(curNode->tarjan_lowlink == curNode->tarjan_index)
	{
		sccs.push_back( std::vector<Node*>() );
		std::vector<Node*> &scc = sccs.back();
		
		while(!current.empty()) {
			Node* n = current.back();
			current.pop_back();
			n->tarjan_in_stack = false;

			scc.push_back(n);
			
			if(n == curNode) break;
		}
	}
}

void AlgorithmMotionGraph::InitializePruning()
{
	const int count = m_nodes.size();
	for(int i = 0; i < count; ++i) {
		m_nodes[i]->scc_set_num = -1;
	}

	const int edge_count = m_edges.size();
	for(int i = 0; i < edge_count; ++i) {
		m_edges[i]->keep_flag = true;
	}
}

bool AlgorithmMotionGraph::EdgeInSet( const Edge* edge, sqlite3_int64 anno ) const
{
	if(anno == 0) return true;

	const int num_annos = edge->annotations.size();
	for(int i = 0; i < num_annos; ++i) {
		if(edge->annotations[i] == anno) {
			return true;
		}
	}
	return false;
}

void AlgorithmMotionGraph::MarkSetNum(int set_num, sqlite3_int64 anno, std::vector<Node*> const& nodes_in_set)
{
	// first mark all nodes in the scc
	const int set_size = nodes_in_set.size();
	for(int i = 0; i < set_size; ++i) {
		nodes_in_set[i]->scc_set_num = set_num;
	}

	const int num_edges = m_edges.size();
	for(int i = 0; i < num_edges; ++i) {
		if(EdgeInSet(m_edges[i], anno)) {
			if(m_edges[i]->start->scc_set_num != set_num ||
			   m_edges[i]->end->scc_set_num != set_num) {
				m_edges[i]->keep_flag = false;
			}
		}
	}
}

bool AlgorithmMotionGraph::Commit(int *num_deleted)
{
	if(num_deleted) *num_deleted = 0;
	Transaction transaction(m_db);
	Query delete_edge(m_db, "DELETE FROM motion_graph_edges WHERE id = ?");

	const int num_edges = m_edges.size();
	for(int i = 0; i < num_edges; ++i) {
		if(!m_edges[i]->keep_flag) {
			delete_edge.Reset();
			delete_edge.BindInt64( 1, m_edges[i]->db_id );
			delete_edge.Step();

			if(delete_edge.IsError()) {
				transaction.Rollback();
				return false;
			}
		}
	}

	Query delete_orphaned_nodes(m_db, 
								"DELETE FROM motion_graph_nodes WHERE motion_graph_id = ? AND id NOT IN "
								"(SELECT start_id FROM motion_graph_edges WHERE motion_graph_id = ?) "
								"AND id NOT IN "
								"(SELECT finish_id FROM motion_graph_edges WHERE motion_graph_id = ?)");
	delete_orphaned_nodes.BindInt64(1, m_db_id).BindInt64(2, m_db_id).BindInt64(3, m_db_id);
	delete_orphaned_nodes.Step();
	
	if(delete_orphaned_nodes.IsError()) {
		transaction.Rollback();
		return false;
	}

	if(num_deleted) *num_deleted = delete_orphaned_nodes.NumChanged();
	return true;
}

AlgorithmMotionGraph::Node* AlgorithmMotionGraph::FindNodeWithAnno(sqlite3_int64 anno) const
{
	const int num_nodes = m_nodes.size();
	if(anno == 0 && num_nodes > 0) {
		return m_nodes[0];
	}
	for(int i = 0; i < num_nodes; ++i) {
		const int num_neighbors = m_nodes[i]->outgoing.size();
		for(int j = 0; j < num_neighbors; ++j) {
			if(EdgeInSet(m_nodes[i]->outgoing[j], anno)) {
				return m_nodes[i];
			}
		}
	}
	return 0;
}

bool AlgorithmMotionGraph::CanReachNodeWithAnno(Node* from, sqlite3_int64 anno) const
{
	ASSERT(from);
	if(anno == 0) { return true; }

	std::list< const Node* > search_list ;
	search_list.push_back(from);

	const int num_edges = m_edges.size();
	for(int i = 0; i < num_edges; ++i) {
		m_edges[i]->visited = false;
	}

	while(!search_list.empty()) {
		const Node* cur = search_list.back();
		search_list.pop_back();

		const int num_neighbors = cur->outgoing.size();
		for(int j = 0; j < num_neighbors; ++j) {
			if(!cur->outgoing[j]->visited) {
				cur->outgoing[j]->visited = true;
				if(EdgeInSet(cur->outgoing[j], anno)) {
					return true;
				}
				else {
					search_list.push_back( cur->outgoing[j]->end );
				}
			}
		}
	}

	return false;
}
