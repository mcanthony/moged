#ifndef INCLUDED_motion_graph_controller_HH
#define INCLUDED_motion_graph_controller_HH

#include <vector>
#include "animcontroller.hh"
#include "motiongraph.hh"
#include "mgpath.hh"

class Skeleton;
class ClipController;
class BlendController;

struct SearchNode;

struct sqlite3;

// Given a path, animate a character along that path with motion graph clips
class MotionGraphController : public AnimController
{
    sqlite3 *m_db;                              // we need a handle to the db

    AlgorithmMotionGraphHandle m_algoGraph;     // A handle to the motion graph!
    std::vector< ClipHandle > m_cachedClips;    // cache to maintain handles to clips so they are loaded.

    ClipController *m_clipControllers[2];       // dual purpose. [0] is used when doing one clip, 
                                                // both are used when blending.
    BlendController *m_blendController;         // Controller when we are in transition

    Vec3 m_curOffset;                           // Running offset applied to the start of the next clip
    Quaternion m_curRotation;                   // Running rotation applied to the start of the next clip

    std::list< AlgorithmMotionGraph::Edge* > m_edgesToWalk; // Current list of edges we are walking.

    bool m_walking;                             // Is the controller currently walking, or are we idling in bind pose?

    std::vector< Vec3 > m_curSearchNodePositions;  // Positions of the nodes we are searching. Used for debug drawing.

    std::vector< Vec3 > m_curPathPositions;      // Positions of the selected path during search. Used for debug drawing.

    MGPath m_requestedPath;                     // Path requested by the user
    MGPath m_pathSoFar;                         // The actual path we've travelled so far

    AlgorithmMotionGraph::Edge *m_curEdge ;     // The current edge we are traversing.

    float m_timeToNextSample;                   // used to sample m_pathSoFar at some frequency

public:
    MotionGraphController(sqlite3* db, const Skeleton* skel);
    ~MotionGraphController();

    // AnimController interface
    void ComputePose();
    void UpdateTime(float dt);

    // MotionGraphController interface
    void SetGraph( AlgorithmMotionGraphHandle graphHandle ); 
    void ResetPaths();
    void SetRequestedPath( const MGPath& path );
    const MGPath& GetCurrentPath() const { return m_pathSoFar; }

    void DebugDraw();
private:

    void ResetAnimationControllers();           // Part of initialize - does what it says

    void AppendWalkEdges();                     // Do a search and append edges to the current list. Walk here
                                                // refers to 'edge' walk, not animation walk, though that
                                                // might be happening!

    void NextEdge(float initialTimeOffset = 0.f); // Advance to the next edge. Call when current is done.
                                                //  initialTimeOffset will start the animation a bit later, to 
                                                //  account for left over time from capped updates in the 
                                                //  Update() stuff.
    void StartEdge(float initialTimeOffset);    // set up animation controllers with the current edge
    void CreateSearchNode(SearchNode& out, 
                          AlgorithmMotionGraph::Edge* edge,
                          SearchNode* parent);
    void ComputeError(SearchNode& info);

    void InitializeGraphWalk() ;                // Set up a graph walk with an appropriate node.

};

#endif

