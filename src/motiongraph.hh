#ifndef INCLUDED_motiongraph_HH
#define INCLUDED_motiongraph_HH

#include <ostream>
#include <list>
#include <vector>

class MGEdge;
class MGNode;
class Clip;
class ClipDB;
class Mesh;
class Skeleton;
class ClipController;
class Pose;
class Vec3;

class MGNode
{
	std::list< MGEdge* > m_outgoing;
public:
	MGNode() {}
	void AddOutgoing(MGEdge* edge);
	bool RemoveOutgoing(MGEdge* edge);
};

class MGEdge
{
	MGNode *m_start;
	MGNode *m_finish;
	const Clip* m_clip;
public:
	MGEdge(MGNode* start, MGNode* finish, const Clip* clip);
	const MGNode* GetStart() const { return m_start; }
	const MGNode* GetFinish() const { return m_finish; }
	const Clip* GetClip() const { return m_clip; }

};

class MotionGraph
{
	std::vector< MGEdge* > m_edges;
	std::vector< MGNode* > m_nodes;
public:
	MotionGraph();
	~MotionGraph();
	
	bool HasEdge(const MGNode* start, const MGNode* finish, const Clip* clip) const;
	int IndexOfEdge(const MGEdge* edge) const ;
	MGEdge* AddEdge(MGNode* start, MGNode* finish, const Clip* clip);
	MGNode* AddNode();

	MGEdge* GetEdge(int idx) ;
	const MGEdge* GetEdge(int idx) const;

	MGNode* GetNode(int idx) ;
	const MGNode* GetNode(int idx) const;
	
	
	int GetNumEdges() const ;
	int GetNumNodes() const ;
};


void populateInitialMotionGraph(MotionGraph* graph, 
								const ClipDB* clips, std::ostream& out);

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
#endif
