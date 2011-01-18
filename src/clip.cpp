#include <vector>
#include "clip.hh"
#include "skeleton.hh"
#include "assert.hh"
#include "lbfloader.hh"
#include "sql/sqlite3.h"

Clip::Clip(sqlite3 *db, sqlite3_int64 clip_id)
	: m_db(db)
	, m_id(clip_id)
	, m_num_frames(0)
	, m_joints_per_frame(0)
	, m_fps(0)
	, m_frame_data(0)
	, m_root_offsets(0)
	, m_root_orientations(0)
{
	if(!LoadFromDB()) {
		m_id = 0;
	}
}

Clip::~Clip()
{
	delete[] m_frame_data;
	delete[] m_root_offsets;
	delete[] m_root_orientations;
}

bool Clip::LoadFromDB()
{
	// Find number of frames
	Query count_frames(m_db, "SELECT count(*) FROM frames WHERE clip_id = ?");
	count_frames.BindInt64(1, m_id);
	int num_frames = 0;
	if( count_frames.Step() ) {
		num_frames = count_frames.ColInt(0) ;
	}
	
	// Find number of joints
	Query count_joints(m_db,  "SELECT num_joints FROM skeleton WHERE id = ?");
	count_joints.BindInt64(1, m_id);
	int num_joints = 0;
	if( count_joints.Step() ) {
		num_joints = count_joints.ColInt(0) ;
	}

	// allocate required buffers
	const int size = num_joints * num_frames;
	if(size == 0) return false;

	m_num_frames = num_frames;
	m_joints_per_frame = num_joints;
	m_frame_data = new Quaternion[ size ];
	m_root_offsets = new Vec3[ num_frames ];
	m_root_orientations = new Quaternion [ num_frames ];
	memset(m_frame_data,0,sizeof(Quaternion)*size);
	memset(m_root_offsets,0,sizeof(Vec3)*num_frames);
	memset(m_root_orientations,0,sizeof(Quaternion)*num_frames);

	// Get basic clip info
	Query get_clip(m_db, "SELECT name,fps FROM clips WHERE id = ?");
	get_clip.BindInt64(1, m_id);
	if( get_clip.Step() ) {
		m_clip_name = get_clip.ColText(0);
		m_fps = get_clip.ColDouble(1);
	} else return false;

	// Get all frames at once. 'num' is the 'index' of the frame.
	Query get_frames(m_db, "SELECT num,"
					 "root_offset_x, root_offset_y, root_offset_z,"
					 "root_rotation_a, root_rotation_b, root_rotation_c, root_rotation_r "
					 "FROM frames WHERE clip_id = ? ORDER BY num ASC");
	get_frames.BindInt64( 1, m_id);
	while( get_frames.Step() ) {
		int offset = get_frames.ColInt( 0 );
		ASSERT(offset <num_frames);
		m_root_offsets[offset] = get_frames.ColVec3(1);
		m_root_orientations[offset] = get_frames.ColQuaternion(4);
	}

	Query get_rots(m_db, 
				   "SELECT frames.num, frame_rotations.joint_offset, "
				   "frame_rotations.q_a, frame_rotations.q_b, frame_rotations.q_c, frame_rotations.q_r FROM "
				   "frames "
				   "LEFT JOIN frame_rotations ON frames.id = frame_rotations.frame_id "
				   "WHERE frames.clip_id = ? "
				   "ORDER BY frames.num ASC,frame_rotations.joint_offset ASC");
	get_rots.BindInt64(1, m_id);
	while( get_rots.Step() ) {
		int frame_num = get_rots.ColInt(0);
		int joint_off = get_rots.ColInt(1);
		ASSERT(frame_num < num_frames);
		ASSERT(joint_off < num_joints);
		m_frame_data[joint_off + num_joints * frame_num] = get_rots.ColQuaternion(2);
	}
	
	return true;
}

void Clip::SetName(const char* name) 
{
	ASSERT(name);
	m_clip_name = name;
	sql_begin_transaction(m_db);
	
	Query set_name(m_db, "UPDATE clips SET name = ? WHERE id = ?");
	set_name.BindInt64(2, m_id);
	set_name.BindText(1, name);
	set_name.Step();

	sql_end_transaction(m_db);
}

const Quaternion* Clip::GetFrameRotations(int frameIdx) const 
{
	ASSERT(frameIdx >= 0 && frameIdx < m_num_frames);
	return &m_frame_data[frameIdx * m_joints_per_frame];
}

const Vec3& Clip::GetFrameRootOffset(int frameIdx) const
{
	ASSERT(frameIdx >= 0 && frameIdx < m_num_frames);
	return m_root_offsets[frameIdx];
}

const Quaternion& Clip::GetFrameRootOrientation(int frameIdx) const
{
	ASSERT(frameIdx >= 0 && frameIdx < m_num_frames);
	return m_root_orientations[frameIdx];
}

struct clip_save_info
{
	int num_frames;
	int joints_per_frame;
	float fps;
	char reserved[4];
};

