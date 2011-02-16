#include <cstdio>
#include <GL/gl.h>
#include <set>
#include <cstdlib>
#include "mgcontroller.hh"
#include "skeleton.hh"
#include "clipcontroller.hh"
#include "blendcontroller.hh"
#include "pose.hh"
#include "math_util.hh"

// how often to sample m_pathSoFar
static const float kSamplePeriod = 1/30.f;

////////////////////////////////////////////////////////////////////////////////
// utility functions
struct ClipDiscoverNodes { 
    ClipDiscoverNodes( std::set<sqlite3_int64> *the_set ) : cur_set(the_set) {}
    std::set<sqlite3_int64> *cur_set;
    
    bool operator()(const AlgorithmMotionGraph::Node* node) {
        cur_set->insert(node->clip_id);
        return true;
    }
};

int FindClipIn(const std::vector< ClipHandle >& clips, sqlite3_int64 id) {
    const int num_clips = clips.size();
    for(int i = 0; i < num_clips; ++i) {
        if(clips[i]->GetID() == id) {
            return i;
        }
    }
    return -1;
}

struct ClipDistributeNodes {
    ClipDistributeNodes( const std::vector< ClipHandle >* the_clips) : clips(the_clips) {}
    const std::vector< ClipHandle >* clips;

    bool operator()(AlgorithmMotionGraph::Node* node) {
        int idx = FindClipIn(*clips, node->clip_id);
        if(idx != -1) node->clip = clips->at( idx );
        return true;
    }
};

////////////////////////////////////////////////////////////////////////////////
// MotionGraphController
MotionGraphController::MotionGraphController(sqlite3* db, const Skeleton* skel)
    : AnimController(skel)
    , m_db(db)
    , m_blendController(0)
    , m_curOffset(0,0,0)
    , m_curRotation(0,0,0,1)
    , m_walking(false)
    , m_requestedPath(2000)
    , m_pathSoFar(8000)
    , m_curEdge(0)
    , m_timeToNextSample( kSamplePeriod )
{
    m_clipControllers[0] = new ClipController(skel);
    m_clipControllers[1] = new ClipController(skel);
    m_blendController = new BlendController(skel);
    m_blendController->SetFrom( m_clipControllers[0] );
    m_blendController->SetTo( m_clipControllers[1] );
}

MotionGraphController::~MotionGraphController()
{
    delete m_blendController;
    delete m_clipControllers[0];
    delete m_clipControllers[1];
}

void MotionGraphController::ComputePose()
{    
    if(!m_walking || m_curEdge == 0) {
        m_pose->RestPose( m_skel );
        return;
    }

    if(m_curEdge->blended) {
        m_blendController->ComputePose();
        m_pose->Copy(m_blendController->GetPose());
    } else {
        m_clipControllers[0]->ComputePose();
        m_pose->Copy(m_clipControllers[0]->GetPose());
    }

    // Apply alignment transform to the pose. It uses the CURRENT alignment and not the stuff in the edge,
    // because the stuff in the edge is already applied via blending. It will get rolled into the current
    // transform when the edge has been walked.
    Vec3 poseOffset = m_pose->GetRootOffset();
    Quaternion poseRot = m_pose->GetRootRotation();
    
    poseOffset = m_curOffset + rotate(poseOffset, m_curRotation);
    poseRot = normalize(m_curRotation * poseRot);

    m_pose->SetRootOffset(poseOffset);
    m_pose->SetRootRotation(poseRot);

    if(m_timeToNextSample <= 0.f) {
        // Record this position in our path so far.
        m_pathSoFar.AddPoint( vec_xz(poseOffset) );
        while(m_timeToNextSample <= 0.f) 
            m_timeToNextSample += kSamplePeriod;
    }
}

void MotionGraphController::ResetAnimationControllers()
{
    m_clipControllers[0]->SetClip(0);
    m_clipControllers[1]->SetClip(0);
}

