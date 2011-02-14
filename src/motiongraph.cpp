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
    , m_stmt_get_all_node_info(db)
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
                            "(motion_graph_id, start_id, finish_id, align_translation, align_rotation) "
                            "VALUES (?, ?, ?, ?, ?)");
    m_stmt_insert_edge.BindInt64(1, m_id);

    m_stmt_insert_node.Init("INSERT INTO motion_graph_nodes (motion_graph_id, clip_id, frame_num) VALUES ( ?,?,? )");
    m_stmt_insert_node.BindInt64(1, m_id);

    m_stmt_find_node.Init("SELECT id FROM motion_graph_nodes WHERE motion_graph_id = ? AND clip_id = ? AND frame_num = ?");
    m_stmt_find_node.BindInt64(1, m_id);

    m_stmt_get_edges.Init("SELECT id FROM motion_graph_edges WHERE motion_graph_id = ?");
    m_stmt_get_edges.BindInt64(1, m_id);

    m_stmt_get_edge.Init("SELECT start_id,finish_id,blended FROM motion_graph_edges WHERE motion_graph_id = ? AND id = ?");
    m_stmt_get_edge.BindInt64(1, m_id);

    m_stmt_get_node.Init("SELECT id,clip_id,frame_num FROM motion_graph_nodes WHERE motion_graph_id = ? AND id = ?");
    m_stmt_get_node.BindInt64(1, m_id);

    m_stmt_get_all_node_info.Init("SELECT id,clip_id,frame_num FROM motion_graph_nodes WHERE motion_graph_id = ?");
    m_stmt_get_all_node_info.BindInt64(1, m_id);

    m_stmt_delete_edge.Init("DELETE FROM motion_graph_edges WHERE id = ? and motion_graph_id = ?");
    m_stmt_delete_edge.BindInt64(2, m_id);

    m_stmt_add_t_edge.Init("INSERT INTO motion_graph_edges(motion_graph_id, start_id, finish_id, blend_time, align_translation, align_rotation, blended )"
                           "VALUES (?, ?, ?, ?, ?, ?, 1)");
    m_stmt_add_t_edge.BindInt64(1, m_id);
}

MotionGraph::~MotionGraph()
{
}

sqlite3_int64 MotionGraph::AddEdge(sqlite3_int64 start, sqlite3_int64 finish)
{
    m_stmt_insert_edge.Reset();
    m_stmt_insert_edge.BindInt64(2, start);
    m_stmt_insert_edge.BindInt64(3, finish);
    Vec3 default_translation(0,0,0);
    Quaternion default_rotation(0,0,0,1);
    m_stmt_insert_edge.BindBlob(4, &default_translation, sizeof(default_translation));
    m_stmt_insert_edge.BindBlob(5, &default_rotation, sizeof(default_rotation));
    m_stmt_insert_edge.Step();
    return m_stmt_insert_edge.LastRowID();
}

