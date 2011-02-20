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

Vec3 MGPath::PointAtLength(float length) const
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

Vec3 MGPath::ClosestPointToPath(Vec3_arg pt) const
{
    if(m_path_points.empty()) 
        return Vec3(0,0,0);

    float bestDist = magnitude_squared(pt - m_path_points[0]);
    Vec3 bestPt = m_path_points[0];
    
    const int numSegments = m_path_points.size() - 1;
    
    for(int seg = 0; seg < numSegments; ++seg)
    {
        Vec3 toEnd = (m_path_points[seg+1] - m_path_points[seg]);
        float magToEnd = magnitude(toEnd);
        toEnd /= (magToEnd > 0 ? magToEnd : 1);
        Vec3 toPt = pt - m_path_points[seg];
        
        float comp = dot(toPt, toEnd);
        Vec3 cmpPt;

        if(comp < 0) cmpPt = m_path_points[seg];
        else if(comp > magToEnd) cmpPt = m_path_points[seg+1];
        else cmpPt = m_path_points[seg] + comp * toEnd;

        float dist = magnitude_squared(cmpPt - pt);
        if(dist < bestDist) {
            bestDist = dist;
            bestPt = cmpPt;
        }
    }

    return bestPt;
}

float MGPath::ArcLengthToClosestPoint(Vec3_arg pt) const
{
    const int numSegments = m_path_points.size() - 1;
    float len = 0.f;
    float bestLen = 0.f;
    float bestDist = magnitude_squared(pt - m_path_points[0]);
    Vec3 bestPt = m_path_points[0];
    
    for(int seg = 0; seg < numSegments; ++seg)
    {
        Vec3 toEnd = (m_path_points[seg+1] - m_path_points[seg]);
        float magToEnd = magnitude(toEnd);
        toEnd /= (magToEnd > 0 ? magToEnd : 1);
        Vec3 toPt = pt - m_path_points[seg];
        
        float comp = dot(toPt, toEnd);
        Vec3 cmpPt;
        float curLen;

        if(comp < 0)
        {
            cmpPt = m_path_points[seg];
            curLen = len ;
        }
        else if(comp > magToEnd) 
        {
            cmpPt = m_path_points[seg+1];
            curLen = len + magToEnd;
        }
        else 
        {
            cmpPt = m_path_points[seg] + comp * toEnd;
            curLen = len + comp;
        }

        float dist = magnitude_squared(cmpPt - pt);
        if(dist < bestDist) {
            bestDist = dist;
            bestPt = cmpPt;
            bestLen = curLen;
        }

        len += magToEnd;
    }

    return bestLen;
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

bool MGPath::ReplaceOrAddPoint(Vec3_arg newPoint, float threshold)
{
	if((int)m_path_points.size() >= m_max_size) return false;

    if(!m_path_points.empty() && 
        magnitude_squared(m_path_points.back() - newPoint) < (threshold*threshold))
    {
        
    	if(m_path_points.size() >= 2) {
            m_len -= magnitude( m_path_points[ m_path_points.size() - 1] -
                    m_path_points[m_path_points.size() - 2]);
        }
        m_path_points.back() = newPoint;
    }
    else
    {
    	m_path_points.push_back(newPoint);
    }

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

