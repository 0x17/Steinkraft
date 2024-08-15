// Frustum.hpp

#ifndef FRUSTUM_HPP
#define FRUSTUM_HPP

#include "Vector.hpp"

namespace as {

class BoundingBox {
public:
	BoundingBox(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);
	BoundingBox(Vec3 _min, Vec3 _max);

	Vec3 min, max;
};

class Camera;

/*	Planes are encoded as Vec3's (nx,ny,nz,d),
	where n(x,y,z) are the normal coords
	and d is the distance to the origin */
class Frustum {
public:
	Frustum(Camera *cam);

	void update();
	bool boxInFrustum(BoundingBox *box);

private:
	bool pointInPlane(Vec3 *p, Vec4 *plane) const;
	bool pointInFrustum(Vec3 *p) const;
	
	Camera *cam;

	Vec4 leftPlane, rightPlane, topPlane, bottomPlane, nearPlane, farPlane;
	Vec4 *planes[6];
};

}

#endif // FRUSTUM_HPP
