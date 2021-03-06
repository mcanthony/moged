#include <cstring>
#include <cstdio>
#include <string>
#include <unistd.h>
#include "sql/sqlite3.h"
#include "assert.hh"
#include "dbhelpers.hh"
#include "Vector.hh"  
#include "Quaternion.hh"

using namespace std;

const int gSuperVerbose = 0;

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
	if(gSuperVerbose >= 1) {
		printf("BEGIN TRANSACTION\n");
	}
	return sqlite3_exec(db, "BEGIN TRANSACTION",NULL, NULL, NULL);
}

int sql_end_transaction( sqlite3 *db )
{
	if(gSuperVerbose >= 1) {
		printf("END TRANSACTION\n");
	}
	return sqlite3_exec(db, "END TRANSACTION",NULL, NULL, NULL);
}

int sql_rollback_transaction( sqlite3 *db )
{
	if(gSuperVerbose >= 1) {
		printf("ROLLBACK TRANSACTION\n");
	}
	return sqlite3_exec(db, "ROLLBACK TRANSACTION",NULL, NULL, NULL);
}

////////////////////////////////////////////////////////////////////////////////
// query helper class
Query::Query(sqlite3* db)
	: m_db(db)
	, m_stmt(0)
	, m_err(0)
	, m_quiet(false)
	, m_ignore_busy(false)
{
	
}

Query::Query(sqlite3* db, const char* text)
	: m_db(db)
	, m_stmt(0)
	, m_err(0)
	, m_quiet(false)
	, m_ignore_busy(false)
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
	if(m_ignore_busy && m_err == SQLITE_BUSY) return;
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

Query& Query::BindBlob(int col, const void* p, int num_bytes)
{
	ASSERT(col >= 1);
	m_err = sqlite3_bind_blob(m_stmt, col, p, num_bytes, SQLITE_STATIC);
	if(m_err != SQLITE_OK) PrintError();
	return *this;
}

Query& Query::BindBlob(int col, int num_bytes)
{
	ASSERT(col >= 1);
	m_err = sqlite3_bind_zeroblob(m_stmt, col, num_bytes);
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
	if(gSuperVerbose >= 2) {
		printf("%s\n", sqlite3_sql(m_stmt));
	}
	return false;
}

sqlite3_int64 Query::LastRowID() const
{
	if(IsError()) return 0;
	return sqlite3_last_insert_rowid(m_db);
}

int Query::NumChanged() const 
{
	return sqlite3_changes(m_db);
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

Vec3 Query::ColVec3FromBlob(int col)
{
	ASSERT(col >= 0);
	Vec3 result;
	const void* src = sqlite3_column_blob(m_stmt, col);
	if(src) 
		memcpy(&result.x, src, sizeof(Vec3));
    else return Vec3(0,0,0);
	return result;
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

Quaternion Query::ColQuaternionFromBlob(int col)
{
	ASSERT(col >= 0);
	Quaternion result;
	const void* src = sqlite3_column_blob(m_stmt, col);
	if(src) 
		memcpy(&result.a, src, sizeof(Quaternion));
    else return Quaternion(0,0,0,1);
	return result;
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
	m_stmt_rollback.SetIgnoreBusy(); 

	Query saveStart(db, start_str.c_str());
	saveStart.Step();
	if(gSuperVerbose == 1) saveStart.PrintSQL();
}

SavePoint::~SavePoint()
{
	if(gSuperVerbose == 1) m_stmt_release.PrintSQL();
	m_stmt_release.Step();
}

void SavePoint::Rollback()
{
	if(gSuperVerbose == 1) m_stmt_rollback.PrintSQL();
	m_stmt_rollback.Step();
}

////////////////////////////////////////////////////////////////////////////////
Blob::Blob(sqlite3* db, 
	const char* table, 
	const char* column,
	sqlite3_int64 row,
	bool write,
	const char* dbName)
	: m_db(db)
	, m_blob(0)
{
	int status = sqlite3_blob_open(db, dbName, table, column, row, write ? 1 : 0, &m_blob);
	if(status != SQLITE_OK) {
		const char* strError = sqlite3_errmsg(m_db);
		fprintf(stderr, "Failed to open blob %s.%s (%s): %s!\n", dbName, table, column, strError);
		Close();
	}
}

Blob::~Blob()
{
	Close();
}

bool Blob::Write(const void* data, int n, int offset)
{
	if(!m_blob) return false;
	int status = sqlite3_blob_write(m_blob, data, n, offset) ;
	if(status != SQLITE_OK)
	{
		const char* strError = sqlite3_errmsg(m_db);
		fprintf(stderr, "Failed to write to blob (%p) (error: %d: %s). Writing %d bytes at offset %d, blob size is %d \n", m_blob, status, strError, n, offset,
			sqlite3_blob_bytes(m_blob));	
	}
	return status == SQLITE_OK;
}

bool Blob::Read(void* data, int n, int offset)
{
	if(!m_blob) return false;
	int status = sqlite3_blob_read(m_blob, data, n, offset) ;
	if(status != SQLITE_OK)
	{
		const char* strError = sqlite3_errmsg(m_db);
		fprintf(stderr, "Failed to read from blob (%p) (error: %d: %s). Reading %d bytes at offset %d, blob size is %d \n", m_blob, status, strError, n, offset,
			sqlite3_blob_bytes(m_blob));	
	}
	return status == SQLITE_OK;
}

int Blob::GetSize() 
{
	if(m_blob) 
		return sqlite3_blob_bytes(m_blob);
	else return 0;
}

void Blob::Close()
{
	if(m_blob) {
		sqlite3_blob_close(m_blob);
	}
	m_blob = 0;
}



