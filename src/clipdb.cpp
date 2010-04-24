#include <cstdio>
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
	// could binary search pointers... or just use a linked list 
	
	int size = m_clips.size();
	for(int i = 0; i < size; ++i) {
		if(m_clips[i] == clip) {
			delete clip;
			m_clips[i] = m_clips[size-1];
			m_clips.pop_back();
			return;
		}
	}

	printf("Called delete on %p but did not find it in DB! weird.\n", clip);
}

int ClipDB::GetNumClips() const 
{
	return m_clips.size();
}

const Clip* ClipDB::GetClip(int index) const 
{
	return m_clips[index];
}


