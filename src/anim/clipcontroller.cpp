#include <cmath>
#include <cstdio>
#include "clipcontroller.hh"
#include "pose.hh"
#include "clip.hh"
#include "skeleton.hh"
#include "MathUtil.hh"

ClipController::ClipController(const Skeleton* skel)
	: AnimController( skel )
	, m_clip(0)
	, m_frame(0.f)
	, m_min_frame(0.f)
	, m_max_frame(0.f)
{
}

void ClipController::ComputePose( ) 
{
	if(m_clip) 
	{
		float frame_low = floor(m_frame);
		float frame_hi = ceil(m_frame);
		float fraction = m_frame - frame_low;
		float one_minus_fraction = 1.0 - fraction;

		int iframe_low = frame_low;
		int iframe_hi = frame_hi;

		const Quaternion *rotations_low = m_clip->GetFrameRotations(iframe_low);
		const Quaternion *rotations_hi = m_clip->GetFrameRotations(iframe_hi);

		Vec3 root_pos = one_minus_fraction * m_clip->GetFrameRootOffset(iframe_low) 
			+ fraction * m_clip->GetFrameRootOffset(iframe_hi) ;

		Quaternion root_rot;
		slerp_rotation( root_rot, m_clip->GetFrameRootOrientation(iframe_low),
						m_clip->GetFrameRootOrientation(iframe_hi), fraction);

		Quaternion* out_rotations = m_pose->GetRotations();

		int num_joints = m_skel->GetNumJoints();

		Quaternion anim_rot ;
		for(int i = 0; i < num_joints; ++i) {
			slerp_rotation(anim_rot, rotations_low[i], rotations_hi[i], fraction);
			out_rotations[i] = anim_rot;
		}			

		m_pose->SetRootOffset(root_pos);
		m_pose->SetRootRotation(root_rot);
	}
	else
		m_pose->RestPose(m_skel);

}

void ClipController::SetClip( const Clip* clip ) 
{
	if(m_clip != clip) {
		m_clip = clip;
		m_frame = 0.f;
	}

	if(m_clip) {
		m_min_frame = 0.f;
		m_max_frame = clip->GetNumFrames() - 1;
	}
}

void ClipController::UpdateTime( float dt )
{
	if(m_clip) {
		float newTime = m_frame + dt * m_clip->GetClipFPS();
		ClampSetFrame(newTime);
	} 
}

void ClipController::SetTime( float time )
{
	float newFrame = time * m_clip->GetClipFPS();
	ClampSetFrame(newFrame);
}

void ClipController::SetFrame( float frame )
{
	ClampSetFrame(frame);
}

bool ClipController::IsAtEnd() const 
{
	if(m_clip)
		return m_frame >= m_max_frame;
	else
		return true;
}

void ClipController::ClampSetFrame(float t)
{
	if(m_clip) {
		m_frame = Clamp(t, m_min_frame, m_max_frame);
	} else 
		m_frame = 0.f;	
}

void ClipController::SetToLastFrame() 
{
	if(m_clip) {
		m_frame = m_max_frame;
	} else 
		m_frame = 0.f;
}

float ClipController::GetTime() const
{
	if(m_clip) {
		return m_frame / m_clip->GetClipFPS();
	} else return 0.f;
}

void ClipController::SetPartialRange( float min_frame, float max_frame )
{
	if(m_clip) {
		float clip_max = m_clip->GetNumFrames()-1.f;
		m_min_frame = Clamp(min_frame, 0.f, clip_max);
		m_max_frame = Clamp(max_frame, min_frame, clip_max);
	} else {
		m_min_frame = 0.f;
		m_max_frame = 0.f;
	}	

	m_frame = Clamp(m_frame, min_frame, max_frame);
}
