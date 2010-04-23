#include <cstdio>
#include <GL/gl.h>
#include "drawskeleton.hh"
#include "skeleton.hh"
#include "Vector.hh"
#include "Quaternion.hh"

DrawSkeletonHelper::DrawSkeletonHelper() 
  : m_skel(0)
  , m_offsets(0)
  , m_rotations(0)
{
}

DrawSkeletonHelper::~DrawSkeletonHelper()
{
	delete[] m_offsets; 
	delete[] m_rotations; 
}

void DrawSkeletonHelper::Draw()
{
	if(m_skel && m_offsets && m_rotations)
	{
		Vec3 root_offset = m_skel->GetRootOffset();
		Quaternion root_rotation = m_skel->GetRootRotation();

		int num_joints = m_skel->GetNumJoints();
		glColor3f(1.f, 0.3f, 1.f);
		glBegin(GL_LINES);
		for(int i = 0; i < num_joints; ++i)
		{			
			int parent = m_skel->GetJointParent(i);
			Vec3 offset = m_skel->GetJointTranslation(i);
			Quaternion rot = m_skel->GetJointOrientation(i);
			
			if(parent == -1) {
				m_rotations[i] = root_rotation * rot;
				m_offsets[i] = root_offset + offset; 
				glVertex3fv(&root_offset.x);
				glVertex3fv(&m_offsets[i].x);
			} else {
				m_rotations[i] = m_rotations[parent] * rot;
				m_offsets[i] = m_offsets[parent] + offset;
				glVertex3fv(&m_offsets[parent].x);
				glVertex3fv(&m_offsets[i].x);
			}
		}		
		glEnd();
	}
}

void DrawSkeletonHelper::SetSkeleton(const Skeleton* skel)
{
	if(m_skel != skel) 
	{
		m_skel = skel;
		delete[] m_offsets; m_offsets = 0;
		delete[] m_rotations; m_rotations = 0;
	}

	if(m_skel)
	{
		int num_joints = skel->GetNumJoints();
		m_offsets = new Vec3[num_joints];
		m_rotations = new Quaternion[num_joints];
	}
}

