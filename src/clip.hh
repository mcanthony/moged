#ifndef INCLUDED_clip_HH
#define INCLUDED_clip_HH

#include <string>

class Skeleton;
class Quaternion;
class Vec3;

namespace LBF { class WriteNode; class ReadNode; }

class Clip
{
	std::string m_clip_name;
	int m_num_frames;
	int m_joints_per_frame;
	float m_fps; // associated sample rate - how fast should we go through these frames

	Quaternion* m_frame_data; // size determined by skeleton, num_frames * num_joints

	Vec3* m_root_offsets; 
	Quaternion* m_root_orientations;
public:

	Clip(int num_joints, int num_frames, float fps);
	~Clip();

	void SetName(const char* name) { m_clip_name = name; }
	const char* GetName() const { return m_clip_name.c_str(); }

	Quaternion* GetFrameRotations(int frameIdx);
	const Quaternion* GetFrameRotations(int frameIdx) const ;

	Vec3& GetFrameRootOffset(int frameIdx);
	const Vec3& GetFrameRootOffset(int frameIdx) const;
	
	Quaternion& GetFrameRootOrientation(int frameIdx);
	const Quaternion& GetFrameRootOrientation(int frameIdx) const;

	int GetNumFrames() const { return m_num_frames; }
	float GetClipTime() const { return m_num_frames / m_fps; }
	float GetClipFPS() const { return m_fps; }

	LBF::WriteNode* createClipWriteNode() const;
	static Clip* createClipFromReadNode(const LBF::ReadNode& rn);
};

#endif
