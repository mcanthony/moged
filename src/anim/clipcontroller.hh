#ifndef INCLUDED_anim_clipcontroller_HH
#define INCLUDED_anim_clipcontroller_HH

#include "animcontroller.hh"
class Skeleton;
class Clip;

class ClipController : public AnimController
{
	const Clip* m_clip;
	float m_frame;
	float m_min_frame;
	float m_max_frame;
public:
	ClipController (const Skeleton* skel);

	void ComputePose( ) ;

	void SetClip( const Clip* clip ) ;
	const Clip* GetClip() const { return m_clip; }
	void UpdateTime( float dt );
	void SetTime( float time );
	void SetFrame( float frame );

	float GetFrame() const { return m_frame; }
	float GetTime() const;
	bool IsAtEnd() const ;
	void SetToLastFrame() ;	

	void SetPartialRange( float min_frame, float max_frame );
private:
	void ClampSetFrame( float frame );
};

#endif
