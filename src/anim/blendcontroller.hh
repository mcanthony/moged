#ifndef INCLUDED_anim_blendcontroller_HH
#define INCLUDED_anim_blendcontroller_HH

#include "animcontroller.hh"

class BlendController : public AnimController
{
	AnimController* m_from;
	AnimController* m_to;
	float m_blend_time;
	float m_cur_time;
public:
	BlendController( const Skeleton* skel );
	void SetFrom( AnimController* controller );
	void SetTo( AnimController* controller );
	void SetBlendLength( float time_in_s);
	void SetCurrentTime( float curtime );
	void UpdateTime(float t);

	bool IsComplete() const ;
		
	void ComputePose( ) ;
};


#endif
