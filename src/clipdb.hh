#ifndef INCLUDED_moged_clipdb_HH
#define INCLUDED_moged_clipdb_HH

struct sqlite3;

#include <string>
#include <vector>
#include "dbhelpers.hh"
#include "intrusive_ptr.hh"

class Clip;
typedef reference<Clip> ClipHandle;

struct ClipInfoBrief {
	std::string name;
	sqlite3_int64 id;
};

class ClipDB
{
	sqlite3* m_db;
	sqlite3_int64 m_skel_id;

	mutable Query m_stmt_count_clips;
	mutable Query m_stmt_get_ids;
	mutable Query m_stmt_get_clip_info;
	mutable Query m_stmt_get_all_clip_info;
	mutable Query m_stmt_remove_clip;

public:
	ClipDB(sqlite3* db, sqlite3_int64 skel_id);
	~ClipDB();

	int GetNumClips() const ;
	void GetClipIDs( std::vector<sqlite3_int64>& out ) const;
	void GetClipInfoBrief ( sqlite3_int64 id , ClipInfoBrief &out) const;
	void GetAllClipInfoBrief( std::vector< ClipInfoBrief >& out) const ;
	
	ClipHandle GetClip( sqlite3_int64 id ) const;
	void RemoveClip( sqlite3_int64 id );

private: 
	void PrepareStatements();
};

namespace LBF { class WriteNode; class ReadNode; }
LBF::WriteNode* createClipsWriteNode( const ClipDB* clips );
bool importClipsFromReadNode( sqlite3* db, sqlite3_int64 skelid, const LBF::ReadNode& rn );

#endif
