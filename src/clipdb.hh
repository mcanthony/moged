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
	int num_frames;
	int is_transition;
};

// annotation editing helper class
class Annotation {
	sqlite3* m_db;
	sqlite3_int64 m_id;

	std::string m_name;
	float m_fidelity;
public:
	Annotation() : m_db(0), m_id(0), m_fidelity(0.f){}
	Annotation( sqlite3* db, sqlite3_int64 id, const char* name, float fidelity) ;
	bool Valid() const { return m_id != 0; }
	sqlite3_int64 GetID() { return m_id; }
	bool SetName( const char* name );
	const char* GetName() const { return m_name.c_str(); }
	void SetFidelity( float fidelity );
	float GetFidelity( ) const { return m_fidelity; }	
	void ApplyToClip( sqlite3_int64 id );
	void RemoveFromClip( sqlite3_int64 id );
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
	mutable Query m_stmt_get_annotations;
	mutable Query m_stmt_get_annotation_clip;
	mutable Query m_stmt_add_annotation;
	mutable Query m_stmt_remove_annotation;
	mutable Query m_stmt_get_single_anno;
public:
	ClipDB(sqlite3* db, sqlite3_int64 skel_id);
	~ClipDB();

	int GetNumClips() const ;
	void GetClipIDs( std::vector<sqlite3_int64>& out ) const;
	void GetClipInfoBrief ( sqlite3_int64 id , ClipInfoBrief &out) const;
	void GetAllClipInfoBrief( std::vector< ClipInfoBrief >& out, bool includeOriginals = true , bool includeTransitions = false) const ;
	
	ClipHandle GetClip( sqlite3_int64 id ) const;
	bool RemoveClip( sqlite3_int64 id );

	void GetAnnotations( std::vector< Annotation >& out) const ;
	void GetAnnotations( std::vector< Annotation >& out, sqlite3_int64 clip) const ;
	Annotation AddAnnotation( const char* name ) ;
	void RemoveAnnotation( sqlite3_int64 id );

private: 
	void PrepareStatements();
};

namespace LBF { class WriteNode; class ReadNode; }
LBF::WriteNode* createClipsWriteNode( const ClipDB* clips );
bool importClipsFromReadNode( sqlite3* db, sqlite3_int64 skelid, const LBF::ReadNode& rn );

#endif
