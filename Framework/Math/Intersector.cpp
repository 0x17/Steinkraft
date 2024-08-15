// Intersector.cpp

#include "StdAfx.h"
#pragma hdrstop

#include <cfloat>

#include "Intersector.hpp"
#include "../Utilities.hpp"

namespace as {
namespace intersector {
bool intersectRayTriangle(Ray *ray, Vec3 t1, Vec3 t2, Vec3 t3, Vec3 *isection) {
	// TAKEN FROM THE BOOK: Physically Based Rendering, by Matt Pharr

	// Compute $\VEC{s}_1$
	// Get triangle vertices in _p1_, _p2_, and _p3_
	Vec3 e1 = t2 - t1;
	Vec3 e2 = t3 - t1;
	Vec3 s1 = ray->direction.crossProduct(e2);

	float divisor = s1 * e1;
	if (divisor == 0.)
		return false;

	float invDivisor = 1.f / divisor;

	// Compute first barycentric coordinate
	Vec3 d = ray->origin - t1;
	float b1 = (d * s1) * invDivisor;
	if (b1 < 0. || b1 > 1.)
		return false;
	// Compute second barycentric coordinate
	Vec3 s2 = d.crossProduct(e1);
	float b2 = (ray->direction * s2) * invDivisor;
	if (b2 < 0. || b1 + b2 > 1.)
		return false;
	// Compute _t_ to intersection point
	float t = (e2 * s2) * invDivisor;
	if (t < 0) //|| t > R.maxT)
		return false;
	*isection = ray->origin + (ray->direction * t);
	return true;
}

bool intersectRayTriangles(Ray *ray, std::vector<float> *tris, Vec3 *isection) {
	Vec3 t1, t2, t3, in;
	float minDist = FLT_MAX, dist;
	Vec3 candidate;
	bool candidateFound = false;

	for (uint i = 0; i < tris->size(); i += 9) {
		t1.setTo((*tris)[i], (*tris)[i+1], (*tris)[i+2]);
		t2.setTo((*tris)[i+3], (*tris)[i+4], (*tris)[i+5]);
		t3.setTo((*tris)[i+6], (*tris)[i+7], (*tris)[i+8]);

		if (intersectRayTriangle(ray, t1, t2, t3, &in)) {
			dist = (ray->origin - in).length();

			if (dist < minDist) {
				minDist = dist;
				candidate = Vec3(in);
				candidateFound = true;
			}
		}
	}

	if (candidateFound) {
		isection->setTo(&candidate);
		return true;
	}

	return false;
}

}
}