void MotionGraphController::SetGraph( AlgorithmMotionGraphHandle handle )
{
    // Reset current controllers

    ResetAnimationControllers();

    m_algoGraph = handle;
    if(!handle) 
        return;

    // collect all clip ids from the nodes in the motion graph so we can load them
    std::set< sqlite3_int64 > unique_clips;
    ClipDiscoverNodes discoverNodes( &unique_clips );
    handle->VisitNodes(discoverNodes);

    // Make sure all of the clips in the graph are loaded by creating and
    // storing a handle to each clip.
    std::set< sqlite3_int64 >::iterator 
        iter = unique_clips.begin(),
        end = unique_clips.end();

    while(iter != end) {
        sqlite3_int64 clip_id = *iter;
        ClipHandle clip = new Clip(m_db, clip_id);
        if(clip->Valid()) 
            m_cachedClips.push_back(clip);
        ++iter;
    }

    // distribute handles back into graph nodes (this is super lame)
    // TODO: make this less super lame
    ClipDistributeNodes distributeNodes( &m_cachedClips );
    handle->VisitNodes( distributeNodes );
}

// Advance the state to the next edge in our list of edges to follow.
// call this when hitting the end of node.
void MotionGraphController::NextEdge(float initialTimeOffset)
{
    if(m_edgesToWalk.empty())
        AppendWalkEdges();
    
    // If we didn't compute any new edges, this function is called in error or we have bad data
    if(m_edgesToWalk.empty())
    {
        m_curEdge = 0;
        return;
    }

    m_curEdge = m_edgesToWalk.front();
    m_edgesToWalk.pop_front();

    // Setup the controller for the new edge.
    if(m_curEdge->blended)
    {
        // Blend from startclip/startframe...
        m_clipControllers[0]->SetClip( m_algoGraph->GetNodeAtIndex(m_curEdge->start)->clip.RawPtr());
        m_clipControllers[0]->SetTime( m_algoGraph->GetNodeAtIndex(m_curEdge->start)->frame_num / m_algoGraph->GetNodeAtIndex(m_curEdge->start)->clip->GetClipFPS() );

        // and finish the blend at endclip/endframe
        m_clipControllers[1]->SetClip(m_algoGraph->GetNodeAtIndex(m_curEdge->end)->clip.RawPtr());
        m_clipControllers[1]->SetTime( m_algoGraph->GetNodeAtIndex(m_curEdge->end)->frame_num / m_algoGraph->GetNodeAtIndex(m_curEdge->end)->clip->GetClipFPS() - 
            m_curEdge->blendTime); 
        m_clipControllers[1]->SetOffset( m_curEdge->align_offset );
        m_clipControllers[1]->SetRotation( m_curEdge->align_rotation );

        m_blendController->SetCurrentTime(0.f);
        m_blendController->SetBlendLength(m_curEdge->blendTime);
        m_blendController->UpdateTime(initialTimeOffset);
    }
    else // no blend needed, just playing a clip
    {
        m_clipControllers[0]->SetClip(m_algoGraph->GetNodeAtIndex(m_curEdge->start)->clip.RawPtr());
        m_clipControllers[0]->SetTime(0.f);

        float startFrame = (float)m_algoGraph->GetNodeAtIndex(m_curEdge->start)->frame_num;
        float endFrame = (float)m_algoGraph->GetNodeAtIndex(m_curEdge->end)->frame_num - 1.f;

        m_clipControllers[0]->SetPartialRange(startFrame, endFrame);
        m_clipControllers[0]->UpdateTime(initialTimeOffset);
    }
}

void MotionGraphController::ResetPaths()
{
    m_requestedPath.Clear();
    m_pathSoFar.Clear();
    
    m_walking = false;
    m_edgesToWalk.clear();
}