sqlite3_int64 MotionGraph::AddTransitionEdge(sqlite3_int64 start, 
    sqlite3_int64 finish, float blendTime, 
    Vec3_arg align_offset, Quaternion_arg align_rot)
{
    m_stmt_add_t_edge.Reset();
    m_stmt_add_t_edge.BindInt64(2, start)
        .BindInt64(3, finish)
        .BindDouble(4, blendTime)
        .BindBlob(5, &align_offset, sizeof(align_offset))
        .BindBlob(6, &align_rot, sizeof(align_rot));
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

void MotionGraph::GetNodeInfos(std::vector< MGNodeInfo >& out) const
{
    m_stmt_get_all_node_info.Reset();
    while(m_stmt_get_all_node_info.Step()) {
        out.push_back( MGNodeInfo(
            m_stmt_get_all_node_info.ColInt64(0),
            m_stmt_get_all_node_info.ColInt64(1),
            m_stmt_get_all_node_info.ColInt(2)
        ));
    }
}

// Split an edge representing a contiguous block of an existing clip (NOT a transition).
//  Splits edge at frame_num, and create two new edges. Deletes the old edge and puts their ids in
//  left and right if the pointers to left and right are non null.
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

    // Validate (and assert, since calling this is a serious error) when splitting a transition edge.
    if(edgeData->IsBlended() || start_info.clip_id != end_info.clip_id) {
        ASSERT(false);
        fprintf(stderr, "WEIRD ERROR: Attempting to split what looks like a transition. Don't split transitions!\n");
        return 0;
    }

    // Are we trying to split out of range of this edge?
    if(frame_num <= start_info.frame_num || 
       frame_num >= end_info.frame_num) {
        fprintf(stderr, "SplitEdge: requested split is out of range (split at %d between %d and %d\n",
                frame_num, start_info.frame_num, end_info.frame_num);
        return 0;
    }

    // Make a new mid point, and the two edges going to this centre node from the original nodes.
    sqlite3_int64 mid_point_node = 0;
    sqlite3_int64 splitClipId = start_info.clip_id;
    {
        SavePoint save(m_db, "splitEdge");

        mid_point_node = AddNode(splitClipId, frame_num);
        if(mid_point_node == 0) { 
            fprintf(stderr, "SplitEdge: failed to add midpoint node\n");
            save.Rollback(); return 0; 
        }

        sqlite3_int64 new_edge_left = AddEdge(original_start_id, mid_point_node);
        if(new_edge_left == 0) { 
            fprintf(stderr, "SplitEdge: failed to add new left edge\n");
            save.Rollback(); return 0; 
        }
        
        sqlite3_int64 new_edge_right = AddEdge(mid_point_node, original_end_id);
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
        sqlite3_int64 start_id = m_stmt_get_edge.ColInt64(0);
        sqlite3_int64 finish_id = m_stmt_get_edge.ColInt64(1);
        bool blended = m_stmt_get_edge.ColInt(2) == 1;

        MGEdgeHandle result = new MGEdge(m_db, id, start_id, finish_id, blended);
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
        sqlite3_int64 start_id = m_stmt_get_edge.ColInt64(0);
        sqlite3_int64 finish_id = m_stmt_get_edge.ColInt64(1);
        bool blended = m_stmt_get_edge.ColInt(2) == 1;

        MGEdgeHandle result = new MGEdge(m_db, id, start_id, finish_id, blended);
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

    Query get_nodes(m_db, "SELECT id,clip_id,frame_num FROM motion_graph_nodes WHERE motion_graph_id = ?");
    get_nodes.BindInt64(1, m_id);
    while(get_nodes.Step()) {
        handle->AddNode( get_nodes.ColInt64(0), get_nodes.ColInt64(1), get_nodes.ColInt(2) );
    }

    Query get_edges(m_db, "SELECT id,start_id,finish_id, blended, blend_time, "
        "align_translation, align_rotation "
        "FROM motion_graph_edges WHERE motion_graph_id = ?");
    get_edges.BindInt64(1, m_id);
    Query get_annos(m_db, "SELECT annotation_id FROM clip_annotations WHERE clip_id = ?");

    while(get_edges.Step()) {
        AlgorithmMotionGraph::Node* start = handle->FindNode( get_edges.ColInt64(1) );
        AlgorithmMotionGraph::Node* end = handle->FindNode( get_edges.ColInt64(2) );
        ASSERT(start && end);

        bool blended = get_edges.ColInt(3) == 1;
        float blendTime = get_edges.ColDouble(4);

        AlgorithmMotionGraph::Edge* edge = handle->AddEdge(start, end, get_edges.ColInt64(0),
            blended, blendTime,
            get_edges.ColVec3FromBlob(4), get_edges.ColQuaternionFromBlob(5));
        
        get_annos.Reset();
        get_annos.BindInt64(1, start->clip_id);
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
            "SELECT sum(clips.num_frames/clips.fps) FROM clips "
            "WHERE clips.id IN "
            "(SELECT motion_graph_nodes.clip_id FROM motion_graph_nodes "
            "WHERE motion_graph_nodes.motion_graph_id = ?) "
            );
    } else {
        count_clip_time.Init(
            "SELECT sum(count(*)/clips.fps) FROM clips "
            "WHERE clips.id IN "
            "(SELECT motion_graph_nodes.clip_id FROM motion_graph_nodes "
            "LEFT JOIN clip_annotations ON "
            "motion_graph_nodes.clip_id = clip_annotations.clip_id "
            "WHERE motion_graph_nodes.motion_graph_id = ? "
            "AND clip_annotations.annotation_id = ?) "
            );
        count_clip_time.BindInt64(2, anno);
    }
    count_clip_time.BindInt64(1, m_id);

    if(count_clip_time.Step()) {
        return count_clip_time.ColDouble(0);
    }
    return 0.f;
}

// Remove nodes in a non-blended sequence that have been added by the algorithm, and then had 
// all of their non-trivial edges removed (making it effectively a frame marker in a non blended clip).
// Deletes unnecessary edges that correspond to the extra nodes.

