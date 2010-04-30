#include <cstdio>
#include <GL/gl.h>
#include <GL/glu.h>
#include "posehelper.hh"
#include "anim/pose.hh"
#include "skeleton.hh"
#include "Quaternion.hh"

void drawPose( const Skeleton* skel, const Pose* pose) 
{
	Vec3 root_offset = pose->GetRootOffset();
	Quaternion root_rot = pose->GetRootRotation();

	int num_joints = pose->GetNumJoints();

	const Vec3 *offsets = pose->GetOffsets();
	const Vec3 *rest_off = skel->GetJointTranslations();
	const Quaternion *rots = pose->GetRotations();
	
	glColor3f(1.f, 0.3f, 1.f); 
	glBegin(GL_LINES); 
	for(int i = 0; i < num_joints; ++i)
	{			
		int parent = skel->GetJointParent(i);
		if(parent == -1) {			
			glVertex3fv(&offsets[i].x); 
			Vec3 off = offsets[i] + rotate(rest_off[i], rots[i]);
			glVertex3f(off.x,off.y,off.z); 
		} else {
			glVertex3fv(&offsets[i].x); 
			Vec3 off = offsets[i] + rotate(rest_off[i], rots[i]);
			glVertex3f(off.x,off.y,off.z); 
		}
	}		
	glEnd();
}
