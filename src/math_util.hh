#ifndef INCLUDED_math_util_HH
#define INCLUDED_math_util_HH

#include <cmath>
#include "Vector.hh"
#include "Quaternion.hh"

namespace Math
{
    // A value very close to zero.
    extern const float kEpsilon ;

    
    // return the rotation that should be applied to 'start' to align it with 'align'.
    inline Quaternion align_rotation(Vec3_arg start, Vec3_arg align)
    {
        Vec3 axis = normalize(cross(start,align));
        float angle = acos(dot(normalize(start),normalize(align)));
        return make_rotation(angle, axis);
    }
}

inline float ComputeCubicBlendParam(float term)
{
	// interpolation scheme from Kovar paper - basically just a 
	// cubic with f(0) = 1, f(1) = 0, f'(0) = f'(1) = 0
	float term2 = term*term;
	float term3 = term2*term;	
	float param = 2.f * term3 - 3.f * term2 + 1.f;
	return Clamp(param, 0.f, 1.f);
}

// cubic blend parameter. Returns 0..1. 
//  p is in 0..k. 
inline float ComputeCubicBlendParam(int p, int k)
{
	// interpolation scheme from Kovar paper - basically just a 
	// cubic with f(0) = 1, f(1) = 0, f'(0) = f'(1) = 0
	float term = (p)/float(k);
    return ComputeCubicBlendParam(term);
}

#endif

