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
	, m_stmt_get_annotations(db)
	, m_stmt_get_annotation_clip(db)
	, m_stmt_add_annotation(db)
	, m_stmt_remove_annotation(db)
	, m_stmt_get_single_anno(db)
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

	m_stmt_get_annotations.Init("SELECT id,name,fidelity FROM annotations");

	m_stmt_get_annotation_clip.Init("SELECT annotations.id,name,fidelity FROM annotations "
									"INNER JOIN clip_annotations ON annotations.id = annotation_id "
									"WHERE clip_id = ?");

	m_stmt_add_annotation.Init("INSERT INTO annotations (name, fidelity) VALUES (?,99999.0)");
	m_stmt_remove_annotation.Init("DELETE FROM annotations WHERE id = ?");
	m_stmt_get_single_anno.Init("SELECT id,name,fidelity FROM annotations WHERE id = ?");
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

void ClipDB::GetAnnotations( std::vector< Annotation >& out) const 
{
	m_stmt_get_annotations.Reset();
	out.clear();
	while( m_stmt_get_annotations.Step() ) {
		out.push_back(Annotation( m_db, m_stmt_get_annotations.ColInt64(0),
								  m_stmt_get_annotations.ColText(1),
								  m_stmt_get_annotations.ColDouble(2) ));
	}
}

void ClipDB::GetAnnotations( std::vector< Annotation >& out, sqlite3_int64 clip) const 
{
	m_stmt_get_annotation_clip.Reset();
	m_stmt_get_annotation_clip.BindInt64(1, clip);
	out.clear();
	while( m_stmt_get_annotation_clip.Step() ) {
		out.push_back(Annotation( m_db, m_stmt_get_annotation_clip.ColInt64(0),
								  m_stmt_get_annotation_clip.ColText(1),
								  m_stmt_get_annotation_clip.ColDouble(2) ));
	}
}

Annotation ClipDB::AddAnnotation( const char* name ) 
{
	m_stmt_add_annotation.Reset();
	m_stmt_add_annotation.BindText(1, name);
	m_stmt_add_annotation.Step();
	
	if(m_stmt_add_annotation.IsError()) {
		return Annotation();
	}

	sqlite3_int64 id = m_stmt_add_annotation.LastRowID();
	
	m_stmt_get_single_anno.Reset();
	m_stmt_get_single_anno.BindInt64(1, id);
	if(m_stmt_get_single_anno.Step())
	{
		return Annotation(m_db,
						  m_stmt_get_single_anno.ColInt64(0),
						  m_stmt_get_single_anno.ColText(1),
						  m_stmt_get_single_anno.ColDouble(2));
	}
	else return Annotation();
}

void ClipDB::RemoveAnnotation( sqlite3_int64 id )
{
	m_stmt_remove_annotation.Reset();
	m_stmt_remove_annotation.BindInt64(1, id);
	m_stmt_remove_annotation.Step();
}

ClipHandle ClipDB::GetClip( sqlite3_int64 id ) const
{
	ClipHandle clip = new Clip(m_db, id);
	if(clip->Valid()) {
		return clip;
	} else return ClipHandle();
}

bool ClipDB::RemoveClip( sqlite3_int64 id )
{
	m_stmt_remove_clip.Reset();
	m_stmt_remove_clip.BindInt64(1, id);
	m_stmt_remove_clip.Step();

	return !m_stmt_remove_clip.IsError();
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

////////////////////////////////////////////////////////////////////////////////
// Annotation editor helper
Annotation::Annotation( sqlite3* db, sqlite_int64 id, const char* name, float fidelity) 
	: m_db(db), m_id(id), m_name(name), m_fidelity(fidelity)
{
}

void Annotation::SetName( const char* name )
{
	Query update(m_db, "UPDATE annoations SET name = ? WHERE id = ?");
	update.BindText(1,name).BindInt64(2, m_id);
	update.Step();
	if(!update.IsError()) {
		m_name = name;
	}
}

void Annotation::SetFidelity( float fidelity )
{
	Query update(m_db, "UPDATE annoations SET fidelity = ? WHERE id = ?");
	update.BindDouble(1,fidelity).BindInt64(2, m_id);
	update.Step();
	if(!update.IsError()) {
		m_fidelity = fidelity;
	}
}

void Annotation::ApplyToClip( sqlite3_int64 id )
{
	Query apply(m_db, "INSERT INTO clip_annotations (annotation_id, clip_id) VALUES(?,?)");
	apply.BindInt64(1, m_id).BindInt64(2, id);
	apply.Step();
}

void Annotation::RemoveFromClip( sqlite3_int64 id )
{
	Query remove(m_db, "DELETE FROM clip_annotations WHERE annotation_id = ? and clip_id = ?");
	remove.BindInt64(1, m_id).BindInt64(2, id);
	remove.Step();
}
