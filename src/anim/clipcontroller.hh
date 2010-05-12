#ifndef INCLUDED_anim_clipcontroller_HH
#define INCLUDED_anim_clipcontroller_HH

#include "animcontroller.hh"
class Skeleton;
class Clip;

class ClipController : public AnimController
{
	const Clip* m_clip;
	float m_frame;
	
public:
	ClipController (const Skeleton* skel);

	void ComputePose( ) ;

	void SetClip( const Clip* clip ) ;
	void UpdateTime( float dt );
	void SetTime( float time );
	void SetFrame( float frame );

	float GetFrame() const { return m_frame; }
	bool IsAtEnd() const ;
	void SetToLastFrame() ;	
private:
	void ClampSetFrame( float frame );
};

#endif
