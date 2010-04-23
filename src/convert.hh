#ifndef INCLUDED_convert_HH
#define INCLUDED_convert_HH

class Skeleton;
class Clip;

namespace AcclaimFormat {
	class Skeleton;
	class Clip;
}

////////////////////////////////////////////////////////////////////////////////
// convert parsed acclaim files to common skeleton/clip format
////////////////////////////////////////////////////////////////////////////////
Skeleton* convertToSkeleton(const AcclaimFormat::Skeleton* asf);
Clip* convertToClip(const AcclaimFormat::Clip* amc, const AcclaimFormat::Skeleton* skel, float fps);


#endif
