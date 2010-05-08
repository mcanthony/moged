#include <cstring>
#include <cstdio>
#include <string>
#include "sql/sqlite3.h"
#include "assert.hh"
#include "dbhelpers.hh"
#include "Vector.hh"  
#include "Quaternion.hh"

using namespace std;

int sql_bind_vec3( sqlite3_stmt *stmt, int start_col, Vec3_arg v)
{
	int err = 0;
	err = sqlite3_bind_double(stmt, start_col, v.x);
	if(err != SQLITE_OK) return err;
	err = sqlite3_bind_double(stmt, start_col+1, v.y);
	if(err != SQLITE_OK) return err;
	err = sqlite3_bind_double(stmt, start_col+2, v.z);
	return err;
}

int sql_bind_quaternion( sqlite3_stmt *stmt, int start_col, Quaternion_arg q)
{
	int err = 0;
	err = sqlite3_bind_double(stmt, start_col, q.a);
	if(err != SQLITE_OK) return err;
	err = sqlite3_bind_double(stmt, start_col+1, q.b);
	if(err != SQLITE_OK) return err;
	err = sqlite3_bind_double(stmt, start_col+2, q.c);
	if(err != SQLITE_OK) return err;
	err = sqlite3_bind_double(stmt, start_col+3, q.r);
	return err;
}

int sql_begin_transaction( sqlite3 *db )
{
	return sqlite3_exec(db, "BEGIN TRANSACTION",NULL, NULL, NULL);
}

int sql_end_transaction( sqlite3 *db )
{
	return sqlite3_exec(db, "END TRANSACTION",NULL, NULL, NULL);
}

int sql_rollback_transaction( sqlite3 *db )
{
	return sqlite3_exec(db, "ROLLBACK TRANSACTION",NULL, NULL, NULL);
}

////////////////////////////////////////////////////////////////////////////////
// query helper class
Query::Query(sqlite3* db)
	: m_db(db)
	, m_stmt(0)
	, m_err(0)
	, m_quiet(false)
{
	
}

Query::Query(sqlite3* db, const char* text)
	: m_db(db)
	, m_stmt(0)
	, m_err(0)
	, m_quiet(false)
{
	m_err = sqlite3_prepare_v2(db, text, -1, &m_stmt, 0);
	if(m_err != SQLITE_OK) { PrintError(text); }
}

Query::~Query()
{
	sqlite3_finalize(m_stmt);
}

void Query::Init(const char* text)
{
	ASSERT(m_stmt == 0);
	m_err = sqlite3_prepare_v2(m_db, text, -1, &m_stmt, 0);
	if(m_err != SQLITE_OK) { PrintError(text); }	
}

void Query::PrintError(const char* extra) const
{
	if(m_quiet && m_err == SQLITE_CONSTRAINT) return;
	fprintf(stderr, "SQL Error: %s\n", sqlite3_errmsg(m_db));
	const char* sql = sqlite3_sql( m_stmt );
	if(sql) fprintf(stderr, "SQL statement: %s\n", sql);
	if(extra) {
		fprintf(stderr, "Text: %s\n", extra);
	}
	DEBUGBREAK(); // debug break here so we can do stuff
}

bool Query::IsError() const 
{
	if(m_err != SQLITE_OK && m_err != SQLITE_DONE && m_err != SQLITE_ROW)
		return true;
	return false;
}

void Query::Reset()
{
	m_err = sqlite3_reset(m_stmt);
	if(m_err != SQLITE_OK)
		PrintError();
}

Query& Query::BindInt64(int col, sqlite3_int64 v)
{
	ASSERT(col >= 1);
	m_err = sqlite3_bind_int64(m_stmt, col, v);
	if(m_err != SQLITE_OK) PrintError();
	return *this;
}

Query& Query::BindInt(int col, int v)
{
	ASSERT(col >= 1);
	m_err = sqlite3_bind_int(m_stmt, col, v);
	if(m_err != SQLITE_OK) PrintError();
	return *this;
}