int MotionGraph::RemoveRedundantNodes() const
{
    Transaction transaction(m_db);
    int num_deleted = 0;

    Query each_node(m_db, "SELECT id FROM motion_graph_nodes WHERE motion_graph_id = ?");
    each_node.BindInt64(1, m_id);

    // Query to get source clip of all non-blended incoming edges for this node.
    Query queryIncomingNodeClips(m_db, "SELECT motion_graph_nodes.clip_id, motion_graph_edges.id FROM "
        "motion_graph_nodes INNER JOIN motion_graph_edges ON motion_graph_nodes.id = motion_graph_edges.start_id "
        "WHERE motion_graph_edges.finish_id = ? AND motion_graph_edges.blended = 0");

    // Query to get destination clip of all non-blended outgoing edges for this node.
    Query queryOutgoingNodeClips(m_db, "SELECT motion_graph_nodes.clip_id, motion_graph_edges.id, motion_graph_nodes.id FROM "
        "motion_graph_nodes INNER JOIN motion_graph_edges ON motion_graph_nodes.id = motion_graph_edges.finish_id "
        "WHERE motion_graph_edges.start_id = ? AND motion_graph_edges.blended = 0");

    Query delete_edge(m_db, "DELETE FROM motion_graph_edges WHERE id = ? AND motion_graph_id = ?");
    delete_edge.BindInt64(2, m_id);
    Query patch_edge(m_db, "UPDATE motion_graph_edges SET finish_id = ? WHERE id = ?");

    // will update patchEdges[i] to finish at patchDests[i]
    std::vector<sqlite3_int64> patchEdges;
    std::vector<sqlite3_int64> patchDests;
    
    // will delete edges in these lists
    std::vector<sqlite3_int64> edgesToDelete;

    while(each_node.Step()) {
        sqlite3_int64 node_id = each_node.ColInt64(0);

        queryIncomingNodeClips.Reset();
        queryIncomingNodeClips.BindInt64(1, node_id);
        if(queryIncomingNodeClips.Step())
        {
            sqlite3_int64 incomingClipId = queryIncomingNodeClips.ColInt64(0);  
            sqlite3_int64 incomingEdgeId = queryIncomingNodeClips.ColInt64(1);

            // we only care about nodes that have a single incoming node and a single outgoing node. If 
            // we get another result, pass.
            if(queryIncomingNodeClips.Step())
                continue;

            queryOutgoingNodeClips.Reset();
            queryOutgoingNodeClips.BindInt64(1, node_id);

            if(queryOutgoingNodeClips.Step())
            {
                sqlite3_int64 outgoingClipId = queryOutgoingNodeClips.ColInt64(0);
                sqlite3_int64 outgoingEdgeId = queryOutgoingNodeClips.ColInt64(1);
                sqlite3_int64 destNodeId = queryOutgoingNodeClips.ColInt64(2);

                // same as above, pass if we get more than one result (meaning this is not a redundant node)
                if(queryOutgoingNodeClips.Step())
                    continue;

                if(incomingClipId == outgoingClipId)
                {
                    // update the incoming edge to point at the old edges finish
                    patchEdges.push_back(incomingEdgeId);
                    patchDests.push_back(destNodeId);

                    // queue middle node and outgoing edge for deletion
                    edgesToDelete.push_back(outgoingEdgeId);
                }
            }
        }
    }

    ASSERT( patchEdges.size() == patchDests.size() );
    const int numPatches = (int)patchEdges.size();
    for(int i = 0; i < numPatches; ++i)
    {
        patch_edge.Reset();
        patch_edge.BindInt64(1, patchDests[i]).BindInt64(2, patchEdges[i]);
        patch_edge.Step();
        if(patch_edge.IsError()) {
            transaction.Rollback();
            return 0;
        }
    }

    const int numEdgesToDelete = (int)edgesToDelete.size();
    for(int i = 0; i < numEdgesToDelete; ++i)
    {
        delete_edge.Reset();
        delete_edge.BindInt64(1, edgesToDelete[i]);
        delete_edge.Step();
        if(delete_edge.IsError()) {
            transaction.Rollback();
            return 0;
        }
    }

    // remove empty nodes
    Query delete_orphaned_nodes(m_db, 
                                "DELETE FROM motion_graph_nodes WHERE motion_graph_id = ? AND id NOT IN "
                                "(SELECT start_id FROM motion_graph_edges WHERE motion_graph_id = ?) "
                                "AND id NOT IN "
                                "(SELECT finish_id FROM motion_graph_edges WHERE motion_graph_id = ?)");
    delete_orphaned_nodes.BindInt64(1, m_id).BindInt64(2, m_id).BindInt64(3, m_id);
    delete_orphaned_nodes.Step();
    
    if(delete_orphaned_nodes.IsError()) {
        transaction.Rollback();
        return 0;
    }

    return num_deleted;
}

