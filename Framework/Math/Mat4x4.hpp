// Mat4x4.h

#ifndef _MAT4X4_H_
#define _MAT4X4_H_

#include <cstring>

#include "Vector.hpp"

namespace as {

class Mat4x4 {
public:
	explicit Mat4x4(const float *A);
	explicit Mat4x4(Mat4x4 *m);
	Mat4x4(float a11, float a12, float a13, float a14,
		   float a21, float a22, float a23, float a24,
		   float a31, float a32, float a33, float a34,
		   float a41, float a42, float a43, float a44);
	Mat4x4();

	const float * const getA() const;

	void setCell(int row, int col, float value);
	float getCell(int row, int col) const;

	Mat4x4 operator+(Mat4x4 const& m) const;
	Mat4x4 operator-(Mat4x4 const& m) const;
	Mat4x4 operator*(Mat4x4 const& m) const;
	Vec3 operator*(Vec3 const& v) const;
	Vec4 operator*(Vec4 const& v) const;

	Mat4x4& operator=(Mat4x4 const& m);

	bool operator==(Mat4x4 const& m) const;

	Mat4x4 transposed() const;
	
	bool invert();

	void print() const;
	void print(const char *s) const;
	
	static Mat4x4 identity;
	
	static Mat4x4 translationMatrix(Vec3 &t);
	static Mat4x4 scaleMatrix(Vec3 &s);
	static Mat4x4 rotationMatrix(Vec3 axis, float alpha);

	static Vec3 translateVecWithMatrix(Vec3 &v, Vec3 &t);
	static Vec3 scaleVecWithMatrix(Vec3 &v, Vec3 &s);

private:
	void zeroOut();

	float A[16];

};

inline const float * const Mat4x4::getA() const {
	return &A[0];
}

inline void Mat4x4::zeroOut() {
	std::memset(this->A, 0, sizeof(float) * 16);
}

inline void Mat4x4::setCell(int row, int col, float value) {
	A[row+col*4] = value;
}

inline float Mat4x4::getCell(int row, int col) const {
	return A[row+col*4];
}

}
#endif
