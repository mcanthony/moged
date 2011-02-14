// -*- C++ -*-
#ifndef INCLUDED_VECTOR_HH
#define INCLUDED_VECTOR_HH
#include <cmath>
#include "MathUtil.hh"
#include "assert.hh"

class Vec3;
typedef const Vec3& Vec3_arg;

class Vec3
{
public:
	float x,y,z;

	inline Vec3() {}
	inline Vec3(float x, float y, float z) : x(x),y(y),z(z) {}
	inline Vec3(Vec3_arg other) : x(other.x),y(other.y),z(other.z) {}
	inline Vec3(const float* v) : x(v[0]), y(v[1]), z(v[2]) {}
	inline Vec3& operator=(Vec3_arg  other)
		{
			x = other.x; y = other.y; z = other.z;
			return *this;
		}

	inline bool operator==(Vec3_arg  other) const 
		{
			return x == other.x && y == other.y && z == other.z;
		}
		
	inline bool operator!=(Vec3_arg  other) const
		{
			return !(*this == other);
		}

	inline Vec3 operator-() const
		{
			return Vec3(-x,-y,-z);
		}

	inline Vec3_arg operator-=(Vec3_arg other)
		{
			x-= other.x; y-= other.y; z-= other.z;
			return *this;
		}

	inline Vec3_arg  operator+=(Vec3_arg other)
		{
			x+= other.x; y+= other.y; z+= other.z;
			return *this;
		}

	inline Vec3_arg  operator*=(float s)
		{
			x*=s; y*=s; z*=s;
			return *this;
		}

	inline Vec3_arg  operator/=(float s)
		{
			x/=s;y/=s;z/=s;
			return *this;
		}

	inline float operator[](int idx) const {
		ASSERT(idx >= 0 && idx < 3);
		return (&x)[idx];
	}

	inline void set(float x, float y, float z) 
		{ 
			this->x = x;
			this->y = y;
			this->z = z;
		}
};

inline Vec3 operator+(Vec3_arg lhs, Vec3_arg rhs) 
{
	return Vec3(lhs.x + rhs.x,
				lhs.y + rhs.y,
				lhs.z + rhs.z);
}


inline Vec3 operator-(Vec3_arg lhs, Vec3_arg rhs) 
{
	return Vec3(lhs.x - rhs.x,
				lhs.y - rhs.y,
				lhs.z - rhs.z);
}


inline Vec3 operator*(Vec3_arg lhs, float s) 
{
	return Vec3(lhs.x*s,lhs.y*s,lhs.z*s);
}
		

inline Vec3 operator* (float scalar, Vec3_arg  other) 
{
	return other*scalar;
}

inline Vec3 operator/(Vec3_arg lhs, float s) 
{
	return Vec3(lhs.x/s, lhs.y/s, lhs.z/s);
}

inline float SumSquareDiff(Vec3_arg  v1, Vec3_arg  v2)
{
	return SumSquareDiff(&v1.x,&v2.x,3);
}

inline void Lerp(Vec3& out,
				 Vec3_arg  left,
				 Vec3_arg  right,
				 float param)
{
	out = (1.f - param) * left + param * right;
}

inline float dot(Vec3_arg lhs, Vec3_arg rhs)
{
	return 
		lhs.x * rhs.x + 
		lhs.y * rhs.y + 
		lhs.z * rhs.z;
}

inline float magnitude_squared(Vec3_arg vec)
{
	return dot(vec,vec);
}

inline float magnitude(Vec3_arg vec)
{
	return sqrt( magnitude_squared(vec) );
}
	
inline Vec3 normalize(Vec3_arg vec) 
{
	float mag = magnitude(vec);
	return vec/mag;
}

inline Vec3 cross(Vec3_arg lhs, Vec3_arg rhs)
{
	return Vec3( lhs.y * rhs.z - lhs.z * rhs.y,
				 lhs.z * rhs.x - lhs.x * rhs.z,
				 lhs.x * rhs.y - lhs.y * rhs.x );
}

inline Vec3 vec_xz(Vec3_arg v)
{
    return Vec3(v.x, 0.0, v.z);
}

#endif
