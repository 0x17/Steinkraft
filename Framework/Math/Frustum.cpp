// Frustum.cpp

#include "StdAfx.h"
#pragma hdrstop

#include "../PGL.h"
#include "../Utilities.hpp"

#include "Vector.hpp"
#include "Mat4x4.hpp"

#include "Frustum.hpp"

#include "../Camera.hpp"

namespace as {
//===========================================================================
// Methods
//===========================================================================
BoundingBox::BoundingBox(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
		: min(Vec3(minX, minY, minZ)), max(Vec3(maxX, maxY, maxZ)) {}

BoundingBox::BoundingBox( Vec3 _min, Vec3 _max ) : min(_min), max(_max) {}

Frustum::Frustum(Camera *_cam) : cam(_cam) {
	planes[0] = &leftPlane;
	planes[1] = &rightPlane;
	planes[2] = &bottomPlane;
	planes[3] = &topPlane;
	planes[4] = &nearPlane;
	planes[5] = &farPlane;
}

void Frustum::update() {
	const float *m;
	
	Mat4x4 prod = (*cam->getProjMatrixObj()) * (*cam->getViewMatrixObj());
	
	m = prod.getA();

	leftPlane.setTo(m[3] + m[0], m[7] + m[4], m[11] + m[8], m[15] + m[12]);
	rightPlane.setTo(m[3] - m[0], m[7] - m[4], m[11] - m[8], m[15] - m[12]);

	bottomPlane.setTo(m[3] + m[1], m[7] + m[5], m[11] + m[9], m[15] + m[13]);
	topPlane.setTo(m[3] - m[1], m[7] - m[5], m[11] - m[9], m[15] - m[13]);

	nearPlane.setTo(m[3] + m[2], m[7] + m[6], m[11] + m[10], m[15] + m[14]);
	farPlane.setTo(m[3] - m[2], m[7] - m[6], m[11] - m[10], m[15] - m[14]);

	leftPlane.normalizeInPlace();
	rightPlane.normalizeInPlace();
	bottomPlane.normalizeInPlace();
	topPlane.normalizeInPlace();
	nearPlane.normalizeInPlace();
	farPlane.normalizeInPlace();
}

inline bool Frustum::pointInPlane(Vec3 *p, Vec4 *plane) const {
	return (plane->x * p->x
			+ plane->y * p->y
			+ plane->z * p->z
			+ plane->w) <= 0;
}

inline bool Frustum::pointInFrustum(Vec3 *p) const {
	for (int i = 0; i < 6; i++) {
		if (pointInPlane(p, planes[i])) {
			return false;
		}
	}
	return true;
}

bool Frustum::boxInFrustum(BoundingBox *box) {
	static Vec3 points[8];
	static bool allInside;

	points[0] = Vec3(box->min.x, box->max.y, box->max.z);
	points[1] = Vec3(box->min.x, box->min.y, box->max.z);
	points[2] = Vec3(box->max.x, box->min.y, box->max.z);
	points[3] = Vec3(box->max.x, box->max.y, box->max.z);
	points[4] = Vec3(box->max.x, box->max.y, box->min.z);
	points[5] = Vec3(box->max.x, box->min.y, box->min.z);
	points[6] = Vec3(box->min.x, box->min.y, box->min.z);
	points[7] = Vec3(box->min.x, box->max.y, box->min.z);

	for (int j = 0; j < 6; j++) { // for each plane
		allInside = true;
		for (int i = 0; i < 8; i++) { // for each point
			allInside &= (pointInPlane(&points[i], planes[j]));
			if (!allInside)
				break;
		}
		if (allInside) return false;
	}

	return true;
}

}
