#ifndef INCLUDED_clip_HH
#define INCLUDED_clip_HH

#include <string>
#include "intrusive_ptr.hh"
#include "dbhelpers.hh"
#include "Vector.hh"
#include "MathUtil.hh"

class Skeleton;
class Vec3;
class Quaternion;

namespace LBF { class WriteNode; class ReadNode; }

struct ClipFrameHeader {
	Vec3 root_offset;
	Quaternion root_quaternion;

	inline Quaternion *GetJointRotations() {
		return reinterpret_cast<Quaternion*>(
			reinterpret_cast<char*>(this) + sizeof(this));
	}
};

class Clip : public refcounted_type<Clip>
{
	sqlite3 *m_db;
	sqlite3_int64 m_id;

	////////////////////////////////////////
	// Cached data, unchanged after loading
	std::string m_clip_name;
	int m_num_frames;
	int m_joints_per_frame;
	float m_fps; // associated sample rate - how fast should we go through these frames

	Quaternion* m_frame_data; // size determined by skeleton, num_frames * num_joints

	Vec3* m_root_offsets; 
	Quaternion* m_root_orientations;
public:

	Clip(sqlite3 *db, sqlite3_int64 clip_id);
	~Clip();

	sqlite3_int64 GetID() const { return m_id; }
	bool Valid() const { return m_id != 0; }

	void SetName(const char* name) ;
	const char* GetName() const { return m_clip_name.c_str(); }

	const Quaternion* GetFrameRotations(int frameIdx) const ;
	const Vec3& GetFrameRootOffset(int frameIdx) const;
	const Quaternion& GetFrameRootOrientation(int frameIdx) const;

	int GetNumFrames() const { return m_num_frames; }
	float GetClipTime() const { return m_num_frames / m_fps; }
	float GetClipFPS() const { return m_fps; }

	LBF::WriteNode* CreateClipWriteNode() const;
	static sqlite3_int64 ImportClipFromReadNode(sqlite3* db, sqlite3_int64 skel_id, 
												const LBF::ReadNode& rn);
private:
	bool LoadFromDB();
};
typedef reference<Clip> ClipHandle;

////////////////////////////////////////////////////////////////////////////////
// Clip utility functions

// Get a vector from the clip position at fromFrame to the clip root pos at toFrame
inline Vec3 GetAnimDir( const ClipHandle& clipHandle, int fromFrame = 0, int toFrame = -1 )
{
	const Clip* clip = clipHandle.RawPtr();
	return normalize(clip->GetFrameRootOffset(toFrame) - clip->GetFrameRootOffset(fromFrame));
}

inline Vec3 GetAnimStart(const ClipHandle& clipHandle, int atFrame = 0)
{
	const Clip* clip = clipHandle.RawPtr();
	return clip->GetFrameRootOffset(atFrame);
}

inline int GetFrameFromTime(const Clip* clip, float time)
{
    return Clamp( time * clip->GetClipFPS(), 0.f, (float)(clip->GetNumFrames() - 1));
}

#endif
