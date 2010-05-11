#ifndef INCLUDED_gui_editcam_HH
#define INCLUDED_gui_editcam_HH

#include "cam.hh"
#include "Vector.hh"

// camera centered at a target, with the camera sitting on a 
// line defined by a pitch/yaw at that target.
class EditorCamera
{
public:
	EditorCamera();

	void SetZClip(float znear, float zfar);
	void SetDimensions(float w, float h);                     
	void GetDimensions(float &w, float &h) const;

	void SetFOV(float fov);
	void SetTarget(float x, float y, float z);
	const Vec3& GetTarget() const { return m_target; }
	void SetZoom(float dist);
	float GetZoom() const { return m_zoom; }

	void SetYaw(float angle);
	void SetPitch(float angle);
	inline float GetYaw() const { return m_yaw; }
	inline float GetPitch() const { return m_pitch; }

	void MoveBy(float x, float y, float z);
	void MoveBy(Vec3 const& dir);
	void ZoomBy(float delta);
	void RotateYaw(float delta);
	void RotatePitch(float delta);

	void Draw();

	Mat4 GetMatrix() const { return m_camera.GetMatrix(); }
	const Vec3& GetLookDir() const { return m_camera.GetLookDir(); }
	const Vec3& GetPosition() const { return m_camera.GetPosition(); }
	Vec3 GetDirectionFromScreen(float x, float y) { return m_camera.GetDirectionFromScreen(x,y); }
private:
	Camera m_camera;         

	Vec3 m_target;
	float m_yaw;
	float m_pitch;
	float m_zoom;
};


#endif
