#ifndef INCLUDED_acclaim_HH
#define INCLUDED_acclaim_HH

#include <vector>
#include <string>
#include "Vector.hh"

namespace AcclaimFormat 
{
	////////////////////////////////////////////////////////////////////////////////
	// parse structures - not really for use except for reading acclaim data.
	////////////////////////////////////////////////////////////////////////////////
	struct DOF {
		enum DOFType {
			TX = 0, TY, TZ, RX, RY, RZ
		};
		DOFType type;
		float low,high;

		DOF(DOFType type) : type(type), low(0.f), high(0.f) {}
	};

	class BoneData;

	struct RootData {
		std::vector< DOF > dofs;
		char axis_order[3];
		Vec3 position;
		Vec3 orientation;
		std::vector< BoneData* > children;
	};

	struct EulerAngles {
		Vec3 angles;
		char axis_order[3];
		EulerAngles() : angles(0,0,0) { 
			axis_order[0] = 'X'; axis_order[1] = 'Y'; axis_order[2] = 'Z';
		}
	};

	struct BoneData {
		int id;
		std::string name;
		Vec3 direction;
		float length;
		EulerAngles axis;
		std::vector< DOF > dofs;
		std::vector< BoneData* > children;
		int parent;
		int index ;

		BoneData() : id(-1), direction(0,0,0), length(0), parent(-1), index(-1) {}
	};

	struct Skeleton
	{
		float scale ; // to convert asf inches to meters, need to multiply lengths by (1/scale) * (2.54/100)
		bool in_deg;
		std::string name;
		RootData root;
		std::vector<BoneData*> bones;	

		Skeleton() 
			: scale(1.f)
			, in_deg(true) {}
		~Skeleton() ;
	};

	struct FrameTransformData {
		float val[6];
		FrameTransformData() { for(int i = 0; i < 6; ++i) val[i] = 0; }
	};

	struct Frame {
		FrameTransformData root_data;
		std::vector< FrameTransformData > data;
		int id; // will be successive for mocap data

		explicit Frame(int num_bones) ;
	};

	struct Clip
	{
		bool in_deg;
		std::vector<Frame*> frames;
		Clip() : in_deg(true) {}
		~Clip();		
	};

	////////////////////////////////////////////////////////////////////////////////
	// parse and create Skeleton/Animation from ASF/AMC files, respectively
	// requires null terminated buffer
	////////////////////////////////////////////////////////////////////////////////
	Skeleton* createSkeletonFromASF( const char* buffer );
	Clip* createClipFromAMC( const char* buffer, const Skeleton* skel);
}


#endif
