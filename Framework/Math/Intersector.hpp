// Intersector.hpp

#ifndef INTERSECTOR_HPP
#define INTERSECTOR_HPP

#include <vector>

#include "Vector.hpp"

namespace as {

class Ray {
public:
	Vec3 origin, direction;

	explicit Ray(Vec3 _origin, Vec3 _direction)
	: origin(_origin), direction(_direction) {}
};

namespace intersector {
bool intersectRayTriangle(Ray *ray, Vec3 t1, Vec3 t2, Vec3 t3, Vec3 *isection);
bool intersectRayTriangles(Ray *ray, std::vector<float> *tris, Vec3 *isection);
}

}

#endif
