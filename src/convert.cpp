#include <cstdio>
#include "sql/sqlite3.h"
#include "dbhelpers.hh"
#include "convert.hh"
#include "skeleton.hh"
#include "clip.hh"
#include "acclaim.hh"
#include "Quaternion.hh"
#include "Mat4.hh"
#include "MathUtil.hh"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////

static Quaternion make_quaternion_from_euler( Vec3_arg angles, const char axis_order[3], float angle_factor )
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

static Quaternion make_quaternion_from_dofs( const std::vector<AcclaimFormat::DOF> &dofs, 
									  const AcclaimFormat::FrameTransformData& data,
									  float angle_factor)
{
	using namespace AcclaimFormat; 
	Quaternion q(0,0,0,1);
	const int num_dofs = dofs.size();
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
sqlite3_int64 convertToSkeleton(sqlite3 *db, const AcclaimFormat::Skeleton* asf)
{
	int num_joints = asf->bones.size();
	float angle_factor = asf->in_deg ? TO_RAD : 1.f;
	float len_factor = (1.0f / asf->scale) * 2.54/100.f; // scaled inchs -> meters

	Vec3 root_t = len_factor * asf->root.position ;
	Quaternion root_q= make_quaternion_from_euler( asf->root.orientation, asf->root.axis_order, angle_factor ) ;

	sql_begin_transaction(db);
	
	// insert initial skeleton entry
	Query insert_skel(db, "INSERT INTO skeleton ( name, "
					  "root_offset_x, root_offset_y, root_offset_z, "
					  "root_rotation_a, root_rotation_b, root_rotation_c, root_rotation_r ) "
					  "VALUES (?, ?,?,?, ?,?,?,?)");
	insert_skel.BindText(1, asf->name.c_str()).BindVec3(2, root_t).BindQuaternion(5, root_q);
	insert_skel.Step();
	if( insert_skel.IsError() ) {
		sql_rollback_transaction(db);
		return 0;
	}
		
	sqlite3_int64 new_skel_id = insert_skel.LastRowID();

	Query insert_joints(db,
						"INSERT INTO skeleton_joints (skel_id, parent_id, offset, "
						"name, t_x, t_y, t_z, weight ) "
						"VALUES (:skel_id, :parent_id, :offset, :name, :t_x, :t_y, :t_z, 1.0 )");
	insert_joints.BindInt64(1, new_skel_id);
	for(int i = 0; i < num_joints; ++i)
	{
		insert_joints.Reset();

		int parent = asf->bones[i]->parent;
		Vec3 offset = (asf->bones[i]->direction) * len_factor * asf->bones[i]->length;

		insert_joints.BindInt(2, parent).BindInt(3, i).BindText(4, asf->bones[i]->name.c_str())
			.BindVec3(5, offset);
		insert_joints.Step();
	}

	sql_end_transaction(db);
	
	return new_skel_id;
}

sqlite3_int64 convertToClip(sqlite3* db, sqlite3_int64 skel_id, 
							const AcclaimFormat::Clip* amc, const AcclaimFormat::Skeleton* skel, 
							const char* name, float fps)
{
	const int num_joints = skel->bones.size();
	const float skel_angle_factor = skel->in_deg ? TO_RAD : 1.f;
	const float angle_factor = amc->in_deg ? TO_RAD : 1.f;
	const float len_factor = (1.0 / skel->scale) * 2.54 / 100.f; // scaled inches -> meters

	sql_begin_transaction(db);
	
	Query insert_clip(db, "INSERT INTO clips (skel_id, name, fps) VALUES (?,?,?)");
	insert_clip.BindInt64(1, skel_id).BindText(2, name).BindDouble(3, fps);
	insert_clip.Step();
	if(insert_clip.IsError()) {
		sql_rollback_transaction(db);
		return 0;
	}
	sqlite3_int64 new_clip_id = insert_clip.LastRowID();

	Query insert_frame(db, "INSERT INTO frames(clip_id, num, "
					   "root_offset_x,  root_offset_y,  root_offset_z,  "
					   "root_rotation_a, root_rotation_b, root_rotation_c, root_rotation_r) "
					   "VALUES(?, ?, ?,?,?, ?,?,?,?) ");

	Query insert_rots(db, "INSERT INTO frame_rotations(frame_id, skel_id, joint_offset, "
					  "q_a, q_b, q_c, q_r) VALUES (?, ?,?, ?,?,?,?)");
	
	insert_frame.BindInt64( 1, new_clip_id );
	insert_rots.BindInt64(2, skel_id);

	const int num_frames = amc->frames.size();
	for(int frm = 0; frm < num_frames; ++frm)
	{
		using namespace AcclaimFormat ;
		Vec3 root_offset( len_factor * amc->frames[frm]->root_data.val[DOF::TX],
						  len_factor * amc->frames[frm]->root_data.val[DOF::TY],
						  len_factor * amc->frames[frm]->root_data.val[DOF::TZ]);	   

		Quaternion root_quaternion = make_quaternion_from_dofs( skel->root.dofs, amc->frames[frm]->root_data, angle_factor );
		
		insert_frame.Reset();
		insert_frame.BindInt(2, frm).BindVec3(3, root_offset).BindQuaternion(6, root_quaternion);
		insert_frame.Step();
		
		if(insert_frame.IsError()) {
			sql_rollback_transaction(db);
			return 0;
		}
		sqlite3_int64 frame_id = insert_frame.LastRowID();

		insert_rots.Reset();
		insert_rots.BindInt64(1, frame_id);
		for(int bone = 0; bone < num_joints; ++bone)
		{
			insert_rots.Reset();
			Quaternion bone_axis_q = make_quaternion_from_euler( skel->bones[bone]->axis.angles, skel->bones[bone]->axis.axis_order, skel_angle_factor );
			Quaternion motion_q = make_quaternion_from_dofs( skel->bones[bone]->dofs, amc->frames[frm]->data[bone], angle_factor );
			Quaternion final_q = bone_axis_q * motion_q * conjugate(bone_axis_q);

			insert_rots.BindInt64(3, bone).BindQuaternion(4, final_q);
			insert_rots.Step();
		}
	}

	sql_end_transaction(db);

	return new_clip_id;
}
