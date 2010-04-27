#ifndef INCLUDED_anim_animcontroller_HH
#define INCLUDED_anim_animcontroller_HH

class Pose;

class AnimController
{
public:
	virtual ~AnimController() {}
	virtual void ComputePose(Pose* out ) = 0 ;
};

#endif
