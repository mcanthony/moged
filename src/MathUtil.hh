#ifndef LAB_MATH_UTIL_HH
#define LAB_MATH_UTIL_HH

#include <cmath>

class Vec3;

#define PI  3.14159265358979323846f
#define TWOPI  (2.0f * PI)
#define E  2.71828182845904523536f

#define TO_RAD  (PI/180.0f)
#define TO_DEG  (180.0f/PI)

inline float Abs(float v) { return fabs(v); }

// Lerp and Clamp are basically the same as those found in
// Physically Based Rendering by Pharr,Humphreys
inline float Lerp(float t, float v1, float v2)
{
	// implemented this way because (v2-v1) may lose accuracy if v2-v1 is large.
	return (1.0f-t)*v1 + t*v2; 
}

template< class T >
inline T Clamp(T val, T min, T max)
{
    if ( val < min) return min;
    else if( val > max) return max;
    return val;
}
	
template< class T >
inline T Min(T left, T right)
{
	return (left < right ? left : right);
}

template< class T >
inline T Max(T left, T right)
{
	return (left > right ? left : right);
}

inline float DegToRad(float deg)
{
	return deg*TO_RAD;
}

inline float RadToDeg(float rad)
{
	return rad*TO_DEG;
}

inline float ClampAngleRad(float angle)
{
	while(angle > TWOPI) {
		angle -= TWOPI;
	}
	while(angle < 0.f) {
		angle += TWOPI;
	}
	return angle;
}

inline float ClampAngleDeg(float angle)
{
	while(angle > 360.f) {
		angle -= 360.f;
	}
	while(angle < 0.f) {
		angle += 360.f;
	}
	return angle;
}

inline float SumSquareDiff(const float *a1, const float *a2, int n)
{
	float result = 0.0f;
	while(n-- > 0) 
	{
		float diff = *a1++ - *a2++;
		diff = diff * diff;
		result += diff;
	}
	return result;
}

inline float Sign(float v) { return v < 0 ? -1.0f : 1.0f ; }


// y0 = cur y
// y1 = next y
// s0 = slope at cur y
// s1 = slope at next y
inline float HermiteInterp(float t, float y0, float y1, float s0, float s1)
{
	float z = t ;
	float y = z * z;
	float x = y * z;

	float row0 = 2.0f * x - 3.0f * y + 1;
	float row1 = -2.0f * x + 3.0f * y;
	float row2 = x - 2.0f * y + z;
	float row3 = x - y;

	float result = y0 * row0 + y1 * row1 + s0 * row2 + s1 * row3;
	return result;
}

inline float CatmullRomInterp(float t, float y0, float y1, float y2, float y3)
{
	float s0 = 0.5f * y2 - 0.5f * y0;
	float s1 = 0.5f * y3 - 0.5f * y1;		
	return HermiteInterp(t, y1, y2, s0, s1);
}

// Output must have room for at least n+1 elements.
int ConvexHull2D(Vec3* input, Vec3* output, int n);


#endif
