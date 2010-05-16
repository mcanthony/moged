#include <cstdio>
#include <GL/gl.h>
#include <set>
#include <cstdlib>
#include "mgstate.hh"
#include "clipcontroller.hh"
#include "pose.hh"

using namespace std;

MGPath::MGPath(int maxSize)
	: m_max_size(maxSize)
	, m_len(0.f)
{
	m_path_points.reserve(maxSize);
}

void MGPath::SetMaxSize(int max_size) 
{
	if(max_size < (int)m_path_points.size()) {
		m_path_points.resize(max_size);
	} else {
		m_path_points.reserve(max_size);
	}
	m_max_size = max_size;
}

float MGPath::TotalLength() const 
{
	return m_len;
}

Vec3 MGPath::PointAtLength(float length)
{
	if(m_path_points.empty()) return Vec3(0,0,0);
	float lenLeft = length;
	const int num_points = m_path_points.size();
	for(int i = 1; i < num_points; ++i)
	{
		Vec3 seg_vec = m_path_points[i] - m_path_points[i-1];
		float len = magnitude(seg_vec);
		if(lenLeft > len) {
			lenLeft -= len;
		} else {
			if(lenLeft == 0.f) return m_path_points[i-1];
			float param = lenLeft / len;
			return m_path_points[i-1] + param * seg_vec;
		}
	}
	return m_path_points[ m_path_points.size() - 1];
}

bool MGPath::AddPoint( Vec3_arg newPoint )
{
	if((int)m_path_points.size() >= m_max_size) return false;
	m_path_points.push_back(newPoint);
	if(m_path_points.size() >= 2) {
		m_len += magnitude( m_path_points[ m_path_points.size() - 1] -
							m_path_points[ m_path_points.size() - 2] );
	}
	return true;
}

void MGPath::SmoothPath()
{
	// 5 pt gaussian
	static const float kSmoothFilter[] = {0.0674508058663448, 0.183350299901404, 
										  0.498397788464502, 0.183350299901404,
										  0.0674508058663448};
	static const int kSmoothLen = sizeof(kSmoothFilter)/sizeof(float);

	const int num_points = m_path_points.size();
	std::vector< Vec3 > result_points( num_points );
	for(int i = 0; i < num_points; ++i) {
		Vec3 smoothed(0,0,0);
		for(int j = 0; j < kSmoothLen; ++j) {
			int idx = Clamp( i - kSmoothLen/2 + j, 0, num_points-1 );
			smoothed += kSmoothFilter[j] * m_path_points[idx] ;
		}
		result_points[i] = smoothed;
	}
	m_path_points = result_points;
	
	float len = 0.f;
	for(int i = 1; i < num_points; ++i) {
		len += magnitude( m_path_points[i] - m_path_points[i-1] );
	}
	m_len = len;
}


void MGPath::Clear()
{
	m_path_points.clear();
	m_len = 0.f;
}

void MGPath::Draw() const
{
	glBegin(GL_LINE_STRIP);
	const int num_points = m_path_points.size();
	for(int i = 0; i < num_points; ++i) {
		glVertex3fv(&m_path_points[i].x);
	}
	glEnd();
}

////////////////////////////////////////////////////////////////////////////////
MotionGraphState::MotionGraphState()
	: m_clip_controller(0)
	, m_requested_path(2000)
	, m_path_so_far(8000)
	, m_last_node(0)
	, m_cur_edge(0)
	, m_walking(false)
{
	
}

MotionGraphState::~MotionGraphState()
{
	delete m_clip_controller;
}

void MotionGraphState::ResetPaths()
{
	m_requested_path.Clear();
	m_path_so_far.Clear();
	
	m_walking = false;
	m_edges_to_walk.clear();
}

Vec3 GetAnimDir( const AlgorithmMotionGraph::Edge* edge )
{
	const Clip* clip = edge->clip.RawPtr();
	int startFrame = 0, 
		lastFrame = clip->GetNumFrames()- 1;
	if(edge->start->clip_id == edge->end->clip_id && edge->start->clip_id == edge->clip_id) {
		startFrame = edge->start->frame_num;
		lastFrame = edge->end->frame_num;
	}
	return normalize(clip->GetFrameRootOffset(lastFrame) - clip->GetFrameRootOffset(startFrame));
}

