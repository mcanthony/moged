#ifndef INCLUDED_dbhelpers_HH
#define INCLUDED_dbhelpers_HH

struct sqlite3 ;
struct sqlite3_stmt;
typedef long long int sqlite3_int64;

#include "Vector.hh"  // TODO: replace with types.hh that just has Vec3_arg/etc
#include "Quaternion.hh" // TODO: same as above

int sql_bind_vec3( sqlite3_stmt *stmt, int start_col, Vec3_arg v);
int sql_bind_quaternion( sqlite3_stmt *stmt, int start_col, Quaternion_arg q);

int sql_begin_transaction( sqlite3 *db );
int sql_end_transaction( sqlite3 *db );
int sql_rollback_transaction( sqlite3 *db );

////////////////////////////////////////////////////////////////////////////////
// transaction scoped helper class
class Transaction {
	sqlite3* m_db;
	bool m_rollback;
public:
	Transaction(sqlite3* db) : m_db(db), m_rollback(false) { sql_begin_transaction(db); }
	void Rollback() { m_rollback = true; sql_rollback_transaction(m_db); }
	~Transaction() { if(!m_rollback) sql_end_transaction(m_db); }
};


////////////////////////////////////////////////////////////////////////////////
// query helper class
class Query {
	sqlite3 *m_db; // needed to print errors
	sqlite3_stmt* m_stmt;
	int m_err;
	bool m_quiet;

	void PrintError(const char* extra=0) const;
	Query(const Query&);
	Query& operator=(const Query&);
public:
	Query(sqlite3* db);
	Query(sqlite3* db, const char* text);
	~Query();
	
	void Init(const char* text);

	// supress stderr output for certain sql errors (constraint violation currently)
	void SetQuiet() { m_quiet = true; }

	int GetError() const { return m_err; }
	bool IsError() const ;

	void Reset();
	void PrintSQL() const ;

	Query& BindInt64(int col, sqlite3_int64 v);
	Query& BindInt(int col, int v);
	Query& BindText(int col, const char* text);
	Query& BindDouble(int col, double v);
	Query& BindVec3(int col, Vec3_arg v);
	Query& BindQuaternion(int col, Quaternion_arg q);
	Query& BindBlob(int col, void*, int num_bytes);

	bool Step() ;
	sqlite3_int64 LastRowID() const;
	
	sqlite3_int64 ColInt64(int col);
	int ColInt(int col);
	const char* ColText(int col);
	double ColDouble(int col);
	Vec3 ColVec3(int col);
	Quaternion ColQuaternion(int col);
	const void* ColBlob(int col);
};

////////////////////////////////////////////////////////////////////////////////
// SavePoint wrapping sql savepoint/release/rollback stuff.
class SavePoint {
	Query m_stmt_release;
	Query m_stmt_rollback;
public:
	SavePoint(sqlite3* db, const char* name) ;
	~SavePoint();
	void Rollback();
};

#endif