void MotionGraphController::SetRequestedPath( const MGPath& path )
{
    ResetPaths();

    m_requestedPath = path;
    m_pathSoFar.SetMaxSize( 4*path.GetMaxSize() ); // The max size multiplier is arbitrary.

    if(!m_algoGraph || m_algoGraph->GetNumNodes() == 0 ||
       m_algoGraph->GetNumEdges() == 0) return;

    InitializeGraphWalk();   
    if(!m_algoGraph.Null() && m_algoGraph->GetNumEdges() > 0)
    {
        // TODO: perhaps this isn't the right approach. Maybe it is completely valid for the entire graph
        // to be made of transitions?
        // TODO: restrict this to set of edges with a particular annotation, if desired.
        // Find a clip to start searching from. This is required to call AppendWalkEdges.
        // Find first non-blend edge
        const int numEdges = m_algoGraph->GetNumEdges();
        m_curEdge = 0;
        for(int i = 0; i < numEdges; ++i) {
            AlgorithmMotionGraph::Edge* edge = m_algoGraph->GetEdgeAtIndex(i);
            if(!edge->blended) {
                m_curEdge = edge;
                break;
            }
        }

        if(m_curEdge == 0) {
            fprintf(stderr, "No non-blended edges found, cannot start motion graph walk!\n");
        }
        else
        {
            // Align the first animation with the requested path.

            // TODO: arbitrary length along path for first direction? kinda silly. Should use dir corresponding
            // to length of the first clip.
            Vec3 requestedDir = vec_xz(m_requestedPath.PointAtLength(1.0f) - m_requestedPath.Front());

            float len = magnitude(requestedDir);
            if(len > Math::kEpsilon) {
                requestedDir /= len;
            } else requestedDir.set(1,0,0);

            Vec3 animStart = vec_xz(GetAnimStart( m_algoGraph->GetNodeAtIndex(m_curEdge->start)->clip, m_algoGraph->GetNodeAtIndex(m_curEdge->start)->frame_num ));
            Vec3 animDir = vec_xz(GetAnimDir( m_algoGraph->GetNodeAtIndex(m_curEdge->start)->clip, 
                m_algoGraph->GetNodeAtIndex(m_curEdge->start)->frame_num, 
                m_algoGraph->GetNodeAtIndex(m_curEdge->end)->frame_num ));

            m_curRotation = Math::align_rotation(vec_xz(animDir), vec_xz(requestedDir));
            m_curOffset = vec_xz(m_requestedPath.Front() - animStart);

            AppendWalkEdges();
            NextEdge();
        }
    }
    
    // If we NextEdge nullified the current edge, then we have nothing good to walk on.
    if(m_curEdge == 0) {
        m_walking = false;
        fprintf(stderr, "Error: no initial edge to walk\n");
        return;
    } else {
        m_walking = true;
    }
}

void MotionGraphController::InitializeGraphWalk()
{
    m_curEdge = 0;
    m_curOffset.set(0,0,0);
    m_curRotation.set(0,0,0,1);
    m_edgesToWalk.clear();
}

// Walk the graph, calling NextEdge as necessary.
void MotionGraphController::UpdateTime(float dt)
{
    if(m_walking && m_curEdge)
    {
        float curTime = 0.f;
        float nextCurTime = 0.f;
        bool isComplete = false;

        // which controller are we updating?
        if(m_curEdge->blended)
        {
            curTime = m_blendController->GetCurrentTime();
            m_blendController->UpdateTime(dt); 
            isComplete = m_blendController->IsComplete();
            nextCurTime = m_blendController->GetCurrentTime();
        }
        else
        {
            curTime = m_clipControllers[0]->GetTime();
            m_clipControllers[0]->UpdateTime(dt);
            isComplete = m_clipControllers[0]->IsAtEnd();
            nextCurTime = m_clipControllers[0]->GetTime();
        }

        if(isComplete)
        {
            // Roll the current edge's alignment transform into the running alignment transform
            m_curOffset = m_curOffset + rotate(m_curEdge->align_offset, m_curRotation);
            m_curRotation = normalize(m_curRotation * m_curEdge->align_rotation);

            // the time may be capped by the end of the blend
            float leftOverTime = (curTime + dt) - nextCurTime;
            NextEdge(leftOverTime);
        }

        // NextEdge may have broken things, if so, stop walking.
        if(m_curEdge == 0)
        { 
            m_walking = false;
            return;
        }

        m_timeToNextSample -= kSamplePeriod;
        
        // Stop walking if we've run out of path, or there's no next edge
        if( m_curEdge == 0 ||
            m_pathSoFar.TotalLength() >= m_requestedPath.TotalLength() ||
            m_pathSoFar.Full() )
        {
            m_walking = false;
        }       
    }
    else  
    {
        m_clipControllers[0]->GetPose()->RestPose(m_clipControllers[0]->GetSkeleton());
        m_clipControllers[1]->GetPose()->RestPose(m_clipControllers[1]->GetSkeleton());
    }
}


