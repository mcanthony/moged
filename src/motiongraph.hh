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
#include "clip.hh"

class MGEdge;

class Mesh;
class Skeleton;
class ClipController;
class Pose;
class Vec3;

class MGEdge : public refcounted_type<MGEdge>
{
	sqlite3* m_db;
	sqlite3_int64 m_id;
	sqlite3_int64 m_start_id;
	sqlite3_int64 m_finish_id;
    bool m_blended;

public:
	MGEdge( sqlite3* db, sqlite3_int64 id, sqlite3_int64 start, sqlite3_int64 finish, bool blended);
	bool Valid() const { return m_id != 0 && m_start_id != 0 && m_finish_id != 0; }
	void Invalidate() { m_id = 0; } // used after splitting an edge - still want the clip, but don't want the id

	sqlite3_int64 GetID() const { return m_id; }
	sqlite3_int64 GetStartNodeID() const { return m_start_id; }
	sqlite3_int64 GetEndNodeID() const { return m_finish_id; }
    bool IsBlended() const { return m_blended; }
};
typedef reference<MGEdge> MGEdgeHandle;

struct MGNodeInfo
{
    inline MGNodeInfo() {}
    inline MGNodeInfo(sqlite3_int64 id, sqlite3_int64 clip_id, int frame_num) 
        : id(id), clip_id(clip_id), frame_num(frame_num) {}
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

class AlgorithmMotionGraph : public refcounted_type<AlgorithmMotionGraph>
{
	sqlite3* m_db;
	sqlite3_int64 m_db_id;
public:
	struct Edge;

	struct Node {
		Node() 
			: db_id(0)
			, clip_id(0)
			, frame_num(0)
			, scc_set_num(-1)
			{}
		std::vector<int> outgoing;      // outgoing EDGES
		sqlite3_int64 db_id;
		sqlite3_int64 clip_id;
		int frame_num;

		// scc set num - set to the current set num for the largest scc of the subgraph
		int scc_set_num;

		// cached clip handle for use when making walks
		ClipHandle clip;
	};

    struct TarjanNode {
        TarjanNode()
            : originalNode(-1)
            , index(-1)              
            , lowLink(-1)            
            , inStack(false)
            {}

        int originalNode;       // node this tarjan node represents
        int index;              // tarjan algorithm index of this node
        int lowLink;            // minimum index of this node and all of its neighbors
        bool inStack;           // true if the node is already in the tarjan stack
    };

	struct Edge {
		Edge() 
			: start(-1)
			, end(-1)
			, db_id(0)
            , blended(false)
            , blendTime(0.f)
			, align_rotation(0,0,0,1)
			, align_offset(0,0,0)
			{}
		int start;              // graph node to start from
		int end;                // graph node to end at
		sqlite3_int64 db_id;    // database id of this edge
		std::vector< sqlite3_int64 > annotations;   // annotations associated with this edge
        bool blended;           // true if this edge is a blend and not just a linear clip. required
                                //  because it's not clear when both nodes are on the same clip 
                                //  how to do things between them
        float blendTime;         // seconds to blend from start to end, if start and end are different clips

		// data needed to use the edge
		Quaternion align_rotation;  // rotation applied to align the 'end' clip to the 'start' clip
		Vec3 align_offset;          // offset applied to align the 'end' clip to the 'start' clip
	};
private:
	std::vector<Node*> m_nodes;
	std::vector<Edge*> m_edges;

public:
	AlgorithmMotionGraph(sqlite3* db, sqlite3_int64 id);
	~AlgorithmMotionGraph();

	sqlite3_int64 GetID() const { return m_db_id; }

	int GetNumEdges() const { return m_edges.size(); }
	int GetNumNodes() const { return m_nodes.size(); }

	int AddNode(sqlite3_int64 id, sqlite3_int64 clip_id, int frame_num);
	int AddEdge(int start, int finish, sqlite3_int64 id, bool blended, float blendTime, Vec3_arg align_offset, Quaternion_arg align_rotation);
	int FindNode(sqlite3_int64 id) const;
	
	typedef std::list< std::vector<int> > SCCList;

	void ComputeStronglyConnectedComponents( SCCList & sccs, std::vector<bool> const& keepFlags, sqlite3_int64 anno = 0 );