////////////////////////////////////////////////////////////////////////////////
MGEdge::MGEdge( sqlite3* db, sqlite3_int64 id, sqlite3_int64 start, sqlite3_int64 finish, bool blended )
    : m_db(db)
    , m_id(id)
    , m_start_id(start)
    , m_finish_id(finish)
    , m_blended(blended)
{
}

////////////////////////////////////////////////////////////////////////////////
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

bool exportMotionGraphToGraphViz(sqlite3* db, sqlite3_int64 graph_id, const char* filename )
{
    ofstream out(filename, ios_base::out|ios_base::trunc);
    if(!out) 
        return false;
    
    out << "digraph G {" << endl ;
    
    Query get_edges(db, "SELECT motion_graph_edges.start_id, motion_graph_edges.finish_id, "
                    "blended, blend_time "
                    "FROM motion_graph_edges "
                    "WHERE motion_graph_edges.motion_graph_id = ?");
    get_edges.BindInt64(1, graph_id);

    while(get_edges.Step())
    {
        sqlite3_int64 startNode = get_edges.ColInt64(0);
        sqlite3_int64 endNode = get_edges.ColInt64(1);
        bool blended = get_edges.ColInt(2) == 1;
        float blendTime = get_edges.ColDouble(3);

        out << "\tn" << startNode << " -> n" << endNode ;
        if(blended) 
            out << "[label=\"blendTime = " << blendTime << "\" style=dotted];" ;
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

AlgorithmMotionGraph::Node* AlgorithmMotionGraph::AddNode(sqlite3_int64 id, sqlite3_int64 clip_id, int frame_num)
{
    Node* newNode = new Node();
    newNode->db_id = id;
    newNode->clip_id = clip_id;
    newNode->frame_num = frame_num;
    m_nodes.push_back(newNode);
    return newNode;
}

AlgorithmMotionGraph::Edge* AlgorithmMotionGraph::AddEdge(Node* start, Node* finish, sqlite3_int64 id, bool blended, float blendTime, Vec3_arg align_offset, Quaternion_arg align_rotation)
{
    Edge* newEdge = new Edge();
    m_edges.push_back(newEdge);
    newEdge->db_id = id;
    newEdge->start = start;
    newEdge->end = finish;
    newEdge->blended = blended;
    newEdge->blendTime = blendTime;
    newEdge->align_rotation = align_rotation;
    newEdge->align_offset = align_offset;

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

    int del_count = 0;
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

            ++del_count;
        }
    }

    if(num_deleted) *num_deleted = del_count;

    Query find_unused_transitions(m_db,
                                  "SELECT clip_id FROM motion_graph_nodes WHERE motion_graph_id = ? "
                                  "AND id NOT IN "
                                  "(SELECT start_id FROM motion_graph_edges WHERE motion_graph_id = ?) "
                                  "AND id NOT IN "
                                  "(SELECT finish_id FROM motion_graph_edges WHERE motion_graph_id = ?)");
    find_unused_transitions.BindInt64(1, m_db_id).BindInt64(2, m_db_id).BindInt64(3, m_db_id);
    std::vector< sqlite3_int64 > clips_to_kill;
    while(find_unused_transitions.Step())
    {
        clips_to_kill.push_back(find_unused_transitions.ColInt64(0));
    }

    Query delete_orphans(m_db,
                         "DELETE FROM motion_graph_nodes WHERE motion_graph_id = ? AND id NOT IN "
                         "(SELECT start_id FROM motion_graph_edges WHERE motion_graph_id = ?) "
                         "AND id NOT IN "
                         "(SELECT finish_id FROM motion_graph_edges WHERE motion_graph_id = ?)");
    delete_orphans.BindInt64(1, m_db_id).BindInt64(2, m_db_id).BindInt64(3, m_db_id);
    delete_orphans.Step();
    if(delete_orphans.IsError()) {
        transaction.Rollback();
        return false;
    }

    Query delete_transition(m_db, "DELETE FROM clips WHERE is_transition = 1 AND id = ?");
    const int num_clips_to_kill = clips_to_kill.size();
    for(int i = 0; i < num_clips_to_kill; ++i) {
        delete_transition.Reset();
        delete_transition.BindInt64(1, clips_to_kill[i]);
        delete_transition.Step();
        if(delete_transition.IsError()) {
            transaction.Rollback();
            return false;
        }
    }
    

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
