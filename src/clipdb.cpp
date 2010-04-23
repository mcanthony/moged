#include <algorithm>
#include "clipdb.hh"
#include "clip.hh"

ClipDB::ClipDB()
{
}

ClipDB::~ClipDB()
{
	int num = m_clips.size();
	for(int i = 0; i < num; ++i) {
		delete m_clips[i];
	}
}

void ClipDB::AddClip(Clip* clip)
{
	m_clips.push_back(clip);
}

void ClipDB::RemoveClip(Clip* clip)
{
	// this is lame, want better way of doing this... if the db gets large, this will be awful
	std::remove(m_clips.begin(), m_clips.end(), clip);
	delete clip;
}

int ClipDB::GetNumClips() const 
{
	return m_clips.size();
}

const Clip* ClipDB::GetClip(int index) const 
{
	return m_clips[index];
}


