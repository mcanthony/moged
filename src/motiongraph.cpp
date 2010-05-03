#include <ostream>
#include <cstdlib>
#include <algorithm>
#include <omp.h>
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

void selectMotionGraphSampleVerts(const Mesh* mesh, int num, std::vector<int> &out)
{
	const int num_verts = mesh->GetNumVerts();
	out.clear();
	for(int i = 0; i < num; ++i) 
	{
		// rand index over vertex buffer. 
		// This may not give the best distribution over the character necessarily since
		// it doesn't take into account density of mesh (where we probably want number of points
		// in an area to be roughly the same).
		// can try a stratified sampling approach if this is bad.
		int rand_index = rand() % num_verts;
		out.push_back(rand_index);		
	}

	// sort the list so we at least access data in a predictable way
	std::sort( out.begin(), out.end());
}

// Get point from a clip controller. 
// 'samples' must be of at least num_samples * sample_indicies.size() in length.
// It begins by starting at a particular frame, and then sampling every sample_interval for num_samples iterations.
// This allows for clips that are running at variable FPS to be used.
// Pose must be initialized with the same skeleton as the clip_controller.

static void poseSamples(Vec3* out, int num_samples, const std::vector<int>& sample_indices, const Mesh* mesh, const Pose* pose)
{
	const float *vert_data = mesh->GetPositionPtr();
	const char *mat_indices = mesh->GetSkinMatricesPtr();
	const float *weights = mesh->GetSkinWeightsPtr();
	
	const Mat4* mats = pose->GetMatricesPtr();

	int i = 0;
	for(i = 0; i < num_samples; ++i) {
		int sample = sample_indices[i];
		int vert_sample = sample*3;
		int mat_sample = sample*4;
		Vec3 mesh_vert(&vert_data[vert_sample]);
	
		Vec3 v0 = transform_point( mats[ int(mat_indices[mat_sample    ]) ], mesh_vert );
		Vec3 v1 = transform_point( mats[ int(mat_indices[mat_sample +1 ]) ], mesh_vert );
		Vec3 v2 = transform_point( mats[ int(mat_indices[mat_sample +2 ]) ], mesh_vert );
		Vec3 v3 = transform_point( mats[ int(mat_indices[mat_sample +3 ]) ], mesh_vert );

		const float *w = weights+mat_sample;
		out[i] = w[0] * v0 + w[1] * v1 + w[2] * v2 + w[3] * v3;
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
	// init
	const int frame_length_in_samples = sample_indices.size();
	Pose** poses = new Pose*[numThreads];
	for(i = 0; i < numThreads; ++i) {
		poses[i] = new Pose(skel);
	}
	ClipController *controllers = new ClipController[numThreads];
	for(int i = 0; i < numThreads; ++i) {
		controllers[i].SetClip(clip);
	}

	omp_set_num_threads(numThreads);

	// processing
#pragma omp parallel \
	shared(poses,controllers,mesh,skel,sample_indices,samples,sample_interval,num_samples)	\
	private(i)
#pragma omp for
	for(i = 0; i < num_samples; ++i)
	{
		int tid = omp_get_thread_num();
		int frame_offset = i * frame_length_in_samples;

		controllers[tid].ComputePose(poses[tid]);
		poses[tid]->ComputeMatrices(skel, mesh->GetTransform());

		poseSamples(&samples[frame_offset],
					frame_length_in_samples,
					sample_indices, 
					mesh, 
					poses[tid]);

		controllers[tid].SetTime( i * sample_interval );
	}

	// clean up
	for(int i = 0; i < numThreads; ++i) {
		delete poses[i];
	}
	delete[] poses;
	delete[] controllers;
}
