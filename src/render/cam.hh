#ifndef INCLUDED_gui_cam_HH
#define INCLUDED_gui_cam_HH

#include "Vector.hh"
#include "Mat4.hh"

class Camera
{
public:
	Camera();
	void Draw() const;
	void LookAt(const Vec3& pos);
                        
	void SetFOV(float fov);
	float GetFOV() const { return m_fov; }

	void SetDimensions(float w, float h);
	float GetW() const { return m_w; }
	float GetH() const { return m_h; }

	void SetZClip(float znear, float zfar);
	float GetNear() const { return m_near; }
	float GetFar() const { return m_far; }

	void SetPosition(const Vec3& position);          
	const Vec3& GetPosition() const { return m_pos; }

	void SetLookDir(const Vec3& lookdir);
	const Vec3& GetLookDir() const { return m_lookdir; }

	Mat4 GetMatrix() const;
private:
	float m_fov, m_w, m_h, m_near, m_far;
	Vec3 m_pos, m_lookdir;
};


#endif
