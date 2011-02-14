#ifndef INCLUDED_anim_blendcontroller_HH
#define INCLUDED_anim_blendcontroller_HH

#include "animcontroller.hh"

class BlendController : public AnimController
{
	AnimController* m_from;
	AnimController* m_to;
	float m_blendTime;
	float m_curTime;
public:
	BlendController( const Skeleton* skel );
	void SetFrom( AnimController* controller );
	void SetTo( AnimController* controller );
	void SetBlendLength( float time_in_s);
	void SetCurrentTime( float curtime );
    float GetCurrentTime() const; // current BLEND time in range (0.. blendTime)
	void UpdateTime(float t);

	bool IsComplete() const ;
		
	void ComputePose( ) ;
};


#endif
