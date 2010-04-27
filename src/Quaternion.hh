#ifndef INCLUDED_MATH_QUATERNION
#define INCLUDED_MATH_QUATERNION

#include <cmath>
#include <cstring>
#include "Vector.hh"
#include "Mat4.hh"

// Math reference: http://local.wasp.uwa.edu.au/~pbourke/miscellaneous/quaternions/
// Passing Quaternions around by reference seemed to make this slower.

class Quaternion;
typedef const Quaternion& Quaternion_arg ;

class Quaternion
{
public:
	float a, b, c, r;	

	inline Quaternion() {}
	inline Quaternion( float a, float b, float c, float r ) :
		a(a),b(b),c(c),r(r) {}
	inline Quaternion( Vec3_arg v, float r) :
		a(v.x),b(v.y),c(v.z),r(r) { }
	inline Quaternion( Quaternion_arg q ) : a(q.a), b(q.b), c(q.b), r(q.r) { }

	inline Quaternion& operator=( Quaternion_arg q )  {
		if(this != &q) {
			a = q.a;
			b = q.b;
			c = q.c;
			r = q.r;
		}
		return *this;
	}

	inline void set(float qa, float qb, float qc, float qr) {
		a = qa;
		b = qb;
		c = qc;
		r = qr;
	}

	inline Quaternion operator-() const {
		return Quaternion(-a,-b,-c,-r);
	}

	inline Quaternion& operator+=(Quaternion_arg o) {
		r+=o.r;
		a+=o.a;
		b+=o.b;
		c+=o.c;
		return *this;
	}
		

	inline Quaternion& operator-=(Quaternion_arg o) {
		r-=o.r;
		a-=o.a;
		b-=o.b;
		c-=o.c;
		return *this;
	}

	inline void conjugate() {
		a=-a;
		b=-b;
		c=-c;
	}

	inline Quaternion& operator*=(float f) {
		r*=f;
		a*=f;
		b*=f;
		c*=f;
		return *this;
	}

	inline Quaternion operator/=(float f) {
		float inv_f = 1.0f/f;
		*this *= inv_f;
		return *this;
	}
		

	inline Mat4 to_matrix() const {			
		float aa = a*a;
		float ab = a*b;
		float ac = a*c;
		float ar = a*r;
			
		float bb = b*b;
		float bc = b*c;
		float br = b*r;
			
		float cc = c*c;
		float cr = c*r;

		float m[16] ;
		
		m[0] = 1.0f - 2.0f * (bb + cc);
		m[1] = 2.0f * (ab - cr);
		m[2] = 2.0f * (ac + br);
		m[3] = 0.0f;

		m[4] = 2.0f * (ab + cr);
		m[5] = 1.0f - 2.0f * (aa + cc);
		m[6] = 2.0f * (bc - ar);
		m[7] = 0.0f;

		m[8] = 2.0f * (ac - br);
		m[9] = 2.0f * (bc + ar);
		m[10] = 1.0f - 2.0f * (aa + bb);
		m[11] = 0.0f;

		m[12] = 0.f;
		m[13] = 0.f;
		m[14] = 0.f;
		m[15] = 1.f;

		return transpose(Mat4( m ));
	}

	inline bool operator==(Quaternion_arg o) const {
		return r == o.r &&
			a == o.a &&
			b == o.b &&
			c == o.c;
	}

	inline bool operator!=(Quaternion_arg o) const {
		return !(*this == o);
	}
};

inline Quaternion conjugate(Quaternion_arg  o)  {
	return Quaternion(-o.a,-o.b,-o.c, o.r);
}

inline float magnitude_squared(Quaternion_arg  v)  {
	return v.r*v.r + v.a*v.a + v.b*v.b + v.c*v.c;
}
		
inline float magnitude(Quaternion_arg v)  {
	return sqrt(magnitude_squared(v));
}

inline Quaternion operator+(Quaternion_arg l, Quaternion_arg r) {
	return Quaternion(l.a + r.a,
					  l.b + r.b,
					  l.c + r.c,
					  l.r + r.r);
}

inline Quaternion operator-(Quaternion_arg l, Quaternion_arg r)  {
	return Quaternion(l.a - r.a,
					  l.b - r.b,
					  l.c - r.c,
					  l.r - r.r);
}

inline Quaternion operator*(Quaternion_arg  l, Quaternion_arg r) {
	float tr = l.r;
	float ta = l.a;
	float tb = l.b;
	float tc = l.c;
	float nr = tr * r.r - ta * r.a - tb * r.b - tc * r.c;
	float na = tr * r.a + ta * r.r + tb * r.c - tc * r.b;
	float nb = tr * r.b + tb * r.r + tc * r.a - ta * r.c;
	float nc = tr * r.c + tc * r.r + ta * r.b - tb * r.a;
	return Quaternion(na,nb,nc,nr);
}

inline Quaternion operator*(Quaternion_arg  o, float f) {
	return Quaternion(o.a*f,o.b*f,o.c*f,o.r*f) ;
}

inline Quaternion operator*(float f, Quaternion_arg  o)  {
	return Quaternion(o.a*f,o.b*f,o.c*f,o.r*f) ;
}

inline Quaternion operator/(Quaternion_arg  lhs, float f)  {
	return lhs * (1.0f/f);
}

inline Quaternion operator/(Quaternion_arg  lhs, Quaternion_arg rhs) {
	float div = magnitude_squared(rhs);
	Quaternion q = lhs * (2.0f * rhs.r) - lhs * rhs;
	return q / div;
}

inline Quaternion inverse(Quaternion_arg v)  {
	return conjugate(v) / magnitude_squared(v);
}

inline Quaternion make_rotation(float radians, 
								Vec3_arg vec) {
	float half_angle = radians*0.5f;
	float cos_a = cos(half_angle);
	float sin_a = sin(half_angle);
	float r = cos_a;
	float a = vec.x * sin_a;
	float b = vec.y * sin_a;
	float c = vec.z * sin_a;
	return Quaternion(a,b,c,r);
}


// assumes q is unit length - otherse conjugate should be the inverse
inline Vec3 rotate(Vec3_arg p, Quaternion_arg q) {
	Quaternion result = q * Quaternion(p,0) * conjugate(q);
	return Vec3(result.a,result.b,result.c);
}

inline Quaternion exp(Quaternion_arg q) {
	float exp_r = expf(q.r);
	float m = magnitude(q);
	float inv_m = 1.0f/m;
	float cos_m = cos(m);
	float sin_m = sin(m);
	float sin_m_exp_r = sin_m * exp_r;
	float a = q.a*inv_m;
	float b = q.b*inv_m;
	float c = q.c*inv_m;
		
	return Quaternion(a * sin_m_exp_r,
					  b * sin_m_exp_r,
					  c * sin_m_exp_r, 
					  exp_r * cos_m);
}	

inline void slerp(
	Quaternion& out,
	Quaternion_arg  left,
	Quaternion_arg  right,
	float param)
{
	float cos_half_angle = left.r * right.r +
		left.a * right.a +
		left.b * right.b +
		left.c * right.c;

	if(fabs(cos_half_angle) >= 1.0f) {
		out = left;
		return;
	}
		
	float half_angle = acos(cos_half_angle);
	float sin_half_angle = sin(half_angle);
	if(fabs(sin_half_angle) < 1e-6f) {
		out = left * 0.5f + right * 0.5f;
		return;
	}
	float left_factor = sin((1.f - param) * half_angle);
	float right_factor = sin(param * half_angle);

	out = (left * left_factor + right * right_factor) / sin_half_angle;
}


#endif
