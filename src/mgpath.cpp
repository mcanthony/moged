#include <GL/gl.h>
#include "mgpath.hh"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
MGPath::MGPath(int maxSize)
	: m_max_size(maxSize)
	, m_len(0.f)
{
	m_path_points.reserve(maxSize);
}

void MGPath::SetMaxSize(int max_size) 
{
	if(max_size < (int)m_path_points.size()) {
		m_path_points.resize(max_size);
	} else {
		m_path_points.reserve(max_size);
	}
	m_max_size = max_size;
}

float MGPath::TotalLength() const 
{
	return m_len;
}

Vec3 MGPath::PointAtLength(float length)
{
	if(m_path_points.empty()) return Vec3(0,0,0);
	float lenLeft = length;
	const int num_points = m_path_points.size();
	for(int i = 1; i < num_points; ++i)
	{
		Vec3 seg_vec = m_path_points[i] - m_path_points[i-1];
		float len = magnitude(seg_vec);
		if(lenLeft > len) {
			lenLeft -= len;
		} else {
			if(lenLeft == 0.f) return m_path_points[i-1];
			float param = lenLeft / len;
			return m_path_points[i-1] + param * seg_vec;
		}
	}
	return m_path_points[ m_path_points.size() - 1];
}

bool MGPath::AddPoint( Vec3_arg newPoint )
{
	if((int)m_path_points.size() >= m_max_size) return false;
	m_path_points.push_back(newPoint);
	if(m_path_points.size() >= 2) {
		m_len += magnitude( m_path_points[ m_path_points.size() - 1] -
							m_path_points[ m_path_points.size() - 2] );
	}
	return true;
}

void MGPath::SmoothPath()
{
	// 5 pt gaussian
	static const float kSmoothFilter[] = {0.0674508058663448, 0.183350299901404, 
										  0.498397788464502, 0.183350299901404,
										  0.0674508058663448};
	static const int kSmoothLen = sizeof(kSmoothFilter)/sizeof(float);

	const int num_points = m_path_points.size();
	std::vector< Vec3 > result_points( num_points );
	for(int i = 0; i < num_points; ++i) {
		Vec3 smoothed(0,0,0);
		for(int j = 0; j < kSmoothLen; ++j) {
			int idx = Clamp( i - kSmoothLen/2 + j, 0, num_points-1 );
			smoothed += kSmoothFilter[j] * m_path_points[idx] ;
		}
		result_points[i] = smoothed;
	}
	m_path_points = result_points;
	
	float len = 0.f;
	for(int i = 1; i < num_points; ++i) {
		len += magnitude( m_path_points[i] - m_path_points[i-1] );
	}
	m_len = len;
}


void MGPath::Clear()
{
	m_path_points.clear();
	m_len = 0.f;
}

void MGPath::Draw() const
{
	glBegin(GL_LINE_STRIP);
	const int num_points = m_path_points.size();
	for(int i = 0; i < num_points; ++i) {
		glVertex3fv(&m_path_points[i].x);
	}
	glEnd();
}

