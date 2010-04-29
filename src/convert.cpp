#include "convert.hh"
#include "skeleton.hh"
#include "clip.hh"
#include "acclaim.hh"
#include "Quaternion.hh"
#include "MathUtil.hh"

////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////

Quaternion make_quaternion_from_euler( Vec3_arg angles, const char axis_order[3], float angle_factor )
{
	Quaternion q(0,0,0,1); 
	for(int i = 0; i < 3; ++i) {
		if(axis_order[i] == 'X') {
			q = make_rotation(angle_factor * angles[i], Vec3(1,0,0)) * q;
		} else if(axis_order[i] == 'Y') {
			q = make_rotation(angle_factor * angles[i], Vec3(0,1,0)) * q ;
		} else if(axis_order[i] == 'Z') {
			q = make_rotation(angle_factor * angles[i], Vec3(0,0,1)) * q ;
		}
	}	
	return q;
}

Quaternion make_quaternion_from_dofs( const std::vector<AcclaimFormat::DOF> &dofs, 
									  AcclaimFormat::FrameTransformData& data,
									  float angle_factor)
{
	using namespace AcclaimFormat; 
	Quaternion q(0,0,0,1);
	int num_dofs = dofs.size();
	for(int i = 0; i < num_dofs; ++i) {
		float val = angle_factor * data.val[ dofs[i].type ];
		if(dofs[i].type == DOF::RX) {
			q = make_rotation(val, Vec3(1,0,0)) * q ;
		} else if(dofs[i].type == DOF::RY) {
			q = make_rotation(val, Vec3(0,1,0)) * q;
		} else if(dofs[i].type == DOF::RZ) {
			q = make_rotation(val, Vec3(0,0,1)) * q ;
		}
	}
	return q;
}

////////////////////////////////////////////////////////////////////////////////
// Public Conversion functions
////////////////////////////////////////////////////////////////////////////////
Skeleton* convertToSkeleton(const AcclaimFormat::Skeleton* asf)
{
	int num_joints = asf->bones.size();
	float angle_factor = asf->in_deg ? TO_RAD : 1.f;
	float len_factor = (1.0f / asf->scale) * 2.54/100.f; // scaled inchs -> meters

	Skeleton* result = new Skeleton(num_joints);
	result->SetName( asf->name.c_str() );
	result->SetRootOffset( asf->root.position );
	result->SetRootRotation( make_quaternion_from_euler( asf->root.orientation, asf->root.axis_order, angle_factor ) );

	for(int i = 0; i < num_joints; ++i)
	{
		Vec3 offset = (asf->bones[i]->direction) * len_factor * asf->bones[i]->length;
		result->GetJointTranslation(i) = offset;
		result->SetJointName( i, asf->bones[i]->name.c_str() );
		result->SetJointParent( i, asf->bones[i]->parent );
	}
	
	return result;
}

Clip* convertToClip(const AcclaimFormat::Clip* amc, const AcclaimFormat::Skeleton* skel, float fps)
{

	int num_joints = skel->bones.size();
	float skel_angle_factor = skel->in_deg ? TO_RAD : 1.f;
	float angle_factor = amc->in_deg ? TO_RAD : 1.f;
	float len_factor = (1.0 / skel->scale) * 2.54 / 100.f; // scaled inches -> meters
	
	int num_frames = amc->frames.size();
	Clip* result = new Clip(num_joints, num_frames, fps);
	for(int frm = 0; frm < num_frames; ++frm)
	{
		using namespace AcclaimFormat ;
		Vec3 root_offset( len_factor * amc->frames[frm]->root_data.val[DOF::TX],
						  len_factor * amc->frames[frm]->root_data.val[DOF::TY],
						  len_factor * amc->frames[frm]->root_data.val[DOF::TZ]);

		Quaternion root_quaternion = make_quaternion_from_dofs( skel->root.dofs, amc->frames[frm]->root_data, angle_factor );		
		result->GetFrameRootOffset(frm) = root_offset;
		result->GetFrameRootOrientation(frm) = root_quaternion;
		Quaternion* q = result->GetFrameRotations(frm);
		for(int bone = 0; bone < num_joints; ++bone)
		{
			Quaternion bone_axis_q = make_quaternion_from_euler( skel->bones[bone]->axis.angles, skel->bones[bone]->axis.axis_order, skel_angle_factor );			
			Quaternion motion_q = make_quaternion_from_dofs( skel->bones[bone]->dofs, amc->frames[frm]->data[bone], angle_factor );
			q[bone] = bone_axis_q * motion_q * conjugate(bone_axis_q);
		}
	}
	return result;
}
