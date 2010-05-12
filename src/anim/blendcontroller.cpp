#include "blendcontroller.hh"

BlendController::BlendController( const Skeleton* skel )
	: AnimController(skel)
	, m_from(0)
	, m_to(0)
	, m_blend_time(1.f)
	, m_cur_time(0.f)
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
	m_blend_time = time_in_s ;
	m_cur_time = Clamp(m_cur_time, 0.f, m_blend_time);
}

void BlendController::SetCurrentTime( float curtime )
{
	m_cur_time = Clamp(curtime, 0.f, m_blend_time);
}

void BlendController::UpdateTime(float t)
{
	SetCurrentTime(m_cur_time + t);
}

bool BlendController::IsComplete() const 
{
	return m_cur_time >= m_blend_time;
}
		
void BlendController::ComputePose( ) 
{
	float t = Clamp(m_cur_time / m_blend_time, 0.f, 1.f);
	(void)t;
	
}
