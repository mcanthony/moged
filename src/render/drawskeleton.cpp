#include <cstdio>
#include <GL/gl.h>
#include <GL/glu.h>
#include "drawskeleton.hh"
#include "skeleton.hh"
#include "Vector.hh"
#include "Quaternion.hh"

DrawSkeletonHelper::DrawSkeletonHelper() 
  : m_skel(0)
  , m_offsets(0)
  , m_selected(0)
  , m_initialized(false)
  , m_quadric(0)
{
}

DrawSkeletonHelper::~DrawSkeletonHelper()
{
	delete[] m_offsets; 
	delete[] m_selected;

	gluDeleteQuadric(m_quadric);
}

static void DrawBone(GLUquadric *quadric, Vec3_arg a, Vec3_arg b, float w)
{
	Vec3 a_to_b = b-a;
	float length = magnitude(a_to_b);
	if(length > 0)
	{
		Vec3 na = a_to_b / length;
		float cos_na_z = dot(na,Vec3(0,0,1));
		Quaternion q(0,0,0,1);
		if(cos_na_z < 0.99999) {
			float angle = acos(cos_na_z);
			Vec3 axis = normalize(cross(na,Vec3(0,0,1))); // normalize to fix 'floating' joints.
			q = make_rotation(angle,axis); // this seems silly, need a more direct "matrix from axis & angle" func
		}

		Mat4 m = q.to_matrix();
		glPushMatrix();
		glTranslatef(a.x,a.y,a.z);
		glMultMatrixf(m.m);

		float bulge = 0.8 * length;
		const float smallest = 0.004f;
		float largest = Clamp( 0.005f*w, smallest, length/1.618f ); // just 'cause :)

		glTranslatef(0,0,length - bulge);
		gluCylinder(quadric, largest, smallest, bulge, 8, 8);
		glRotatef(180.f, 1, 0, 0);	
		gluCylinder(quadric, largest, smallest, length - bulge, 8, 3);
		
		glPopMatrix();
	}
}


void DrawSkeletonHelper::Init()
{
	if(!m_initialized)
	{
		m_quadric = gluNewQuadric();
		m_initialized = true;
	}
}

void DrawSkeletonHelper::Draw()
{
	if(m_skel && m_quadric)
	{
		Vec3 root_offset = m_skel->GetRootOffset();
		Quaternion root_rotation = m_skel->GetRootRotation();

		int num_joints = m_skel->GetNumJoints();
		for(int i = 0; i < num_joints; ++i)
		{			
			int parent = m_skel->GetJointParent(i);
			Vec3 offset = rotate(m_skel->GetJointTranslation(i), root_rotation);

			float w = m_weights->GetJointWeight(i);

			if(!m_selected[i]) glColor3f(0.3f, 0.3f, 1.f);
			else glColor3f(1.f, 0.3f, 1.f);
			
			if(parent == -1) {
				m_offsets[i] = root_offset + offset; 
				DrawBone(m_quadric, root_offset, m_offsets[i], w);
			} else {
				m_offsets[i] = m_offsets[parent] + offset;
				DrawBone(m_quadric, m_offsets[parent], m_offsets[i], w);
			}
		}		
	}
}

void DrawSkeletonHelper::SetSkeleton(const Skeleton* skel, const SkeletonWeights* weights)
{
	if(m_skel != skel) 
	{
		m_skel = skel;
		m_weights = weights;

		delete[] m_offsets; m_offsets = 0;
		delete[] m_selected; m_selected = 0;

		if(m_skel)
		{
			int num_joints = skel->GetNumJoints();
			m_offsets = new Vec3[num_joints];
			m_selected = new int[num_joints];
			memset(m_selected,0,sizeof(int)*num_joints);
		}
	}
}

void DrawSkeletonHelper::SetSelected(int idx, bool selected)
{
	if(m_skel && idx >= 0 && idx < m_skel->GetNumJoints()) {
		m_selected[idx] = selected ? 1 : 0;		
	}
}