////////////////////////////////////////////////////////////////////////////////
// Utility stuff for searching

struct SearchNode {
    SearchNode() 
        : edge(0)
        , parent(0)
        , error(0.f)
        , time(0.f)
        , arclength(0.f)
        , align_offset(0,0,0)
        , align_rotation(0,0,0,1)
        , start_pos(0,0,0)
        , end_pos(0,0,0)
        {}
    AlgorithmMotionGraph::Edge* edge;       // The edge representing the animation for this node.
    
    SearchNode* parent;                     // The animation/edge that came before this node.
    float error;                            // cumulative error from the requested path.
    float time;                             // cumulative time of animations
    float arclength;                        // cumulative distance travelled of clips to this point.

    Vec3 align_offset;                      // offset to align dest clip with src clip
    Quaternion align_rotation;              // rotation to align dest clip with src clip

    Vec3 start_pos;                         // the aligned position this animation starts at
    Vec3 end_pos;                           // the aligned position this animation ends at
};

// TODO: estimated arc lengths should be cached for each edge in the motion graph.
static const float kArcLengthSamplePeriod = 1/30.f;
float EstimateBlendedArcLength(const Clip* startClip, float startTime,
    const Clip* endClip, float endTime, 
    Vec3_arg alignOffset, Quaternion_arg alignRotation,
    float blendTime)
{
    float result = 0.f;

    Vec3 lastPt = startClip->GetFrameRootOffset(GetFrameFromTime(startClip, startTime));
    Vec3 curPt;

    float curTime = 0.f;
    do
    {
        curTime += kArcLengthSamplePeriod;
        float param = ComputeCubicBlendParam(curTime / blendTime);

        Vec3 curPtStart = startClip->GetFrameRootOffset(GetFrameFromTime(startClip, startTime + curTime));
        Vec3 curPtEnd = endClip->GetFrameRootOffset(GetFrameFromTime(endClip, endTime - blendTime + curTime));
        curPtEnd = alignOffset + rotate(curPtEnd, alignRotation);
        
        Lerp(curPt, curPtStart, curPtEnd, param);

        result += magnitude(curPt - lastPt);
        lastPt = curPt;
    } while(curTime <= blendTime);

    return result;
}

float EstimateArcLength(const Clip* clip, float startTime, float endTime)
{
    float result = 0.f;

    Vec3 lastPt = clip->GetFrameRootOffset(GetFrameFromTime(clip, startTime));
    do
    {
        startTime += kArcLengthSamplePeriod;
        Vec3 curPt = clip->GetFrameRootOffset(GetFrameFromTime(clip, startTime));
        result += magnitude(curPt - lastPt);
        lastPt = curPt;
    } while(startTime <= endTime);

    return result;
}

void ComputePosition( const AlgorithmMotionGraphHandle& graph, 
    SearchNode& info, Vec3_arg parentOffset, Quaternion_arg parentRotation,
    float startTime, float endTime)
{
    const Clip* startClip = graph->GetNodeAtIndex(info.edge->start)->clip.RawPtr();
    const Clip* endClip = graph->GetNodeAtIndex(info.edge->end)->clip.RawPtr();
    
    // The assumption here is that alignment is identity if this is not a blend. So, if this is not a blend,
    // start pos/rot AND end pos/rot will be as if the clip had been played from the parent.
    // if it IS a blend, then start/pos rot is aligned with the parent, and the end pos/rot will have the
    // alignment transform applied to it so that subsequent animations are aligned.
    int startFrame = GetFrameFromTime(startClip,startTime);
    int endFrame = GetFrameFromTime(endClip, endTime);
    info.start_pos = vec_xz(parentOffset + rotate(startClip->GetFrameRootOffset(startFrame), parentRotation));
    info.end_pos = vec_xz(info.align_offset + rotate(endClip->GetFrameRootOffset(endFrame), info.align_rotation));
}

