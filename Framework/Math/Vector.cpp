// Vec3.cpp

#include "StdAfx.h"
#pragma hdrstop

#include <cmath>
#include <cstring>

#include "Vector.hpp"
#include "Mat4x4.hpp"

#include "../Utilities.hpp"

namespace as {

//==============================================================================
// Two coordinate vector
//==============================================================================
Vec2::Vec2( float _x, float _y ) : x(_x), y(_y) {}
Vec2::Vec2( Vec3 const &v ) : x(v.x), y(v.y) {}

//==============================================================================
// Four coordinate vector
//==============================================================================

Vec4::Vec4() {
	std::memset((&x), 0, sizeof(float)*4);
}

Vec4::Vec4(float a, float b, float c, float d) {
	this->x = a;
	this->y = b;
	this->z = c;
	this->w = d;
}

Vec4::Vec4( Vec3 const &v ) : x(v.x), y(v.y), z(v.z), w(1.0f)
{}


float Vec4::length() const {
	return sqrtf(x*x + y*y + z*z);
}

void Vec4::normalizeInPlace() {
	float l = length();
	x /= l;
	y /= l;
	z /= l;
	w /= l;
}

//==============================================================================
// Three coordinate vector
//==============================================================================

Vec3 Vec3::origin(0.0f, 0.0f, 0.0f);
Vec3 Vec3::xaxis(1.0f, 0.0f, 0.0f);
Vec3 Vec3::yaxis(0.0f, 1.0f, 0.0f);
Vec3 Vec3::zaxis(0.0f, 0.0f, 1.0f);

Vec3::Vec3(float a, float b, float c) {
	x = a;
	y = b;
	z = c;
}

Vec3::Vec3() {
	memset((&x), 0, sizeof(float) * 3);
}

Vec3::Vec3(Vec3 const& _v) {
	for (int i = 0; i < 3; i++)
		(&x)[i] = _v.get(i);
}

Vec3::Vec3( Vec4 const& v ) : x(v.x), y(v.y), z(v.z)
{
}


//==============================================================================
Vec3 Vec3::operator+(Vec3 const& w) const {
	return Vec3(x + w.x, y + w.y, z + w.z);
}

Vec3 Vec3::operator-(Vec3 const& w) const {
	return Vec3(x - w.x, y - w.y, z - w.z);
}

float Vec3::operator*(Vec3 const& w) const {
	return x*w.x + y*w.y + z*w.z;
}

float Vec3::length() const {
	return sqrtf(x*x + y*y + z*z);
}

float Vec3::lengthSquared() const {
	return x*x+y*y+z*z;
}

Vec3 operator*(float k, Vec3 const& v) {
	return v*k;
}

Vec3 Vec3::crossProduct(Vec3 const& w) const {
	return Vec3(y*w.z - z*w.y,
				z*w.x - x*w.z,
				x*w.y - y*w.x);
}

Vec3 Vec3::normalized() const {
	return 1 / length() * (*this);
}

void Vec3::normalizeInPlace() {
	float l = length();
	x /= l;
	y /= l;
	z /= l;
}

float Vec3::angleBetween(Vec3 const& other) const {
	static float l, ol;
	l = length();
	ol = other.length();
	if (!l && !ol) return 0; // prevent div by zero
	return acosf(((*this) * other) / (l*ol));
}

float Vec3::angleBetweenNor(Vec3 const& normalized) const {
	static float l;
	l = length();
	if (!l) return 0; // prevent div by zero
	return acosf(((*this) * normalized) / l);
}

//==============================================================================
Vec3 Vec3::translateWithMatrix(Vec3 t) {
	return Mat4x4::translateVecWithMatrix(*this, t);
}

Vec3 Vec3::scaleWithMatrix(Vec3 s) {
	return Mat4x4::scaleVecWithMatrix(*this, s);
}

void Vec3::rot(Vec3 axis, float alpha) {
	Mat4x4 rmx = Mat4x4::rotationMatrix(axis, alpha);
	Vec3 prod = rmx * (*this);
	for (int i = 0; i < 3; i++)
		(&x)[i] = prod.get(i);
}

//==============================================================================
Vec3 calcTriangleNormal(Vec3 v1, Vec3 v2, Vec3 v3) {
	return ((v3 - v1).crossProduct((v2 - v1))).normalized();
}

}
