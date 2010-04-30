#ifndef LAB_MATH_MAT4_HH
#define LAB_MATH_MAT4_HH

#include <cstring>
#include "Vector.hh"
#include "MathUtil.hh"

class Mat4;
typedef const Mat4& Mat4_arg;

inline Mat4 operator*(Mat4_arg lhs, Mat4_arg rhs) ;

class Mat4
{
public:
	struct ident_t { };

	float m[16];
			
	Mat4( ) {}		
	Mat4( float v ) {
		for(int i = 0; i < 16; ++i) {
			m[i] = v;
		}
	}
	Mat4( ident_t ) {	
		for(int i = 0; i < 16; ++i) {
			m[i] = 0.0;
		}
		m[0] = 1.f; m[5] = 1.f; m[10] = 1.f; m[15] = 1.f;
	}	

	Mat4( float a, float b, float c, float d,
		  float e, float f, float g, float h,
		  float i, float j, float k, float l,
		  float mm, float n, float o, float p)
		{
			m[0] = a; m[1] = b; m[2] = c; m[3] = d;
			m[4] = e; m[5] = f; m[6] = g; m[7] = h;
			m[8] = i; m[9] = j; m[10] = k; m[11] = l;
			m[12] = mm; m[13] = n; m[14] = o; m[15] = p;
		}

	Mat4( float v[16] ) 
		{
			memcpy(m,v,sizeof(float)*16);
		}
	Mat4(Mat4_arg  other) {
		memcpy(m,other.m,sizeof(m));
	}
			
	Mat4& operator=(Mat4_arg  other) {
		if(this != &other) {
			memcpy(m,other.m,sizeof(m));
		} 
		return *this;
	}

	bool operator==(Mat4_arg  other) const {
		return (memcmp(m,other.m,sizeof(m)) == 0);
	}
		
	bool operator!=(Mat4_arg  other) const {
		return !operator==(other);
	}

	Mat4& operator*=(Mat4_arg  other) 
		{
			*this = (*this * other);
			return *this;
		}

	Mat4& operator+=(Mat4_arg  other) 
		{
			for(int i = 0; i < 16; ++i) {
				m[i] += other.m[i];
			}
			return *this;
		}

};

inline float det3x3(float a11, float a12, float a13,
					float a21, float a22, float a23,
					float a31, float a32, float a33) 
{
	float d1 = a22*a33 - a23*a32;
	float d2 = a23*a31 - a21*a33; // negated
	float d3 = a21*a32 - a22*a31;
	
	float r = a11 * d1 + a12 * d2 + a13 * d3;
	return r;
}

inline float det(Mat4_arg m)
{	
	float d1 = det3x3(m.m[1*4 + 1],m.m[1*4 + 2],m.m[1*4 + 3],
					  m.m[2*4 + 1],m.m[2*4 + 2],m.m[2*4 + 3],
					  m.m[3*4 + 1],m.m[3*4 + 2],m.m[3*4 + 3]);
	float d2 = det3x3(m.m[1*4 + 0],m.m[1*4 + 2],m.m[1*4 + 3],
					  m.m[2*4 + 0],m.m[2*4 + 2],m.m[2*4 + 3],
					  m.m[3*4 + 0],m.m[3*4 + 2],m.m[3*4 + 3]);
	float d3 = det3x3(m.m[1*4 + 0],m.m[1*4 + 1],m.m[1*4 + 3],
					  m.m[2*4 + 0],m.m[2*4 + 1],m.m[2*4 + 3],
					  m.m[3*4 + 0],m.m[3*4 + 1],m.m[3*4 + 3]);
	float d4 = det3x3(m.m[1*4 + 0],m.m[1*4 + 1],m.m[1*4 + 2],
					  m.m[2*4 + 0],m.m[2*4 + 1],m.m[2*4 + 2],
					  m.m[3*4 + 0],m.m[3*4 + 1],m.m[3*4 + 2]);
				
	float r = m.m[0*4 + 0] * d1 - m.m[0*4 + 1] * d2 + m.m[0*4 + 2] * d3 - m.m[0*4 + 3] * d4;
	return r;	
}

inline Mat4 transpose(Mat4_arg m)
{
	Mat4 result = m;
	for(int i = 0; i < 4; ++i) {
		for(int j = 0; j < i; ++j) {
			int firstIdx = j + 4*i;
			int secondIdx = i + 4*j;
			float t = result.m[firstIdx];
			result.m[firstIdx] = result.m[secondIdx];
			result.m[secondIdx] = t;
		}
	}
	return result;
}