Query& Query::BindText(int col, const char* text)
{
	ASSERT(col >= 1);
	m_err = sqlite3_bind_text(m_stmt, col, text, -1, SQLITE_STATIC);
	if(m_err != SQLITE_OK) PrintError();
	return *this;
}

Query& Query::BindDouble(int col, double v)
{
	ASSERT(col >= 1);
	m_err = sqlite3_bind_double(m_stmt, col, v);
	if(m_err != SQLITE_OK) PrintError();
	return *this;
}

Query& Query::BindVec3(int col, Vec3_arg v)
{
	ASSERT(col >= 1);
	m_err = sql_bind_vec3(m_stmt, col, v);
	if(m_err != SQLITE_OK) PrintError();
	return *this;
}

Query& Query::BindQuaternion(int col, Quaternion_arg q)
{
	ASSERT(col >= 1);
	m_err = sql_bind_quaternion(m_stmt, col, q);
	if(m_err != SQLITE_OK) PrintError();
	return *this;
}

Query& Query::BindBlob(int col, void* p, int num_bytes)
{
	ASSERT(col >= 1);
	m_err = sqlite3_bind_blob(m_stmt, col, p, num_bytes, SQLITE_STATIC);
	if(m_err != SQLITE_OK) PrintError();
	return *this;
}

bool Query::Step() 
{
	m_err = sqlite3_step(m_stmt);
	if(m_err == SQLITE_ROW) return true;
	else if(m_err != SQLITE_DONE) {
		PrintError();
	}
	return false;
}

sqlite3_int64 Query::LastRowID() const
{
	if(IsError()) return 0;
	return sqlite3_last_insert_rowid(m_db);
}
	
sqlite3_int64 Query::ColInt64(int col)
{
	ASSERT(col >= 0);
	return sqlite3_column_int64(m_stmt, col);
}

int Query::ColInt(int col)
{
	ASSERT(col >= 0);
	return sqlite3_column_int(m_stmt, col);
}

const char* Query::ColText(int col)
{
	ASSERT(col >= 0);
	return (const char*)sqlite3_column_text(m_stmt, col);
}

double Query::ColDouble(int col)
{
	ASSERT(col >= 0);
	return sqlite3_column_double(m_stmt, col);
}

Vec3 Query::ColVec3(int col)
{
	ASSERT(col >= 0);
	float x = 0, y = 0, z = 0;
	x = sqlite3_column_double(m_stmt, col);
	y = sqlite3_column_double(m_stmt, col+1);
	z = sqlite3_column_double(m_stmt, col+2);
	return Vec3(x,y,z);
}

Quaternion Query::ColQuaternion(int col)
{
	ASSERT(col >= 0);
	float a = 0, b = 0, c = 0, r = 0;
	a = sqlite3_column_double(m_stmt, col);
	b = sqlite3_column_double(m_stmt, col+1);
	c = sqlite3_column_double(m_stmt, col+2);
	r = sqlite3_column_double(m_stmt, col+3);
	return Quaternion(a,b,c,r);
}

const void* Query::ColBlob(int col)
{
	ASSERT(col >= 0);
	return sqlite3_column_blob(m_stmt, col);
}

void Query::PrintSQL() const
{
	printf("%s\n", sqlite3_sql(m_stmt));
}

////////////////////////////////////////////////////////////////////////////////
SavePoint::SavePoint(sqlite3* db, const char* name) 
	: m_stmt_release(db)
	, m_stmt_rollback(db)
{
	string start_str = "SAVEPOINT " ; 
	start_str += name;

	string release_str = "RELEASE SAVEPOINT ";
	release_str += name;
	string rollback_str = "ROLLBACK TO SAVEPOINT " ;
	rollback_str += name;
	m_stmt_release.Init(release_str.c_str());
	m_stmt_rollback.Init(rollback_str.c_str());

	Query saveStart(db, start_str.c_str());
	saveStart.Step();
//	saveStart.PrintSQL();
}

SavePoint::~SavePoint()
{
//	m_stmt_release.PrintSQL();
	m_stmt_release.Step();
}

void SavePoint::Rollback()
{
//	m_stmt_rollback.PrintSQL();
	m_stmt_rollback.Step();
}
