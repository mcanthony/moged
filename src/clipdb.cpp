#include <cstdio>
#include "sql/sqlite3.h"
#include "clipdb.hh"
#include "clip.hh"
#include "lbfloader.hh"
#include "lbfhelpers.hh"
#include "assert.hh"

ClipDB::ClipDB(sqlite3* db, sqlite3_int64 skel_id)
	: m_db(db)
	, m_skel_id(skel_id)
	, m_stmt_count_clips(db)
	, m_stmt_get_ids(db)
	, m_stmt_get_clip_info(db)
	, m_stmt_get_all_clip_info(db)
	, m_stmt_remove_clip(db)

{
	PrepareStatements();
}

ClipDB::~ClipDB()
{
}

void ClipDB::PrepareStatements()
{
	m_stmt_count_clips.Init("SELECT count(*) FROM clips WHERE skel_id = ?") ;
	m_stmt_count_clips.BindInt64(1, m_skel_id);

	m_stmt_get_ids.Init("SELECT id FROM clips WHERE skel_id = ?");
	m_stmt_get_ids.BindInt64(1, m_skel_id);

	m_stmt_get_clip_info.Init("SELECT id,name FROM clips WHERE id = ? AND skel_id = ?");
	m_stmt_get_clip_info.BindInt64(2, m_skel_id);

	m_stmt_get_all_clip_info.Init("SELECT id,name FROM clips WHERE skel_id = ?");
	m_stmt_get_all_clip_info.BindInt64(1, m_skel_id);

	m_stmt_remove_clip.Init("DELETE FROM clips WHERE id = ? AND skel_id = ?");
	m_stmt_remove_clip.BindInt64(2, m_skel_id);
}

int ClipDB::GetNumClips() const 
{
	m_stmt_count_clips.Reset();
	if( m_stmt_count_clips.Step() )
		return m_stmt_count_clips.ColInt(0);
	else 
		return 0;
}

void ClipDB::GetClipIDs( std::vector<sqlite3_int64>& out ) const
{
	out.clear();
	m_stmt_get_ids.Reset();
	while(m_stmt_get_ids.Step())
	{
		sqlite3_int64 id = m_stmt_get_ids.ColInt64(0);
		out.push_back(id);
	}	
}

void ClipDB::GetClipInfoBrief ( sqlite3_int64 id, ClipInfoBrief& out) const
{
	m_stmt_get_clip_info.Reset();
	m_stmt_get_clip_info.BindInt64(1, id);
	if(m_stmt_get_clip_info.Step())
	{
		out.id = m_stmt_get_clip_info.ColInt64(0);
		out.name = m_stmt_get_clip_info.ColText(1);
	} else out.id = 0;
}

void ClipDB::GetAllClipInfoBrief( std::vector< ClipInfoBrief >& out) const 
{
	m_stmt_get_all_clip_info.Reset();
	int count = GetNumClips();
	out.clear();
	out.reserve(count);
	while(m_stmt_get_all_clip_info.Step())
	{
		ClipInfoBrief info;
		info.id = m_stmt_get_all_clip_info.ColInt64(0); 
		info.name = m_stmt_get_all_clip_info.ColText(1);
		out.push_back(info);
	}	
}

ClipHandle ClipDB::GetClip( sqlite3_int64 id ) const
{
	ClipHandle clip = new Clip(m_db, id);
	if(clip->Valid()) {
		return clip;
	} else return ClipHandle();
}

void ClipDB::RemoveClip( sqlite3_int64 id )
{
	m_stmt_remove_clip.Reset();
	m_stmt_remove_clip.BindInt64(1, id);
	m_stmt_remove_clip.Step();
}

LBF::WriteNode* createClipsWriteNode( const ClipDB* clips )
{
	LBF::WriteNode* firstClip = 0;
	LBF::WriteNode* curInsert = 0;

	std::vector<sqlite3_int64> ids;
	clips->GetClipIDs(ids);

	int num_clips = ids.size();
	for(int i = 0; i < num_clips; ++i)
	{
		ClipHandle clip = clips->GetClip( ids[i] );
		if(clip->Valid()) {
			LBF::WriteNode* clipNode = clip->CreateClipWriteNode();
			if(firstClip == 0) {
				firstClip = clipNode;
			} else {
				curInsert->AddSibling(clipNode);
			}
			curInsert = clipNode;
		}
	}
	
	return firstClip;
}

bool importClipsFromReadNode( sqlite3* db, sqlite3_int64 skelid, const LBF::ReadNode& rn )
{
	ASSERT(rn.GetType() == LBF::ANIMATION);

	LBF::ReadNode cur =rn;
	while(cur.Valid()) {
		Clip::ImportClipFromReadNode(db, skelid, cur);
		cur = cur.GetNext(LBF::ANIMATION);
	}
	return true;
}

