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

