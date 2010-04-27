#ifndef INCLUDED_anim_clipcontroller_HH
#define INCLUDED_anim_clipcontroller_HH

#include "animcontroller.hh"
class Skeleton;
class Clip;

class ClipController : public AnimController
{
	const Skeleton* m_skel;
	const Clip* m_clip;
	float m_frame;
	
public:
	ClipController ();

	void ComputePose( Pose* out ) ;

	void SetSkeleton( const Skeleton* skel ) ;
	void SetClip( const Clip* clip ) ;
	void UpdateTime( float dt );
	void SetFrame( float frame );

	float GetFrame() const { return m_frame; }
	bool IsAtEnd() const ;
	void SetToLastFrame() ;
private:
	void ClampSetFrame( float frame );
};

#endif
