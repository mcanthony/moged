#include "blendcontroller.hh"
#include "math_util.hh"
#include "anim/pose.hh"

BlendController::BlendController( const Skeleton* skel )
	: AnimController(skel)
	, m_from(0)
	, m_to(0)
	, m_blendTime(1.f)
	, m_curTime(0.f)
{
}

void BlendController::SetFrom( AnimController* controller )
{
	m_from = controller;
}

void BlendController::SetTo( AnimController* controller )
{
	m_to = controller;
}

void BlendController::SetBlendLength( float time_in_s)
{
	m_blendTime = time_in_s ;
	m_curTime = Clamp(m_curTime, 0.f, m_blendTime);
}

// changing this without changing from/to will set the blend to a particular point, but will not 
// fast forward or rewind the from/to clips. Perhaps everything should be going through UpdateTime?
void BlendController::SetCurrentTime( float curtime )
{
	m_curTime = Clamp(curtime, 0.f, m_blendTime);
}

void BlendController::UpdateTime(float dt)
{
    if(m_from == 0 || m_to == 0) return;
	SetCurrentTime(m_curTime + dt);
    m_from->UpdateTime(dt);
    m_to->UpdateTime(dt);
}

float BlendController::GetCurrentTime() const
{
    return m_blendTime;
}

bool BlendController::IsComplete() const 
{
	return m_curTime >= m_blendTime;
}
		
void BlendController::ComputePose( ) 
{
    if(m_from && m_to)
    {
    	float t = Clamp(m_curTime / m_blendTime, 0.f, 1.f);
	    float blendParam = ComputeCubicBlendParam(t);
        float oneMinusBlendParam = 1.0f - blendParam;
	    const Quaternion* from_rotations = m_from->GetPose()->GetRotations();
    	const Quaternion* to_rotations = m_to->GetPose()->GetRotations();
        Quaternion* outRotations = m_pose->GetRotations();

        m_from->ComputePose();
        m_to->ComputePose();

        m_pose->SetRootOffset( 
            blendParam * m_from->GetPose()->GetRootOffset() + 
            oneMinusBlendParam * m_to->GetPose()->GetRootOffset());
        Quaternion rootRot;
        slerp_rotation(rootRot, 
            m_from->GetPose()->GetRootRotation(), 
            m_to->GetPose()->GetRootRotation(), 
            oneMinusBlendParam);
        m_pose->SetRootRotation(rootRot);
				
        const int numJoints = m_pose->GetNumJoints();
		for(int joint = 0; joint < numJoints; ++joint)
		{
			slerp_rotation(outRotations[joint], 
                from_rotations[joint],
				to_rotations[joint], 
                oneMinusBlendParam);
		}
    }
    else
    {
        m_pose->RestPose( m_skel );
    }
}

////////////////////////////////////////////////////////////////////////////////
// Old blend code, need to make this run time

/*
static void blendClips(const Skeleton *skel,
					   const Clip* from, const Clip* to, 
					   float from_start, float to_start,
					   int num_frames, float sample_interval,
					   Vec3_arg align_translation, 
					   float align_rotation,
					   std::vector< Vec3 >& root_translations,
					   std::vector< Quaternion >& root_rotations,
					   std::vector< Quaternion >& frame_rotations )
{
	root_translations.clear();
	root_rotations.clear();
	frame_rotations.clear();

	const int num_joints = skel->GetNumJoints();
	root_translations.resize(num_frames);
	root_rotations.resize(num_frames);
	frame_rotations.resize((num_frames) * num_joints);

	// use controllers to sample the clips at the right intervals.
	ClipController* from_controller = new ClipController(skel);
	ClipController* to_controller = new ClipController(skel);

	from_controller->SetClip(from);
	to_controller->SetClip(to);

	const Quaternion* from_rotations = from_controller->GetPose()->GetRotations();
	const Quaternion* to_rotations = to_controller->GetPose()->GetRotations();

	Quaternion align_q = make_rotation(align_rotation, Vec3(0,1,0));
	from_controller->SetTime( from_start );
	to_controller->SetTime( to_start );
	int out_joint_offset = 0;
	for(int i = 0; i < num_frames; ++i)
	{
		from_controller->ComputePose();
		to_controller->ComputePose();

		float blend = compute_blend_param(i, num_frames);
		float one_minus_blend = 1.f - blend;

		// printf("Blending %s @ %f (%f) to %s @ %f (%f)\n", 
		// 	   from->GetName(),
		// 	   from_controller->GetFrame(), 
		// 	   from_controller->GetTime(),
		// 	   to->GetName(),
		// 	   to_controller->GetFrame(),
		// 	   to_controller->GetTime());		

		// transform the target pose with the alignment
		Vec3 target_root_off = align_translation + rotate(to_controller->GetPose()->GetRootOffset(), align_q);
		Quaternion target_root_q = normalize(align_q * to_controller->GetPose()->GetRootRotation());
		// {
		// 	Vec3 axis; float angle;
		// 	Quaternion q = to_controller->GetPose()->GetRootRotation();
		// 	get_axis_angle(to_controller->GetPose()->GetRootRotation(), axis, angle);
		// 	printf("pre-aligned q %f %f %f %f\n", q.a, q.b, q.c, q.r);
		// 	printf("pre-align rotation %f around %f %f %f\n", angle*TO_DEG, axis.x, axis.y, axis.z);
		// 	q = target_root_q;
		// 	get_axis_angle(q, axis, angle);
		// 	printf("post-aligned q %f %f %f %f\n", q.a, q.b, q.c, q.r);
		// 	printf("post-align rotation %f around %f %f %f\n", angle*TO_DEG, axis.x, axis.y, axis.z);
			
		// }

		root_translations[i] = blend * 
			from_controller->GetPose()->GetRootOffset() + one_minus_blend * target_root_off;
		slerp_rotation(root_rotations[i], from_controller->GetPose()->GetRootRotation(), target_root_q, one_minus_blend);
					
		for(int joint = 0; joint < num_joints; ++joint)
		{
			slerp_rotation(frame_rotations[out_joint_offset], from_rotations[joint],
				  to_rotations[joint], one_minus_blend);
			++out_joint_offset;
		}

		from_controller->UpdateTime( sample_interval );
		to_controller->UpdateTime( sample_interval );
	}
	
	delete from_controller;
	delete to_controller;
}
*/