Vec3 GetAnimStart(const AlgorithmMotionGraph::Edge* edge)
{
	const Clip* clip = edge->clip.RawPtr();
	int startFrame = 0;
	if(edge->start->clip_id == edge->end->clip_id && edge->start->clip_id == edge->clip_id) {
		startFrame = edge->start->frame_num;
	}
	return clip->GetFrameRootOffset(startFrame);
}

void MotionGraphState::SetRequestedPath( const MGPath& path )
{
	ResetPaths();

	m_requested_path = path;
	m_path_so_far.SetMaxSize( 4*path.GetMaxSize() );

	if(!m_algo_graph || m_algo_graph->GetNumNodes() == 0 ||
	   m_algo_graph->GetNumEdges() == 0) return;

	int rand_index = rand() % m_algo_graph->GetNumNodes();
	m_last_node = m_algo_graph->GetNodeAtIndex(rand_index);
	m_cur_edge = 0;
	m_cur_offset.set(0,0,0);
	m_cur_rotation.set(0,0,0,1);

	m_edges_to_walk.clear();
	FindBestGraphWalk();
	if(m_edges_to_walk.empty()) {
		m_walking = false;
		fprintf(stderr, "Error: no edges to walk\n");
		return;
	} else {
		NextEdge();
		Vec3 start_dir = m_requested_path.PointAtLength(1.0f) - m_requested_path.Front();
		float len = magnitude(start_dir);
		if(len > 1e-3f) {
			start_dir /= len;
		} else start_dir.set(1,0,0);
		Vec3 anim_dir = GetAnimDir( m_cur_edge );
		float angle = acos(dot(anim_dir, start_dir));
		m_cur_rotation = make_rotation(angle, Vec3(0,1,0));	
		m_cur_offset = m_requested_path.Front() - rotate(GetAnimStart( m_cur_edge ), m_cur_rotation);
		m_cur_offset.y = 0;

		m_edges_to_walk.clear();
		FindBestGraphWalk();
		
		m_walking = true;
	}

}

struct ClipDiscoverNodes { 
	ClipDiscoverNodes( std::set<sqlite3_int64> *the_set ) : cur_set(the_set) {}
	std::set<sqlite3_int64> *cur_set;
	
	bool operator()(const AlgorithmMotionGraph::Node* node) {
		cur_set->insert(node->clip_id);
		return true;
	}
};

struct ClipDiscoverEdges { 
	ClipDiscoverEdges( std::set<sqlite3_int64> *the_set ) : cur_set(the_set) {}
	std::set<sqlite3_int64> *cur_set;
	