// Error is the squared difference of the character position after playing tihs clip with the requested
// path position.
float MotionGraphController::ComputeError(const SearchNode& info)
{
    Vec3 requested_point = m_requestedPath.PointAtLength(info.arclength);
    return magnitude_squared( info.end_pos - requested_point );
}

// Fill in a search node structure
void MotionGraphController::CreateSearchNode(SearchNode& out, 
                                        AlgorithmMotionGraph::Edge* edge,
                                        SearchNode* parent)
{
    out.edge = edge;
    out.parent = parent;

    float startTime = m_algoGraph->GetNodeAtIndex(edge->start)->frame_num / m_algoGraph->GetNodeAtIndex(edge->start)->clip->GetClipFPS();
    float endTime = m_algoGraph->GetNodeAtIndex(edge->end)->frame_num / m_algoGraph->GetNodeAtIndex(edge->end)->clip->GetClipFPS();

    // arcLength and time are computed differently if we are blending
    float arcLength = 0.f;
    float edgeTime = 0.f;    
    if(edge->blended)
    {
        edgeTime = edge->blendTime;
        arcLength = EstimateBlendedArcLength(
            m_algoGraph->GetNodeAtIndex(edge->start)->clip.RawPtr(), startTime, 
            m_algoGraph->GetNodeAtIndex(edge->end)->clip.RawPtr(), endTime, edge->align_offset, edge->align_rotation,
            edge->blendTime);
    }
    else
    {
        edgeTime = endTime - startTime;
        arcLength = EstimateArcLength(m_algoGraph->GetNodeAtIndex(edge->start)->clip.RawPtr(), startTime, endTime);
    }

    if(parent)
    {
        // edge's align_offset and align_rotation will be identity if this is not a blend.
        out.align_offset = parent->align_offset + rotate( edge->align_offset, parent->align_rotation ) ;
        out.align_rotation = normalize(parent->align_rotation * edge->align_rotation);
        out.time = parent->time + edgeTime;
        out.arclength = parent->arclength + arcLength;
        ComputePosition(m_algoGraph, out, parent->align_offset, parent->align_rotation, startTime, endTime);
        out.error = parent->error + ComputeError(out);
    }
    else
    {
        out.align_offset = m_curOffset + rotate( edge->align_offset, m_curRotation);
        out.align_rotation = normalize(m_curRotation * edge->align_rotation);
        out.time = edgeTime;
        out.arclength = m_pathSoFar.TotalLength() + arcLength;
        ComputePosition(m_algoGraph, out, m_curOffset, m_curRotation, startTime, endTime);
        out.error = ComputeError(out);
    }
}

struct compare_search_nodes 
{
    bool operator()( const SearchNode* left, const SearchNode* right)
        {
            return left->error < right->error;
        }
};

////////////////////////////////////////////////////////////////////////////////
// The actual search

