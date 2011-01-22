#include <vector>
#include "clip.hh"
#include "skeleton.hh"
#include "assert.hh"
#include "lbfloader.hh"
#include "sql/sqlite3.h"

// TODO binary-ize this whole file

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
	sqlite3_int64 clip_id = 0;

	// Find number of frames
	Query info_query(m_db, "SELECT id,name,fps,num_frames FROM clips WHERE id = ?");
	info_query.BindInt64(1, m_id);
	int num_frames = 0;
	if( info_query.Step() ) {
		clip_id = info_query.ColInt64(0);
		m_clip_name = info_query.ColText(1);
		m_fps = info_query.ColDouble(2);
		num_frames = info_query.ColInt(3) ;
	} else 
		return false;
	
	// Find number of joints
	Query count_joints(m_db,  "SELECT num_joints FROM skeleton WHERE id = ?");
	count_joints.BindInt64(1, m_id);
	int num_joints = 0;
	if( count_joints.Step() ) {
		num_joints = count_joints.ColInt(0) ;
	}

	// allocate required buffers
	const int size = num_joints * num_frames;
	if(size == 0) {
		m_clip_name.clear();
		m_fps = 0;
		return false;
	}

	m_num_frames = num_frames;
	m_joints_per_frame = num_joints;
	m_frame_data = new Quaternion[ size ];
	m_root_offsets = new Vec3[ num_frames ];
	m_root_orientations = new Quaternion [ num_frames ];
	memset(m_frame_data,0,sizeof(Quaternion)*size);
	memset(m_root_offsets,0,sizeof(Vec3)*num_frames);
	memset(m_root_orientations,0,sizeof(Quaternion)*num_frames);

	Blob frameDataReader(m_db, "clips", "frames", clip_id, false);
	int frameDataOffset = 0;

	const int byteCountFrameData = sizeof(Quaternion)*num_joints;
	int curFrameJointCount = 0;
	
	for(int frame = 0; frame < num_frames; ++frame)
	{
		ClipFrameHeader header;
		frameDataReader.Read(&header, sizeof(header), frameDataOffset);
		frameDataOffset += sizeof(header);

		m_root_offsets[frame] = header.root_offset;
		m_root_orientations[frame] = header.root_quaternion;

		frameDataReader.Read(&m_frame_data[curFrameJointCount], byteCountFrameData, frameDataOffset);
		curFrameJointCount += num_joints;
		frameDataOffset += byteCountFrameData;
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
		Query insert_clip( db, "INSERT INTO clips (skel_id,name,fps,num_frames,frames) VALUES (?,?,?,?,?)");
		insert_clip.BindInt64(1, skel_id);
		insert_clip.BindText(2, clipName.c_str());
		insert_clip.BindDouble(3, info.fps);
		insert_clip.BindInt64(4, info.num_frames);
		insert_clip.BindBlob(5, sizeof(Quaternion) * info.num_frames * info.joints_per_frame +
			sizeof(ClipFrameHeader) * info.num_frames);			
		insert_clip.Step();
		if( insert_clip.IsError() ) {
			sql_rollback_transaction(db);	
			return 0;
		}

		result = insert_clip.LastRowID();

		Blob framesWriter(db, "clips", "frames", result, true);
		int framesOffset = 0;

		// add a frame header
		BufferReader rootOffs = rnRootOff.GetReader();
		BufferReader rootRots = rnRootRots.GetReader();
		BufferReader rotsReader = rnRots.GetReader();

		for(int i = 0; i < info.num_frames; ++i)
		{
			ClipFrameHeader header;
			rootOffs.Get(&header.root_offset, sizeof(header.root_offset));
			rootRots.Get(&header.root_quaternion, sizeof(header.root_quaternion));

			framesWriter.Write(&header, sizeof(header), framesOffset);
			framesOffset += sizeof(header);
			
			Quaternion q;
			for(int joint = 0; joint < info.joints_per_frame; ++joint) 
			{
				// TODO: would rather do this in one call...
				rotsReader.Get(&q, sizeof(q));
				framesWriter.Write(&q, sizeof(q), framesOffset);
				framesOffset += sizeof(q);
			}
		}

		sql_end_transaction(db);
	}
	return result;
}
