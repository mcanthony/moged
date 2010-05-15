#ifndef INCLUDED_clip_HH
#define INCLUDED_clip_HH

#include <string>
#include "intrusive_ptr.hh"
#include "dbhelpers.hh"

class Skeleton;
class Vec3;
class Quaternion;

namespace LBF { class WriteNode; class ReadNode; }

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

#endif
