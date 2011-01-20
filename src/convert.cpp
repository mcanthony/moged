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
	Quaternion root_q = make_quaternion_from_euler( asf->root.orientation, asf->root.axis_order, angle_factor ) ;

	Transaction trans(db);

	// insert initial skeleton entry
	Query insert_skel(db, "INSERT INTO skeleton ( name, root_offset, root_rotation, num_joints,"
		"translations, parents, weights) VALUES (?, ?, ?, ?, ?, ?, ?)");
	insert_skel.BindText(1, asf->name.c_str());
	insert_skel.BindBlob(2, &root_t, sizeof(Vec3));
	insert_skel.BindBlob(3, &root_q, sizeof(Quaternion));
	insert_skel.BindInt(4, num_joints);
	insert_skel.BindBlob(5, num_joints * sizeof(Vec3));
	insert_skel.BindBlob(6, num_joints * sizeof(int));
	insert_skel.BindBlob(7, num_joints * sizeof(float));
	insert_skel.Step();
	if( insert_skel.IsError() ) {
		return 0;
	}
		
	sqlite3_int64 new_skel_id = insert_skel.LastRowID();

	{
		Blob transWriter(db, "skeleton", "translations", new_skel_id, true);
		Blob parentsWriter(db, "skeleton", "parents", new_skel_id, true);
		Blob weightsWriter(db, "skeleton", "weights", new_skel_id, true);
		int transOffset = 0;
		int parentsOffset = 0;
		int weightsOffset = 0; 
		const float kWeight = 1.0;
		for(int i = 0; i < num_joints; ++i)
		{
			int parent = asf->bones[i]->parent;
			Vec3 offset = (asf->bones[i]->direction) * len_factor * asf->bones[i]->length;

			transWriter.Write(&offset, sizeof(offset), transOffset);
			transOffset += sizeof(offset);

			parentsWriter.Write(&parent, sizeof(parent), parentsOffset);
			parentsOffset += sizeof(parent);

			weightsWriter.Write(&kWeight, sizeof(kWeight), weightsOffset);
			weightsOffset += sizeof(kWeight);
			
		}

		Query insert_joints(db,
			"INSERT INTO skeleton_joints (skel_id, offset, name) "
			"VALUES (:skel_id, :offset, :name)");
		insert_joints.BindInt64(1, new_skel_id);
		for(int i = 0; i < num_joints; ++i)
		{
			insert_joints.Reset();
			insert_joints.BindInt(2, i).BindText(3, asf->bones[i]->name.c_str());
			insert_joints.Step();
		}
	}

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
	const int num_frames = amc->frames.size();

	Transaction transaction(db);
	
	Query insert_clip(db, "INSERT INTO clips (skel_id, name, fps,"
		"num_frames, frames) "
		"VALUES (?,?,?,?,?)");
	insert_clip.BindInt64(1, skel_id).BindText(2, name).BindDouble(3, fps);
	insert_clip.BindInt64(4, num_frames);
	insert_clip.BindBlob(5, sizeof(Quaternion) * num_frames * num_joints + 
		sizeof(ClipFrameHeader) * num_frames);
	insert_clip.Step();
	if(insert_clip.IsError()) {
		transaction.Rollback();
		return 0;
	}

	sqlite3_int64 new_clip_id = insert_clip.LastRowID();

	Blob framesWriter(db, "clips", "frames", new_clip_id, true);
	int framesOffset = 0;

	for(int frm = 0; frm < num_frames; ++frm)
	{
		using namespace AcclaimFormat ;
		ClipFrameHeader header;

		header.root_offset = Vec3( len_factor * amc->frames[frm]->root_data.val[DOF::TX],
				len_factor * amc->frames[frm]->root_data.val[DOF::TY],
				len_factor * amc->frames[frm]->root_data.val[DOF::TZ]);	   
		header.root_quaternion = make_quaternion_from_dofs( skel->root.dofs, 
			amc->frames[frm]->root_data, angle_factor );

		framesWriter.Write(&header, sizeof(header), framesOffset);
		framesOffset += sizeof(header);

		for(int bone = 0; bone < num_joints; ++bone)
		{
			Quaternion bone_axis_q = make_quaternion_from_euler( skel->bones[bone]->axis.angles, skel->bones[bone]->axis.axis_order, skel_angle_factor );
			Quaternion motion_q = make_quaternion_from_dofs( skel->bones[bone]->dofs, amc->frames[frm]->data[bone], angle_factor );
			Quaternion final_q = bone_axis_q * motion_q * conjugate(bone_axis_q);

			framesWriter.Write(&final_q, sizeof(final_q), framesOffset);
			framesOffset += sizeof(final_q);
		}
	}

	return new_clip_id;
}
