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

