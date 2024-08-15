// Vec3.h

#ifndef _Vec3_H
#define _Vec3_H

#include <cstdio>
#include "../PGL.h"

namespace as {
class Vec3;

//==============================================================================
// Two coordinate vector
//==============================================================================
class Vec2 {
public:
	float x, y;

	Vec2();
	Vec2(float _x, float _y);
	Vec2(Vec3 const &v);
};

//==============================================================================
// Four coordinate vector
//==============================================================================
class Vec4 {
public:
	float x,y,z,w;

	Vec4();
	Vec4(float a, float b, float c, float d);
	explicit Vec4(Vec3 const &v);
	void setTo(float x, float y, float z, float w);
	float length() const;
	void normalizeInPlace();
	float get(int i) const;
	void set(int i, float value);
	float operator[](const size_t i) const;
};

//==============================================================================

inline void Vec4::setTo(float _x, float _y, float _z, float _w) {
	this->x = _x;
	this->y = _y;
	this->z = _z;
	this->w = _w;
}

inline float Vec4::get(int i) const { return (&x)[i]; }
inline void Vec4::set(int i, float value) { (&x)[i] = value; }

inline float Vec4::operator[](const size_t i) const {
	return (&x)[i];
}

//==============================================================================
// Three coordinate vector
//==============================================================================

class Vec3 {
public:
	float x,y,z;

	Vec3(float a, float b, float c);
	Vec3();
	Vec3(Vec3 const& v);
	explicit Vec3(Vec4 const& v);

	Vec3 operator+(Vec3 const& w) const;
	Vec3 operator-(Vec3 const& w) const;
	Vec3& operator=(Vec3 const& w);
	Vec3 operator*(float k) const;
	bool operator==(Vec3 const& w) const;
	float operator*(Vec3 const& w) const;
	float operator[](const size_t i) const;

	float length() const;
	float lengthSquared() const;

	Vec3 crossProduct(Vec3 const& w) const;
	Vec3 normalized() const;
	void normalizeInPlace();

	void subInPlace(const Vec3 *o);

	float angleBetween(Vec3 const& other) const;
	float angleBetweenNor(Vec3 const& normalized) const;

	float get(int i) const;

	void set(int i, float value);
	void setTo(Vec3 *o);
	void setTo(float x, float y, float z);
	
	void setTo(float f);

	void print() const;
	void print(const char *s) const;

	Vec3 translateWithMatrix(Vec3 t);
	Vec3 scaleWithMatrix(Vec3 s);

	void rot(Vec3 axis, float alpha);

	static Vec3 xaxis, yaxis, zaxis;
	static Vec3 origin;
};

Vec3 calcTriangleNormal(Vec3 v1, Vec3 v2, Vec3 v3);

//==============================================================================
inline void Vec3::subInPlace(const Vec3 *o) {
	x -= o->x;
	y -= o->y;
	z -= o->z;
}

inline float Vec3::get(int i) const { return (&x)[i]; }
inline void Vec3::set(int i, float value) { (&x)[i] = value; }

inline void Vec3::setTo(float _x, float _y, float _z) {
	this->x = _x;
	this->y = _y;
	this->z = _z;
}

inline void Vec3::setTo(float f) {
	setTo(f, f, f);
}

inline void Vec3::print() const {
	std::printf("Vec3(%f;%f;%f)\n", x,y,z);
}

inline void Vec3::print(const char *s) const {
	std::printf("%s\n", s);
	print();
}

inline Vec3& Vec3::operator=(Vec3 const& w) {
	int i;

	for (i = 0; i < 3; i++)
		(&x)[i] = w.get(i);

	return *this;
}

inline void Vec3::setTo(Vec3 *o) {
	int i;
	for (i = 0; i < 3; i++) {
		(&x)[i] = o->get(i);
	}
}

inline Vec3 Vec3::operator*(float k) const {
	return Vec3(x*k, y*k, z*k);
}

inline bool Vec3::operator==(Vec3 const& w) const {
	return x == w.x && y == w.y && z == w.z;
}

Vec3 operator*(float k, Vec3 const& v);

inline float Vec3::operator[](const size_t i) const {
	return (&x)[i];
}

} // namespace as

#endif