void MotionGraphController::AppendWalkEdges()
{
    static const float kSearchTimeDepth = 4.f;              // How many seconds of animation to search
    static const float kFrameTimeToRetain = 1.f;            // how many seconds of animation to retain from the search
    SearchNode* bestWalkRoot = 0;

    m_curSearchNodePositions.clear();

    std::list< SearchNode > searchNodes ; // sort of acting as an 'allocator' in this case. // TODO: replace with FixedAlloc
    std::set< SearchNode*, compare_search_nodes > openList ;

    // We need a current edge, so if we don't have one, pick a random edge.

    // add the possible next edges to the search list.
    const int lastNumNeighbors = m_algoGraph->GetNodeAtIndex(m_curEdge->end)->outgoing.size();
    for(int i = 0; i < lastNumNeighbors; ++i) {
        AlgorithmMotionGraph::Edge* cur = m_algoGraph->GetEdgeAtIndex(
            m_algoGraph->GetNodeAtIndex(m_curEdge->end)->outgoing[i]);

        searchNodes.push_back( SearchNode() ); // TODO: replace with FixedAlloc
        SearchNode &info = searchNodes.back();

        CreateSearchNode(info, cur, 0);
        openList.insert(&info);
    }

    int search_count = 0;
    while(!openList.empty())
    {
        SearchNode* info = *openList.begin();
        openList.erase( openList.begin() );
        
        m_curSearchNodePositions.push_back(info->start_pos);
        m_curSearchNodePositions.push_back(info->end_pos);
        ++search_count;

        // has the cumulative time exceeded the amount of time we plan on searching?
        if(info->time >= kSearchTimeDepth) {
            // Update the current best, if it is one!
            if( (bestWalkRoot == 0 || info->error < bestWalkRoot->error) ) {
                bestWalkRoot = info;
            }
        } else { // otherwise search more!
            const int num_neighbors = m_algoGraph->GetNodeAtIndex(info->edge->end)->outgoing.size();
            for(int i = 0; i < num_neighbors; ++i) {
                searchNodes.push_back( SearchNode() ); // TODO: replace with FixedAlloc
                SearchNode& sn = searchNodes.back();
                CreateSearchNode(sn, m_algoGraph->GetEdgeAtIndex(
                    m_algoGraph->GetNodeAtIndex(info->edge->end)->outgoing[i]), info);

                ASSERT(sn.time > info->time); // The search node must add time to its parent node.
                    
                // if we already have a better path, there's no point in considering this one if it's already worse
                if(bestWalkRoot == 0 || sn.error < bestWalkRoot->error) {
                    openList.insert(&sn);
                } else {
                    searchNodes.pop_back(); // TODO: replaced with fixedalloc (delete)
                }
            }
        }
    }

    // We've searched deep enough in our graph, so append the edges to our list of edges to walk.
    if(bestWalkRoot) {
        int count = 0;
        SearchNode* cur = bestWalkRoot ;
        while(cur) {
            ++count;
            cur = cur->parent;
        }

        std::vector< SearchNode* > bestEdges;
        bestEdges.resize(count, 0);

        // Walk backwards along the path to put bestEdges in the correct order.
        m_curPathPositions.clear();
        int idx = count - 1;
        cur = bestWalkRoot;
        while(cur) {
            bestEdges[idx] = cur;
            m_curPathPositions.push_back(cur->start_pos);
            m_curPathPositions.push_back(cur->end_pos);
            --idx;
            cur = cur->parent;
        }

        // Retain a portion of the result
        idx = 0;
        while(idx < count)
        {
            m_edgesToWalk.push_back( bestEdges[idx]->edge );
            if(bestEdges[idx]->time >= kFrameTimeToRetain)
                break;
            ++idx;
        }
    } else {
        fprintf(stderr, "Could not find a graph walk long enough to satisfy search.\n");
        m_walking = false;
    }
}

void MotionGraphController::DebugDraw()
{
    const int s1 = m_curSearchNodePositions.size() & ~1;
    glColor4f(0.8,0.8,0.8,0.3);
    glPushMatrix();
    glTranslatef(0,0.1f,0);
    glBegin(GL_LINES);
    for(int i = 0; i < s1; i += 2) {
        glVertex3fv( &m_curSearchNodePositions[i].x );
        glVertex3fv( &m_curSearchNodePositions[i+1].x );
    }
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0,0.01f,0);
    const int s2 = m_curPathPositions.size() & ~1;
    glBegin(GL_LINES);
    glColor4f(0.8,0,0,0.8);
    for(int i = 0; i < s2; i += 2) {
        glVertex3fv( &m_curPathPositions[i].x );
        glVertex3fv( &m_curPathPositions[i+1].x );
    }
    glEnd();
    glPopMatrix();

    // draw the current actual mgpath to the requested path along the same length, to give
    // an idea of how much error there is.

    glBegin(GL_LINES);
    glColor3f(1,1,1);
    glVertex3fv( &m_pathSoFar.Back().x );
    glVertex3fv( &m_requestedPath.PointAtLength(m_pathSoFar.TotalLength()).x );
    glEnd();

    // debug axis
    glLineWidth(2.f);
    glBegin(GL_LINES);
    glColor3f(1,0,0);
    glVertex3f(0,0,0);
    glVertex3f(1,0,0);

    glColor3f(0,1,0);
    glVertex3f(0,0,0);
    glVertex3f(0,1,0);

    glColor3f(0,0,1);
    glVertex3f(0,0,0);
    glVertex3f(0,0,1);
    glEnd();
    glLineWidth(1.f);

}