LBF::WriteNode* Clip::CreateClipWriteNode() const
{
	LBF::WriteNode* node = new LBF::WriteNode(LBF::ANIMATION,0,sizeof(clip_save_info));
	clip_save_info info;
	info.num_frames = m_num_frames;
	info.joints_per_frame = m_joints_per_frame;
	info.fps = m_fps;
	node->PutData(&info, sizeof(info));

	LBF::WriteNode* wnName = new LBF::WriteNode(LBF::ANIMATION_NAME,0,sizeof(char)*m_clip_name.length());
	node->AddChild(wnName);
	wnName->PutData(&m_clip_name[0], sizeof(char)*m_clip_name.length());

	long rotSize = sizeof(Quaternion)*info.num_frames*info.joints_per_frame;
	LBF::WriteNode* wnRot = new LBF::WriteNode(LBF::FRAME_ROTATIONS, 0, rotSize);
	node->AddChild(wnRot);
	wnRot->PutData(m_frame_data, rotSize);
	
	long rootOffSize = sizeof(Vec3)*info.num_frames;
	LBF::WriteNode* wnRootOff = new LBF::WriteNode(LBF::FRAME_ROOT_OFFSETS, 0, rootOffSize);
	node->AddChild(wnRootOff);
	wnRootOff->PutData(m_root_offsets, rootOffSize);

	long rootRotSize = sizeof(Quaternion)*info.num_frames;
	LBF::WriteNode* wnRootRots = new LBF::WriteNode(LBF::FRAME_ROOT_ROTATIONS, 0, rootRotSize);
	node->AddChild(wnRootRots);
	wnRootRots->PutData(m_root_orientations, rootRotSize);

	return node;
}

sqlite3_int64 Clip::ImportClipFromReadNode(sqlite3* db, sqlite3_int64 skel_id, const LBF::ReadNode& rn)
{
	sqlite3_int64 result = 0;
	if(rn.Valid() && rn.GetType() == LBF::ANIMATION) 
	{
		clip_save_info info;
		rn.GetData(&info, sizeof(info));

		LBF::ReadNode rnRots = rn.GetFirstChild(LBF::FRAME_ROTATIONS);
		if(!rnRots.Valid()) return 0;
		LBF::ReadNode rnRootOff = rn.GetFirstChild(LBF::FRAME_ROOT_OFFSETS);
		if(!rnRootOff.Valid()) return 0;
		LBF::ReadNode rnRootRots = rn.GetFirstChild(LBF::FRAME_ROOT_ROTATIONS);
		if(!rnRootRots.Valid()) return 0;

		sql_begin_transaction(db);
		
		std::string clipName ;
		LBF::ReadNode rnName = rn.GetFirstChild(LBF::ANIMATION_NAME);
		if(rnName.Valid()) {
			clipName = std::string(rnName.GetNodeData(), rnName.GetNodeDataLength());
		} else {
			clipName = "missing name";
		}

		// add basic clip info
		Query insert_clip( db, "INSERT INTO clips (skel_id,name,fps) VALUES (?,?,?)");
		insert_clip.BindInt64(1, skel_id);
		insert_clip.BindText(2, clipName.c_str());
		insert_clip.BindDouble(3, info.fps);
		insert_clip.Step();
		if( insert_clip.IsError() ) {
			sql_rollback_transaction(db);	
			return 0;
		}
		result = insert_clip.LastRowID();

		// add a frame header
		std::vector<sqlite3_int64> frame_ids;
		frame_ids.resize(info.num_frames, 0);
		BufferReader rootOffs = rnRootOff.GetReader();
		BufferReader rootRots = rnRootRots.GetReader();

		Query insert_frame(db, "INSERT INTO frames (clip_id,num,"
						   "root_offset_x, root_offset_y, root_offset_z, "
						   "root_rotation_a, root_rotation_b, root_rotation_c, root_rotation_r) "
						   "VALUES(?, ?, ?,?,?, ?,?,?,?)");
		insert_frame.BindInt64(1, result);
		for(int i = 0; i < info.num_frames; ++i)
		{
			insert_frame.Reset();
			insert_frame.BindInt(2, i);

			Vec3 off;
			rootOffs.Get(&off, sizeof(off));
			insert_frame.BindVec3(3, off);

			Quaternion q;
			rootRots.Get(&q, sizeof(q));
			insert_frame.BindQuaternion(6, q);

			insert_frame.Step();
			frame_ids[i] = insert_frame.LastRowID();
		}

		// add joint rotations for each frame
		BufferReader rotsReader = rnRots.GetReader();
		Query insert_rotations(db, "INSERT INTO frame_rotations "
							   "(frame_id, skel_id, joint_offset, q_a, q_b, q_c, q_r) "
							   "VALUES (?, ?,?, ?,?,?,?)");		
		insert_rotations.BindInt64(2, skel_id);
		for(int frame = 0; frame < info.num_frames; ++frame)
		{
			sqlite3_int64 frame_id = frame_ids[frame];
			insert_rotations.Reset();
			insert_rotations.BindInt64(1, frame_id);
			for(int joint = 0; joint < info.joints_per_frame; ++joint) 
			{
				insert_rotations.Reset();
			
				Quaternion q;
				rotsReader.Get(&q, sizeof(q));

				insert_rotations.BindInt64(3, joint);
				insert_rotations.BindQuaternion(4, q);

				insert_rotations.Step();
			}
		}

		sql_end_transaction(db);
	}
	return result;
}