inline Mat4 operator+(Mat4_arg lhs, Mat4_arg  rhs) 
{
	Mat4 result;
	for(int i = 0; i < 16; ++i) {
		for(int j = 0; j < 16; ++j) {
			result.m[i] = lhs.m[i] + rhs.m[i];
		}
	}
	return result;
}


inline Mat4 operator*(Mat4_arg lhs, Mat4_arg rhs) 
{
	Mat4 result;
				
	int resultIdx = 0;
	for(int rowIdx = 0; rowIdx < 16; rowIdx += 4)
	{
		for(int col = 0; col < 4; ++col)
		{
			result.m[resultIdx] = lhs.m[rowIdx] * rhs.m[col]
				+ lhs.m[rowIdx + 1] * rhs.m[col + 4]
				+ lhs.m[rowIdx + 2] * rhs.m[col + 8]
				+ lhs.m[rowIdx + 3] * rhs.m[col + 12];
			++resultIdx;
		}
	}
	return result;
}

inline Vec3 transform_point(Mat4_arg lhs, Vec3_arg rhs)
{
	float x = lhs.m[0] * rhs.x + lhs.m[1] * rhs.y + lhs.m[2] * rhs.z + lhs.m[3];
	float y = lhs.m[4] * rhs.x + lhs.m[5] * rhs.y + lhs.m[6] * rhs.z + lhs.m[7];
	float z = lhs.m[8] * rhs.x + lhs.m[9] * rhs.y + lhs.m[10] * rhs.z + lhs.m[11];
	return Vec3(x,y,z);
}

inline Vec3 transform_vector(Mat4_arg lhs, Vec3_arg rhs)
{
	float x = lhs.m[0] * rhs.x + lhs.m[1] * rhs.y + lhs.m[2] * rhs.z;
	float y = lhs.m[4] * rhs.x + lhs.m[5] * rhs.y + lhs.m[6] * rhs.z;
	float z = lhs.m[8] * rhs.x + lhs.m[9] * rhs.y + lhs.m[10] * rhs.z;
	return Vec3(x,y,z);
}

inline Mat4 rotation_x(float rad)
{
	float a = cos(rad);
	float b = sin(rad);
	return Mat4(1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, a,    -b,    0.0f,
				0.0f, b,   a,    0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
}

inline Mat4 rotation_y(float rad)
{
	float a = cos(rad);
	float b = sin(rad);
	return Mat4(a,    0.0f, b, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				-b,   0.0f, a,    0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
}

inline Mat4 rotation_z(float rad)
{
	float a = cos(rad);
	float b = sin(rad);
	return Mat4 (a,    -b,    0.0f, 0.0f,
				 b,   a,    0.0f, 0.0f,
				 0.0f, 0.0f, 1.0f, 0.0f,
				 0.0f, 0.0f, 0.0f, 1.0f);
	
}

inline Mat4 translation( Vec3_arg t)
{
	return Mat4 (1.f, 0.f, 0.f, t.x,
				 0.f, 1.f, 0.f, t.y,
				 0.f, 0.f, 1.f, t.z,
				 0.f, 0.f, 0.f, 1.f );	
}
			

inline float SumSquareDiff(Mat4_arg  m1, Mat4_arg  m2)
{
	return SumSquareDiff(m1.m, m2.m, 16);
}

inline bool IsOrthonormal(Mat4_arg  m)
{
	Vec3 row0( &m.m[0]);
	Vec3 row1( &m.m[4]);
	Vec3 row2( &m.m[8]);
		
	float len ;
	len = magnitude(row0);
	if( Abs(1.0 - len) > 1e-3f) return false;
	len = magnitude(row1);
	if( Abs(1.0 - len) > 1e-3f) return false;
	len = magnitude(row2);
	if( Abs(1.0 - len) > 1e-3f) return false;
		
	if(dot(row0,row1) > 1e-3f) return false;
	if(dot(row1,row2) > 1e-3f) return false;
	if(dot(row2,row0) > 1e-3f) return false;
	return true;

}
inline void MultiplyMatrix(float out[4], Mat4_arg  m, float const in[4])
{
	for(int row = 0, rowIdx = 0; row < 4; rowIdx+=4, ++row)
	{
		out[row] = m.m[rowIdx + 0] * in[0] + m.m[rowIdx + 1] * in[1] + m.m[rowIdx + 2] * in[2] + m.m[rowIdx + 3] * in[3];
	}
}


#endif
