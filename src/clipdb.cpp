#include <cstdio>
#include "clipdb.hh"
#include "clip.hh"
#include "lbfloader.hh"
#include "lbfhelpers.hh"
#include "assert.hh"

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

LBF::WriteNode* createClipsWriteNode( const ClipDB* clips )
{
	LBF::WriteNode* firstClip = 0;
	LBF::WriteNode* curInsert = 0;

	int num_clips = clips->GetNumClips();
	for(int i = 0; i < num_clips; ++i)
	{
		LBF::WriteNode* clipNode = clips->GetClip(i)->createClipWriteNode();
		if(firstClip == 0) {
			firstClip = clipNode;
		} else {
			curInsert->AddSibling(clipNode);
		}
		curInsert = clipNode;
	}
	
	return firstClip;
}

ClipDB* createClipsFromReadNode( const LBF::ReadNode& rn )
{
	ASSERT(rn.GetType() == LBF::ANIMATION);
	
	ClipDB* db = new ClipDB;
	LBF::ReadNode cur =rn;
	while(cur.Valid()) {
		Clip* clip = Clip::createClipFromReadNode(cur);
		db->AddClip(clip);
		cur = cur.GetNext(LBF::ANIMATION);
	}
	return db;
}

