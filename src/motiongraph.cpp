#include <ostream>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <omp.h>
#include <cstdio>
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

// TODO: use binary trees or sort lists so finding is fast

MotionGraph::MotionGraph()
{
}

MotionGraph::~MotionGraph()
{
	const int num_edges = m_edges.size();
	for(int i = 0; i < num_edges; ++i) {
		delete m_edges[i];
	}
	const int num_nodes = m_nodes.size();
	for(int i = 0; i < num_nodes; ++i) {
		delete m_nodes[i];
	}
}

bool MotionGraph::HasEdge(const MGNode* start, const MGNode* finish, const Clip* clip) const
{
	const int num_edges = m_edges.size();
	for(int i = 0; i < num_edges; ++i)
	{
		if(m_edges[i]->GetStart() == start &&
		   m_edges[i]->GetFinish() == finish &&
		   (clip == 0 || clip == m_edges[i]->GetClip()))
			return true;
	}
	return false;
}

int MotionGraph::IndexOfEdge(const MGEdge* edge) const 
{
	const int num_edges = m_edges.size();
	for(int i = 0; i < num_edges; ++i)
	{
		if(m_edges[i] == edge)
			return i;
	}
	return -1;
}

MGEdge* MotionGraph::AddEdge( MGNode* start, MGNode* finish, const Clip* clip )
{
	ASSERT(start && finish);
	MGEdge *edge = new MGEdge(start,finish,clip);
	start->AddOutgoing(edge);
	m_edges.push_back(edge);
	return edge;
}

MGNode* MotionGraph::AddNode()
{
	MGNode* node = new MGNode();
	m_nodes.push_back(node);
	return node;
}

int MotionGraph::GetNumEdges() const 
{
	return m_edges.size();
}

int MotionGraph::GetNumNodes() const 
{
	return m_nodes.size();
}

MGEdge* MotionGraph::GetEdge(int idx)
{
	ASSERT(idx >= 0 && idx < (int)m_edges.size());
	return m_edges[idx];
}

const MGEdge* MotionGraph::GetEdge(int idx) const
{
	ASSERT(idx >= 0 && idx < (int)m_edges.size());
	return m_edges[idx];
}

MGNode* MotionGraph::GetNode(int idx) 
{
	ASSERT(idx >= 0 && idx < (int)m_nodes.size());
	return m_nodes[idx];
}

const MGNode* MotionGraph::GetNode(int idx) const
{
	ASSERT(idx >= 0 && idx < (int)m_nodes.size());
	return m_nodes[idx];
}

////////////////////////////////////////////////////////////////////////////////
void MGNode::AddOutgoing(MGEdge* edge)
{
	m_outgoing.push_back(edge);
}

bool MGNode::RemoveOutgoing(MGEdge* edge)
{
	std::list<MGEdge*>::iterator iter = m_outgoing.begin();
	while(iter != m_outgoing.end()) {
		if(*iter == edge) {
			m_outgoing.erase(iter);
			return true;
		}
		++iter;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
MGEdge::MGEdge(MGNode* start, MGNode* finish, const Clip* clip)
	: m_start(start)
	, m_finish(finish)
	, m_clip(clip)
{
}

////////////////////////////////////////////////////////////////////////////////
void populateInitialMotionGraph(MotionGraph* graph, 
								const ClipDB* clips,
								std::ostream& out)
{
	const int num_clips = clips->GetNumClips();
	for(int i = 0; i < num_clips; ++i)
	{
		const Clip* clip = clips->GetClip(i);
		MGNode* start = graph->AddNode();
		MGNode* end = graph->AddNode();
		graph->AddEdge( start, end, clip );
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

#pragma omp parallel for private(i) shared(weights,total_from_x,total_from_z,total_to_x,total_to_z,a,b)
	for(i = 0; i < total_num_samples; ++i)
	{
		float w = weights[i];
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
							 int points_per_frame,
							 int num_frames,
							 Vec3& align_translation,
							 float& align_rotation)
{
	(void)from_cloud;
	(void)to_cloud;
	(void)points_per_frame;
	(void)num_frames;
	(void)align_translation;
	(void)align_rotation;
	return 99999.f;
}