	bool operator()(const AlgorithmMotionGraph::Edge* edge) {
		cur_set->insert(edge->clip_id);
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

struct ClipDistributeEdges {
	ClipDistributeEdges( const std::vector< ClipHandle >* the_clips) : clips(the_clips) {}
	const std::vector< ClipHandle >* clips;

	bool operator()(AlgorithmMotionGraph::Edge* edge) {
		int idx = FindClipIn(*clips, edge->clip_id);
		if(idx != -1) edge->clip = clips->at( idx );
		return true;
	}
};

void MotionGraphState::SetGraph( sqlite3*db, const Skeleton* skel,  AlgorithmMotionGraphHandle handle )
{
	if(m_clip_controller == 0 || skel != m_clip_controller->GetSkeleton()) {
		delete m_clip_controller; m_clip_controller = 0;
		if(skel)
			m_clip_controller = new ClipController(skel);
		else return;
	}

	m_algo_graph = handle;
	if(!handle) {
		return;
	}

	m_clip_controller->SetClip(0);

	// collect unique clip ids
	std::set< sqlite3_int64 > unique_clips;
	ClipDiscoverNodes discoverNodes( &unique_clips );
	ClipDiscoverEdges discoverEdges( &unique_clips );
	handle->VisitNodes(discoverNodes);
	handle->VisitEdges(discoverEdges);

	// cache handles
	std::set< sqlite3_int64 >::iterator iter = unique_clips.begin(),
		end = unique_clips.end();
	while(iter != end) {
		sqlite3_int64 clip_id = *iter;
		ClipHandle clip = new Clip(db, clip_id);
		if(clip->Valid()) 
			m_cached_clips.push_back(clip);
		++iter;
	}

	// distribute handles back into graph to avoid further lookups
	ClipDistributeNodes distributeNodes( &m_cached_clips );
	ClipDistributeEdges distributeEdges( &m_cached_clips );
	handle->VisitNodes( distributeNodes );
	handle->VisitEdges( distributeEdges );
}

void MotionGraphState::NextEdge()
{
	m_cur_edge = m_edges_to_walk.front();
	m_edges_to_walk.pop_front();
	m_last_node = m_cur_edge->end;
	
	m_clip_controller->SetClip( m_cur_edge->clip.RawPtr() );
	m_clip_controller->SetTime(0.f);	

	float start_frame = 0.f;
	float end_frame = m_cur_edge->clip->GetNumFrames() - 1.f;

	if(m_cur_edge->start->clip_id == m_cur_edge->end->clip_id &&
	   m_cur_edge->start->clip_id == m_cur_edge->clip_id)
	{
		start_frame = m_cur_edge->start->frame_num;
		end_frame = m_cur_edge->end->frame_num;
	}

	m_clip_controller->SetPartialRange(start_frame, end_frame);	
	// printf("NextEdge: edge %p clip %d %s from frame %d to frame %d start %f end %f\n", 
	// 	   m_cur_edge,
	// 	   (int)m_cur_edge->clip->GetID(), 
	// 	   m_cur_edge->clip->GetName(), 
	// 	   m_cur_edge->start->frame_num, 
	// 	   m_cur_edge->end->frame_num,
	// 	   start_frame, end_frame);
}

void MotionGraphState::Update(float dt)
{
	if(m_walking)
	{
		// assume we have edges to walk
		float curTime = m_clip_controller->GetTime();

		m_clip_controller->UpdateTime( dt );
		
		if(m_clip_controller->IsAtEnd()) {
			float leftover = (curTime + dt) - m_clip_controller->GetTime();

			m_cur_offset = m_cur_offset + rotate(m_cur_edge->align_offset, m_cur_rotation);
			m_cur_rotation = normalize(m_cur_rotation * m_cur_edge->align_rotation);

			if(m_edges_to_walk.empty()) {
				FindBestGraphWalk( );
			}

			NextEdge();
			m_clip_controller->SetTime( m_clip_controller->GetTime() + leftover );			
		}

		m_clip_controller->ComputePose();

		Pose* pose = m_clip_controller->GetPose();
		Vec3 pose_offset = pose->GetRootOffset();
		Quaternion pose_rot = pose->GetRootRotation();
		
		pose_offset = m_cur_offset + rotate(pose_offset, m_cur_rotation);
		pose_rot = normalize(m_cur_rotation * pose_rot);

		pose->SetRootOffset(pose_offset);
		pose->SetRootRotation(pose_rot);

		// apply transformation to pose, project root onto ground
		pose_offset.y = 0.f; // project to y=0 plane
		m_path_so_far.AddPoint( pose_offset );

		if( m_path_so_far.TotalLength() >= m_requested_path.TotalLength() ||
			m_path_so_far.Full() )
		{
			m_walking = false;
		}		
	}
	else if (m_clip_controller) {
		m_clip_controller->GetPose()->RestPose(m_clip_controller->GetSkeleton());
	}
}

const Pose* MotionGraphState::GetPose() const 
{
	if(m_clip_controller) 
		return m_clip_controller->GetPose();
	else 
		return 0;
}

void MotionGraphState::ComputeMatrices( Mat4_arg m ) 
{
	if(m_clip_controller){ 
		m_clip_controller->ComputeMatrices(m);
	}
}

const Skeleton* MotionGraphState::GetSkeleton() const
{
	if(m_clip_controller) {
		return m_clip_controller->GetSkeleton();
	} else return 0;
}

////////////////////////////////////////////////////////////////////////////////
static const float kSearchTimeDepth = 6.f;
static const float kFrameTimeToRetain = 1.f;

struct SearchNode {
	SearchNode() 
		: edge(0)
		, parent(0)
		, error(0.f)
		, time(0.f)
		, align_offset(0,0,0)
		, align_rotation(0,0,0,1)
		, start_pos(0,0,0)
		, start_rotation(0,0,0,1)
		, end_pos(0,0,0)
		, end_rotation(0,0,0,1)
		, arclength(0.f)
		{}
	AlgorithmMotionGraph::Edge* edge;
	
	// needed for doing a graph walk and reconstructing the result
	SearchNode* parent;
	float error;
	float time;

	Vec3 align_offset;
	Quaternion align_rotation;

	Vec3 start_pos;
	Quaternion start_rotation;

	Vec3 end_pos; // after done
	Quaternion end_rotation;

	float arclength; // arc length on travelled path after taking this edge
};

float EstimateArcLength(const Clip* clip, int start_frame, int end_frame)
{
	float result = 0.f;
	const int num_frames = end_frame - start_frame + 1;
	if(num_frames < 2) {
		return 0.f;
	}
	Vec3 lastPt = clip->GetFrameRootOffset(start_frame);
	for(int i = start_frame; i <= end_frame; ++i)
	{
		Vec3 curPt = clip->GetFrameRootOffset(i);
		result += magnitude(curPt - lastPt);
		lastPt = curPt;
	}
	return result;
}

void ComputePosition( SearchNode& info, int start_frame, int end_frame)
{
	const Clip* clip = info.edge->clip.RawPtr();

	Vec3 start_offset = info.align_offset + rotate(clip->GetFrameRootOffset(start_frame), info.align_rotation);
	Vec3 end_offset = info.align_offset + rotate(clip->GetFrameRootOffset(end_frame), info.align_rotation);
	
	start_offset.y = end_offset.y = 0.f;
	info.start_pos = start_offset;
	info.end_pos = end_offset;

	Quaternion start_rot = info.align_rotation * clip->GetFrameRootOrientation(start_frame);
	Quaternion end_rot = info.align_rotation * clip->GetFrameRootOrientation(end_frame);
	info.start_rotation = start_rot;
	info.end_rotation = end_rot;
}

float MotionGraphState::ComputeError(const SearchNode& info)
{
	Vec3 requested_point = m_requested_path.PointAtLength(info.arclength);
	return magnitude_squared( info.end_pos - requested_point );
}

void MotionGraphState::CreateSearchNode(SearchNode& out, 
										AlgorithmMotionGraph::Edge* edge,
										SearchNode* parent)
{
	out.edge = edge;
	out.parent = parent;

	int start_frame = 0;
	int end_frame = edge->clip->GetNumFrames() - 1;

	if(edge->start->clip_id == edge->end->clip_id &&
	   edge->start->clip_id == edge->clip_id)
	{
		start_frame = edge->start->frame_num;
		end_frame = edge->end->frame_num;
	}

	int num_frames = end_frame - start_frame + 1;

	if(parent)
	{
		out.time = parent->time + num_frames / edge->clip->GetClipFPS();
		out.align_offset = parent->align_offset + rotate( parent->edge->align_offset, parent->align_rotation ) ;
		out.align_rotation = normalize(parent->align_rotation * parent->edge->align_rotation);
		out.arclength = parent->arclength + EstimateArcLength(edge->clip.RawPtr(), start_frame, end_frame);
		ComputePosition(out, start_frame, end_frame);
		out.error = parent->error + ComputeError(out);
	}
	else
	{
		out.time = num_frames / edge->clip->GetClipFPS();
		out.align_offset = m_cur_offset;
		out.align_rotation = m_cur_rotation;
		out.arclength = m_path_so_far.TotalLength() + EstimateArcLength(edge->clip.RawPtr(), start_frame, end_frame);
		ComputePosition(out, start_frame, end_frame);
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

void MotionGraphState::FindBestGraphWalk()
{
	SearchNode* best_walk_root = 0;

	m_cur_search_nodes.clear();

	std::list< SearchNode > search_nodes ; // sort of acting as an 'allocator' in this case. 
	std::set< SearchNode*, compare_search_nodes > open_list ;

	const int last_num_neighbors = m_last_node->outgoing.size();
	for(int i = 0; i < last_num_neighbors; ++i) {
		AlgorithmMotionGraph::Edge* cur = m_last_node->outgoing[i];
		search_nodes.push_back( SearchNode() );
		SearchNode &info = search_nodes.back();
		CreateSearchNode(info, cur, 0);
		open_list.insert(&info);
	}

	int search_count = 0;
	while(!open_list.empty())
	{
		SearchNode* info = *open_list.begin();
		open_list.erase( open_list.begin() );
		
		m_cur_search_nodes.push_back(info->start_pos);
		m_cur_search_nodes.push_back(info->end_pos);
		++search_count;

		// is this a result?
		if(info->time >= kSearchTimeDepth) {
			if( (best_walk_root == 0 || info->error < best_walk_root->error) ) {
				best_walk_root = info;
			}
		} else {
			// if THIS node is not over the max search depth length, search more!
			const int num_neighbors = info->edge->end->outgoing.size();
			for(int i = 0; i < num_neighbors; ++i) {
				search_nodes.push_back( SearchNode() );
				SearchNode& sn = search_nodes.back();
				CreateSearchNode(sn, info->edge->end->outgoing[i], info);
				ASSERT(sn.time > info->time);
					
				// if we already have a better path, there's no point in considering this one if it's already worse
				if(best_walk_root == 0 || sn.error < best_walk_root->error) {
					open_list.insert(&sn);
				} else {
					search_nodes.pop_back();
				}
			}
		}
	}

	// printf("searched %d nodes, allocated %d\n", search_count, search_nodes.size());

	if(best_walk_root) {
		int count = 0;
		SearchNode* cur = best_walk_root ;
		while(cur) {
			++count;
			cur = cur->parent;
		}

		std::vector< SearchNode* > best_edges;
		best_edges.resize(count, 0);
		m_cur_path.clear();
		int idx = count - 1;
		cur = best_walk_root;
		while(cur) {
			best_edges[idx] = cur;
			m_cur_path.push_back(cur->start_pos);
			m_cur_path.push_back(cur->end_pos);
			--idx;
			cur = cur->parent;
		}

		// Now only use the first portion of the result.
		float time_so_far = 0.f;
		idx = 0;
		while(idx < count && time_so_far < kFrameTimeToRetain)
		{
			// printf("walk edge: %p clip %d\n", best_edges[idx]->edge,
			// 	   (int)best_edges[idx]->edge->clip->GetID());
			m_edges_to_walk.push_back( best_edges[idx]->edge );
			time_so_far = best_edges[idx]->time; // this is the time along this path
			++idx;
		}
//		printf("search path length %f\n", time_so_far);
	} else {
		fprintf(stderr, "oh no! no best walk.\n");
	}
	
}

void MotionGraphState::DebugDraw()
{
	const int s1 = (m_cur_search_nodes.size()/2)*2;
	glColor4f(0.8,0.8,0.8,0.3);
	glBegin(GL_LINES);
	for(int i = 0; i < s1; i += 2) {
		glVertex3fv( &m_cur_search_nodes[i].x );
		glVertex3fv( &m_cur_search_nodes[i+1].x );
	}
	glEnd();

	glPushMatrix();
	glTranslatef(0,0.01f,0);
	glBegin(GL_LINES);
	glColor4f(0.8,0,0,0.8);
	const int s2 = 2*(m_cur_path.size()/2);
	for(int i = 0; i < s2; i += 2) {
		glVertex3fv( &m_cur_path[i].x );
		glVertex3fv( &m_cur_path[i+1].x );
	}
	glEnd();
	glPopMatrix();

	glBegin(GL_LINES);
	glColor3f(1,1,0);
	glVertex3fv( &m_path_so_far.Back().x );
	glVertex3fv( &m_requested_path.PointAtLength(m_path_so_far.TotalLength()).x );
	glEnd();
}