	void InitializePruning(std::vector<bool>& keepFlags);
	// TODO: change anno to combination of annos
	void MarkSetNum(int set_num, sqlite3_int64 anno, std::vector<int> const& nodes_in_set, std::vector<bool>& keepFlags);
  	bool Commit(int *numEdgesDeleted, int *numNodesDeleted, std::vector<bool> const& keepFlags);

	Node* FindNodeWithAnno(sqlite3_int64 anno) const;
	bool CanReachNodeWithAnno(Node* from, sqlite3_int64 anno) const;

	template<class F> void VisitNodes( F& visit ) { 
		const int num_nodes = m_nodes.size();
		for(int i = 0; i < num_nodes; ++i) {
			if(!visit(m_nodes[i])) return;
		}
	}

	template<class F> void VisitEdges( F& visit ) {
		const int num_edges = m_edges.size(); 
		for(int i = 0; i < num_edges; ++i) {
			if(!visit(m_edges[i])) return;
		}
	}

	Node* GetNodeAtIndex(int index) { return m_nodes[index]; }
    Edge* GetEdgeAtIndex(int index) { return m_edges[index]; }
	const Node* GetNodeAtIndex(int index) const { return m_nodes[index]; }
    const Edge* GetEdgeAtIndex(int index) const { return m_edges[index]; }
			
private:
	void Tarjan( std::vector<TarjanNode> &tarjanNodes, 
        SCCList & sccs, std::vector<TarjanNode*>& stack, 
        TarjanNode* node, int &index, sqlite3_int64 anno, std::vector<bool> const& keepFlags);
 	bool EdgeInSet( const Edge* edge, sqlite3_int64 anno ) const;

};

typedef reference<AlgorithmMotionGraph> AlgorithmMotionGraphHandle;

sqlite3_int64 NewMotionGraph( sqlite3* db, sqlite3_int64 skel, const char* name );
int CountMotionGraphs( sqlite3* db, sqlite3_int64 skel );

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
    mutable Query m_stmt_get_all_node_info;
	mutable Query m_stmt_delete_edge;
	mutable Query m_stmt_add_t_edge;

	void PrepareStatements();
public:
	MotionGraph(sqlite3* db, sqlite3_int64 skel_id, sqlite3_int64 id);
	~MotionGraph();

	bool Valid() const { return m_id != 0; }
	sqlite3_int64 GetID() const { return m_id; }

	const char* GetName() const { return m_name.c_str(); }

	sqlite3_int64 AddEdge(sqlite3_int64 start, sqlite3_int64 finish);
	sqlite3_int64 AddTransitionEdge(sqlite3_int64 start, sqlite3_int64 finish, float blendTime, 
									Vec3_arg align_offset, Quaternion_arg align_rot);
	sqlite3_int64 AddNode(sqlite3_int64 clip_id, int frame_num);
	sqlite3_int64 FindNode(sqlite3_int64 clip_id, int frame_num) const;

	bool GetNodeInfo(sqlite3_int64 node_id, MGNodeInfo& out) const;
    void GetNodeInfos(std::vector<MGNodeInfo>& out) const;

	// returns node subdividing an edge_id. edge is DELETED and two edges are added.
	// if left or right is not null, then they are filled witht he new edge ids on success.
	sqlite3_int64 SplitEdge(sqlite3_int64 edge_id, int frame_num, sqlite3_int64* left, sqlite3_int64* right);

	bool DeleteEdge(sqlite3_int64 id);

	void GetEdgeIDs(std::vector<sqlite3_int64>& out) const;

	// return graph used for doing graph-like algorithms. edges point to nodes and nodes have a list of outgoing edges.
	AlgorithmMotionGraphHandle GetAlgorithmGraph() const;

	MGEdgeHandle GetEdge(sqlite3_int64 id) ;
	const MGEdgeHandle GetEdge(sqlite3_int64 id) const;

	int GetNumEdges() const ;
	int GetNumNodes() const ;

	float CountClipTimeWithAnno(sqlite3_int64 anno) const;
	bool RemoveRedundantNodes(int *numNodesDeleted) const;
};

bool exportMotionGraphToGraphViz(sqlite3* db, sqlite3_int64 graph_id, const char* filename );

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

/*
sqlite3_int64 createTransitionClip(sqlite3* db, 
								   const Skeleton *skel,
								   const Clip* from, 
								   const Clip* to, 
								   float from_start,
								   float to_start,
								   int num_frames, 
								   float sample_interval, 
								   Vec3_arg align_translation, 
								   float align_rotation,
								   const char *transition_name);

*/

#endif
