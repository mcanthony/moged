#ifndef INCLUDED_convert_HH
#define INCLUDED_convert_HH

class Skeleton;
class Clip;

#include "dbhelpers.hh"

namespace AcclaimFormat {
	class Skeleton;
	class Clip;
}

////////////////////////////////////////////////////////////////////////////////
// convert parsed acclaim files to sql entries
////////////////////////////////////////////////////////////////////////////////
sqlite3_int64 convertToSkeleton(sqlite3* db, const AcclaimFormat::Skeleton* asf);
sqlite3_int64 convertToClip(sqlite3* db, sqlite3_int64 skel_id, 
							const AcclaimFormat::Clip* amc, 
							const AcclaimFormat::Skeleton* skel, const char* name, float fps);


#endif
