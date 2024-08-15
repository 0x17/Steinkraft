// Camera.cpp



#include <cstring>
#include <cmath>

#include "Utilities.hpp"

#include "Math/Intersector.hpp"
#include "Math/Mat4x4.hpp"

#include "Camera.hpp"

namespace as {

//======================================================================
// Orthographic camera
//======================================================================
OrthoCamera::OrthoCamera() {
	calcProjection(0.0f, (float)SCR_W, 0.0f, (float)SCR_H, -1.0f, 100.0f);
	vmx = Mat4x4::identity;
}

OrthoCamera::OrthoCamera(float left, float right, float bottom, float top, float znear, float zfar) {
	calcProjection(left, right, bottom, top, znear, zfar);
	vmx = Mat4x4::identity;
}

#if IPHONE
#define CORR_IPHONE_ROTATION() pmx = Mat4x4::rotationMatrix(Vec3::zaxis, deg2rad(-90.0f)) * pmx;
#else
#define CORR_IPHONE_ROTATION() ;
#endif

void OrthoCamera::calcProjection(float left, float right, float bottom, float top, float znear, float zfar) {
	pmx.setCell(0,0, 2 / (right - left));
	pmx.setCell(1,1, 2 / (top - bottom));
	pmx.setCell(2,2, -2 / (zfar - znear));
	pmx.setCell(0,3, -(right + left) / (right - left));
	pmx.setCell(1,3, -(top + bottom) / (top - bottom));
	pmx.setCell(2,3, -(zfar + znear) / (zfar - znear));
	pmx.setCell(3,3, 1);
	
	//CORR_IPHONE_ROTATION();
}

//======================================================================
// Perspective camera
//======================================================================
Camera::Camera(float _fov, Vec3 _pos, Vec3 _normal, Vec3 _lookUp)
:	pos(_pos),
	lookUp(_lookUp),
	normal(_normal),
	u(_normal.crossProduct(_lookUp)),
	fov(_fov),
	yaw(0)
{
	frustum = new Frustum(this);

	calcProjection();
	updateView();
}

void Camera::calcProjection() {
	float	aspect = (float)SCR_W / (float)SCR_H,
			znear = NEAR_PLANE,
			zfar = FAR_PLANE,
			f = 1.0f / tanf(deg2rad(fov));
	
	pmx.setCell(0,0, f / aspect);
	pmx.setCell(1,1, f);
	pmx.setCell(2,2, (zfar + znear) / (znear - zfar));
	pmx.setCell(3,2, -1);
	pmx.setCell(2,3, (2 * zfar * znear) / (znear - zfar));
	
	//CORR_IPHONE_ROTATION();
}

Camera::~Camera() {
	SAFE_DELETE(frustum);
}

void Camera::updateView() {
	Mat4x4 wmx, cmx;

/*#if !MOBILE
	gluLookAt(pos.x, pos.y, pos.z,
			  pos.x + normal.x,
			  pos.y + normal.y,
			  pos.z + normal.z,
			  lookUp.x, lookUp.y, lookUp.z);
	return Mat4x4::identity()->getA();
#endif*/

	// translations depending on position
	int i;
	for (i = 0; i < 4; i++) {
		cmx.setCell(i, i, 1.0);
		if (i < 3) {
			cmx.setCell(i, 3, -pos.get(i));
		}
	}

	// rotations depending on normal vector
	for (i = 0; i < 3; i++)  {
		wmx.setCell(0, i, u.get(i));
		wmx.setCell(1, i, lookUp.get(i));
		wmx.setCell(2, i, -normal.get(i));
	}

	wmx.setCell(3, 3, 1.0);

	vmx = wmx * cmx;
}

Ray Camera::getPickRay(int x, int y) const {
	Vec3 obj;
	int viewport[4];
	determineViewport(viewport);

	unproject(Vec4((float)x, (float)y, 0.0f, 1.0f), viewport, &obj);
	Vec3 origin = obj;
	unproject(Vec4((float)x, (float)y, 1.0f, 1.0f), viewport, &obj);
	Vec3 direction = (obj - origin).normalized();

	return Ray(origin, direction);
}

void Camera::project( Vec4 objPos, Vec3 *scrPos ) const
{
	int viewport[4];
	determineViewport(viewport);
	project(objPos, viewport, scrPos);
	
	// TODO: Move all this Android/iPhone coordinate adaption
	// crap to some central place it's ugly
/*#if IPHONE
	int x1 = (int)scrPos->x, y1 = (int)scrPos->y;
	int x, y;
	if (RETINA) { x1 >>= 1; y1 >>= 1; }
	x = SCR_W - y1;
	y = x1;
	scrPos->x = x;
	scrPos->y = y;
#endif*/
}

void Camera::unproject(Vec4 scrPos, const int *viewport, Vec3 *objPos) const {
	Vec4 out;

	Mat4x4 final = pmx * vmx;
	if(!final.invert()) return;

	// Map x and y from window coordinates
	scrPos.x = (scrPos[0] - viewport[0]) / viewport[2];
	scrPos.y = (scrPos[1] - viewport[1]) / viewport[3];

	// Map to range -1 to 1
	scrPos.x = scrPos[0] * 2 - 1;
	scrPos.y = scrPos[1] * 2 - 1;
	scrPos.z = scrPos[2] * 2 - 1;

	out = final * scrPos;
	
	if (out[3] == 0.0) return;
	out.x = out[0] / out[3];
	out.y = out[1] / out[3];
	out.z = out[2] / out[3];
	objPos->setTo(out[0], out[1], out[2]);
}

bool Camera::project( Vec4 objPos, const int *viewport, Vec3 *scrPos ) const
{
	Vec4 out;
	
	Mat4x4 final = pmx * vmx;

	out = final * objPos;
	if (out[3] == 0.0f)
		return false;

	out.x = out[0] / out[3];
	out.y = out[1] / out[3];
	out.z = out[2] / out[3];

	scrPos->x = viewport[0] + (1.0f + out[0]) * viewport[2] / 2.0f;
	scrPos->y = viewport[1] + (1.0f + out[1]) * viewport[3] / 2.0f;
	scrPos->z = (1.0f + out[2]) / 2.0f;
	
	if (RETINA) {
		int x = (int)scrPos->x;
		int y = (int)scrPos->y;
		x >>= 1;
		y >>= 1;
		scrPos->x = (float)x;
		scrPos->y = (float)y;
	}

	return true;
}

void Camera::determineViewport( int *viewport ) const
{
#if IPHONE // use glGet on iOS since it needs 90 degree rotation
	glGetIntegerv(GL_VIEWPORT, viewport);
#else
	// glGetFloatv doesn't work on software rasterizers (e.g. Wildfire)
	viewport[0] = 0;
	viewport[1] = 0;
	viewport[2] = SCR_W;
	viewport[3] = SCR_H;
#endif // IPHONE
}

}
