#ifndef INCLUDED_motion_graph_path_HH
#define INCLUDED_motion_graph_path_HH

#include <vector>
#include "Vector.hh"

class MGPath
{
	int m_max_size;
	std::vector< Vec3 > m_path_points;
	float m_len;
public:
	MGPath(int maxSize);

	int GetMaxSize() const { return m_max_size; }
	void SetMaxSize(int max_size) ;

	float TotalLength() const ;
	Vec3 PointAtLength(float length);

	bool AddPoint( Vec3_arg newPoint );
	void SmoothPath();
	void Clear();
	void Draw() const;
	bool Empty() const { return m_path_points.empty(); }
	bool Full() const { return (int)m_path_points.size() == m_max_size; }
	int Size() const { return m_path_points.size(); }

	const Vec3& Front() { return m_path_points.front(); }
	const Vec3& Back() { return m_path_points.back(); }
};


#endif

