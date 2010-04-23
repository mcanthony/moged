#include "clip.hh"
#include "skeleton.hh"
#include "assert.hh"

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