/*
sqlite3_int64 createTransitionClip(sqlite3* db, 
								   const Skeleton* skel,
								   const Clip* from, 
								   const Clip* to, 
								   float from_start, 
								   float to_start,
								   int num_frames, float sample_interval,
								   Vec3_arg align_translation, 
								   float align_rotation, 
								   const char* transition_name)
{
	std::vector< Vec3 > root_translations;
	std::vector< Quaternion > root_rotations;
	std::vector< Quaternion > frame_rotations;
	const int num_joints = skel->GetNumJoints();
	blendClips(skel, from, to, from_start, to_start,
			   num_frames, sample_interval, align_translation, align_rotation,
			   root_translations, root_rotations, frame_rotations);
	ASSERT( (int)root_translations.size() == num_frames );
	ASSERT( (int)root_rotations.size() == num_frames );
	ASSERT( (int)frame_rotations.size() == (num_frames) * num_joints );

	SavePoint save(db, "createTransitionClip");

	Query insert_clip(db, "INSERT INTO clips(skel_id,name,fps,num_frames,frames) "
					  "VALUES (?, ?, ?, ?, ?)");	
	insert_clip.BindInt64(1, skel->GetID())
		.BindText(2, transition_name)
		.BindDouble(3, 1.f/sample_interval)
        .BindInt(4, num_frames)
        .BindBlob(5, sizeof(Quaternion) * num_frames * num_joints + sizeof(ClipFrameHeader) * num_frames);
	insert_clip.Step();
	if(insert_clip.IsError()) {
		save.Rollback();
		return 0;
	}
	sqlite3_int64 clip_id = insert_clip.LastRowID();

    Blob framesWriter(db, "clips", "frames", clip_id, true);
    int framesOffset = 0;

	int joint_index = 0;
	for(int i = 0; i < num_frames; ++i)
	{
        ClipFrameHeader header;
        header.root_offset = root_translations[i];
        header.root_quaternion = root_rotations[i];

        framesWriter.Write(&header, sizeof(header), framesOffset);
        framesOffset += sizeof(header);

		for(int joint = 0; joint < num_joints; ++joint)
		{
            framesWriter.Write(&frame_rotations[joint_index++], sizeof(frame_rotations[0]), framesOffset);
            framesOffset += sizeof(frame_rotations[0]);
		}
	}
    framesWriter.Close();

	// finally, annotate the clip with a union of the parent clip annotations 
	Query get_annos(db, "SELECT DISTINCT annotation_id FROM clip_annotations WHERE clip_id = ? OR clip_id = ?");
	get_annos.BindInt64(1, from->GetID()).BindInt64(2, to->GetID());
	Query insert_anno(db, "INSERT INTO clip_annotations(annotation_id, clip_id) VALUES (?,?)");
	while(get_annos.Step()) {
		insert_anno.Reset();
		insert_anno.BindInt64(1, get_annos.ColInt64(0)).BindInt64(2,clip_id);
		insert_anno.Step();
	}

	return clip_id;
}
*/
