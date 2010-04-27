#include <GL/gl.h>
#include "posehelper.hh"
#include "anim/pose.hh"
#include "skeleton.hh"

void drawPose( const Skeleton* skel, const Pose* pose) 
{
	Vec3 root_offset = pose->GetRootOffset();

	int num_joints = pose->GetNumJoints();

	const Vec3 *offsets = pose->GetOffsets();

	glColor3f(1.f, 0.3f, 1.f);
	glBegin(GL_LINES);
	for(int i = 0; i < num_joints; ++i)
	{			
		int parent = skel->GetJointParent(i);
		if(parent == -1) {			
			glVertex3fv(&root_offset.x);
			glVertex3fv(&offsets[i].x);
		} else {
			glVertex3fv(&offsets[parent].x);
			glVertex3fv(&offsets[i].x);
		}
	}		
	glEnd();
}
