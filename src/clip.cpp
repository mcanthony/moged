#include "clip.hh"
#include "skeleton.hh"
#include "assert.hh"
#include "lbfloader.hh"

Clip::Clip(int num_joints, int num_frames, float fps)
	: m_num_frames(num_frames)
	, m_joints_per_frame(num_joints)
	, m_fps(fps)
	, m_frame_data(0)
	, m_root_offsets(0)
	, m_root_orientations(0)
{
	ASSERT(fps > 0.f && num_frames > 0);

	const int size = num_joints * num_frames;
	m_frame_data = new Quaternion[ size ];
	m_root_offsets = new Vec3[ num_frames ];
	m_root_orientations = new Quaternion [ num_frames ];

	memset(m_frame_data,0,sizeof(Quaternion)*size);
	memset(m_root_offsets,0,sizeof(Vec3)*num_frames);
	memset(m_root_orientations,0,sizeof(Quaternion)*num_frames);
}

Clip::~Clip()
{
	delete[] m_frame_data;
	delete[] m_root_offsets;
	delete[] m_root_orientations;
}

Quaternion* Clip::GetFrameRotations(int frameIdx)
{
	ASSERT(frameIdx >= 0 && frameIdx < m_num_frames);
	return &m_frame_data[frameIdx * m_joints_per_frame];
}

const Quaternion* Clip::GetFrameRotations(int frameIdx) const 
{
	ASSERT(frameIdx >= 0 && frameIdx < m_num_frames);
	return &m_frame_data[frameIdx * m_joints_per_frame];
}

Vec3& Clip::GetFrameRootOffset(int frameIdx)
{
	ASSERT(frameIdx >= 0 && frameIdx < m_num_frames);
	return m_root_offsets[frameIdx];
}

const Vec3& Clip::GetFrameRootOffset(int frameIdx) const
{
	ASSERT(frameIdx >= 0 && frameIdx < m_num_frames);
	return m_root_offsets[frameIdx];
}

Quaternion& Clip::GetFrameRootOrientation(int frameIdx)
{
	ASSERT(frameIdx >= 0 && frameIdx < m_num_frames);
	return m_root_orientations[frameIdx];
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

LBF::WriteNode* Clip::createClipWriteNode() const
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

Clip* Clip::createClipFromReadNode(const LBF::ReadNode& rn)
{
	Clip* clip = 0;
	if(rn.Valid() && rn.GetType() == LBF::ANIMATION) 
	{
		clip_save_info info;
		rn.GetData(&info, sizeof(info));
		clip = new Clip(info.joints_per_frame, info.num_frames, info.fps);
	
		LBF::ReadNode rnName = rn.GetFirstChild(LBF::ANIMATION_NAME);
		if(rnName.Valid()) {
			clip->m_clip_name = std::string(rnName.GetNodeData(), rnName.GetNodeDataLength());
		} else {
			clip->m_clip_name = "missing name";
		}

		LBF::ReadNode rnRots = rn.GetFirstChild(LBF::FRAME_ROTATIONS);
		if(rnRots.Valid()) {
			long rotSize = sizeof(Quaternion)*info.num_frames*info.joints_per_frame;
			rnRots.GetData(clip->m_frame_data, rotSize);
		}

		LBF::ReadNode rnRootOff = rn.GetFirstChild(LBF::FRAME_ROOT_OFFSETS);
		if(rnRootOff.Valid()) {
			long rootOffSize = sizeof(Vec3)*info.num_frames;
			rnRootOff.GetData(clip->m_root_offsets, rootOffSize);
		}

		LBF::ReadNode rnRootRots = rn.GetFirstChild(LBF::FRAME_ROOT_ROTATIONS);
		if(rnRootRots.Valid())
		{
			long rootRotSize = sizeof(Quaternion)*info.num_frames;
			rnRootRots.GetData(clip->m_root_orientations, rootRotSize);
		}		
	}
	return clip;
}
