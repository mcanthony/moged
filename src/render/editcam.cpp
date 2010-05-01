#include "editcam.hh"
#include "MathUtil.hh"

EditorCamera::EditorCamera()
	: m_target(0.f, 0.f, 0.f)
	, m_yaw(0.f)
	, m_pitch(0.f)
	, m_zoom(10.f)
{
                
}

void EditorCamera::SetZClip(float znear, float zfar)
{
	m_camera.SetZClip(znear, zfar);
}

void EditorCamera::SetDimensions(float w, float h)
{
	m_camera.SetDimensions(w,h);
}

void EditorCamera::GetDimensions(float &w, float &h) const
{
	w = m_camera.GetW();
	h = m_camera.GetH();
}

void EditorCamera::SetFOV(float fov)
{
	m_camera.SetFOV(fov);
}

void EditorCamera::SetTarget(float x, float y, float z)
{
	m_target.x = x;
	m_target.y = y;
	m_target.z = z;
}

void EditorCamera::SetZoom(float dist)
{
	dist = Max(dist,m_camera.GetNear());
	m_zoom = dist;          
}

void EditorCamera::SetYaw(float angle)
{
	m_yaw = ClampAngleDeg(angle);
}

void EditorCamera::SetPitch(float angle)
{
	m_pitch = ClampAngleDeg(angle);
}

void EditorCamera::MoveBy(float x, float y, float z)
{
	m_target.x += x;
	m_target.y += y;
	m_target.z += z;
}

void EditorCamera::MoveBy(Vec3 const& dir)
{
	m_target += dir;
}

void EditorCamera::ZoomBy(float delta)
{
	SetZoom(m_zoom + delta);
}

void EditorCamera::RotateYaw(float delta)
{
	SetYaw(m_yaw + delta);
}

void EditorCamera::RotatePitch(float delta)
{
	SetPitch(m_pitch + delta);
}
                
void EditorCamera::Draw()
{
	Mat4 to_look_at = rotation_y(DegToRad(m_yaw)) * rotation_x(DegToRad(m_pitch));
	Vec3 vec = m_zoom * get_column(to_look_at, 2);               
	m_camera.SetPosition(m_target+vec);
	m_camera.SetLookDir(-vec);
	m_camera.Draw();
}


