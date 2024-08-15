// Camera.hpp

#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "Math/Vector.hpp"
#include "Math/Mat4x4.hpp"
#include "Math/Frustum.hpp"
#include "Observable.hpp"

namespace as {

class Ray;

//======================================================================
// Base camera
//======================================================================
class BaseCamera {
public:
	virtual ~BaseCamera() {}
	
	const float *getViewMatrix() const;
	const float *getProjMatrix() const;
	
	const Mat4x4 *getViewMatrixObj() const;
	const Mat4x4 *getProjMatrixObj() const;
	
	void apply() const;
	void applyView() const;
	
protected:
	Mat4x4 vmx, pmx;
};

inline const float *BaseCamera::getViewMatrix() const {
	return vmx.getA();
}

inline const float *BaseCamera::getProjMatrix() const {
	return pmx.getA();
}

inline const Mat4x4 *BaseCamera::getViewMatrixObj() const {
	return &vmx;
}

inline const Mat4x4 *BaseCamera::getProjMatrixObj() const {
	return &pmx;
}

inline void BaseCamera::apply() const {
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(pmx.getA());
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(vmx.getA());
}

inline void BaseCamera::applyView() const {
	glLoadMatrixf(vmx.getA());
}

//======================================================================
// Orthographic camera
//======================================================================
class OrthoCamera : public BaseCamera {
public:
	OrthoCamera();
	OrthoCamera(float left, float right, float bottom, float top, float znear, float zfar);
	virtual ~OrthoCamera() {}
	
private:
	void calcProjection(float left, float right, float bottom, float top, float znear, float zfar);
};

//======================================================================
// Perspective camera
//======================================================================
class Camera : public Observable<Vec3>, public BaseCamera {
public:
	Camera(float fov, Vec3 pos, Vec3 normal, Vec3 lookUp);
	virtual ~Camera();

	// Getters/Setters
	float getFov() const;
	Vec3 getLookUp() const;
	Vec3 getNormal() const;
	Vec3 *getNormalPtr();
	Vec3 getPos() const;
	Vec3 *getPosPtr();
	Vec3 getU() const;
	Vec3 *getUPtr();

	void setFov(float fov);
	void setLookUp(Vec3 lookUp);
	void setNormal(Vec3 normal);
	void setPos(Vec3 pos);
	
	void updateView();

	void moveForward();
	void moveBackward();
	void moveUp(float c);
	void moveDown(float c);
	void moveLeft();
	void moveRight();

	void rotate(Vec3 axis, float alpha);

	Ray getPickRay(int x, int y) const;
	void project(Vec4 objPos, Vec3 *scrPos) const;

	void restrictToBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);

	float getYaw() const;

	Frustum *getFrustumPtr();

private:
	void calcProjection();
	void unproject(Vec4 scrPos, const int *viewport, Vec3 *objPos) const;
	bool project(Vec4 objPos, const int *viewport, Vec3 *scrPos) const;
	void determineViewport(int *viewport) const;

	Vec3 pos, lookUp, normal, u;
	float fov;
	float yaw;

	Frustum *frustum;
};

inline Frustum *Camera::getFrustumPtr() {
	return frustum;
}

inline float Camera::getYaw() const {
	return yaw;
}

inline float Camera::getFov() const {
	return fov;
}

inline Vec3 Camera::getLookUp() const {
	return lookUp;
}

inline Vec3 Camera::getNormal() const {
	return normal;
}

inline Vec3 *Camera::getNormalPtr() {
	return &normal;
}

inline Vec3 Camera::getPos() const {
	return pos;
}

inline Vec3 *Camera::getPosPtr() {
	return &pos;
}

inline void Camera::setFov(float _fov) {
	this->fov = _fov;
}

inline void Camera::setLookUp(Vec3 _lookUp) {
	this->lookUp = _lookUp;
	this->u = normal.crossProduct(_lookUp);
}

inline void Camera::setNormal(Vec3 _normal) {
	this->normal = _normal;
	this->u = _normal.crossProduct(lookUp);
}

inline void Camera::setPos(Vec3 _pos) {
	this->pos = _pos;
	notifyObservers(&_pos);
}

inline void Camera::moveForward() {
	setPos(pos + normal*0.25);
}

inline void Camera::moveBackward() {
	setPos(pos - normal*0.25);
}

inline void Camera::moveLeft() {
	setPos(pos - u*0.25);
}

inline void Camera::moveRight() {
	setPos(pos + u*0.25);
}

inline void Camera::moveUp(float c) {
	setPos(pos + Vec3(0, 1, 0)*c);
}

inline void Camera::moveDown(float c) {
	setPos(pos - Vec3(0, 1, 0)*c);
}

inline void Camera::rotate(Vec3 axis, float alpha) {
	static Vec3 oldLookUp;

	if (axis.x == 0.0f && axis.y == 1.0f && axis.z == 0.0f)
		yaw += alpha;

	oldLookUp.setTo(&lookUp);

	lookUp.rot(axis, alpha);

	// don't go upside down!
	if (lookUp.y < 0) {
		lookUp.setTo(&oldLookUp);
		return;
	}

	normal.rot(axis, alpha);
	u = normal.crossProduct(lookUp).normalized();
}

inline Vec3 Camera::getU() const {
	return u;
}

inline Vec3 *Camera::getUPtr() {
	return &u;
}

inline void Camera::restrictToBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
	if (pos.x < minX) pos.x = minX;
	if (pos.y < minY) pos.y = minY;
	if (pos.z < minZ) pos.z = minZ;
	if (pos.x >= maxX) pos.x = maxX;
	if (pos.y >= maxY) pos.y = maxY;
	if (pos.z >= maxZ) pos.z = maxZ;
}

}

#endif
