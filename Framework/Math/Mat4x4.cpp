// Mat4x4.cpp



#include <cstring>
#include <cstdio>
#include <cmath>

#include "Mat4x4.hpp"

namespace as {

Mat4x4 Mat4x4::identity(1.0f, 0.0f, 0.0f, 0.0f,
						0.0f, 1.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f);

Mat4x4::Mat4x4(Mat4x4 *m) {
	memcpy(this->A, m->getA(), sizeof(float) * 16);
}

Mat4x4::Mat4x4(const float *_A) {
	if (_A != NULL)
		memcpy(this->A, _A, sizeof(float) * 16);
	else
		zeroOut();
}

Mat4x4::Mat4x4() {
	zeroOut();
}

Mat4x4::Mat4x4(float a11, float a12, float a13, float a14,
			   float a21, float a22, float a23, float a24,
			   float a31, float a32, float a33, float a34,
			   float a41, float a42, float a43, float a44) {
	A[0] = a11;
	A[1] = a21;
	A[2] = a31;
	A[3] = a41;
	
	A[4] = a12;
	A[5] = a22;
	A[6] = a32;
	A[7] = a42;
	
	A[8] = a13;
	A[9] = a23;
	A[10] = a33;
	A[11] = a43;
	
	A[12] = a14;
	A[13] = a24;
	A[14] = a34;
	A[15] = a44;
}

void Mat4x4::print() const {
	std::printf("Mat4x4\n");
	for (int i = 0; i < 4; i++) {
		std::printf("(");
		for (int j = 0; j < 4; j++) {
			printf("%f; ", getCell(i,j));
		}
		std::printf(")\n");
	}
}

Mat4x4 Mat4x4::operator+(Mat4x4 const& m) const {
	Mat4x4 sum;
	int i, j;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			sum.setCell(i, j, getCell(i,j) + m.getCell(i, j));
		}
	}

	return sum;
}

Mat4x4 Mat4x4::operator-(Mat4x4 const& m) const {
	Mat4x4 delta;
	int i, j;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			delta.setCell(i, j, getCell(i,j) - m.getCell(i, j));
		}
	}

	return delta;
}

Vec3 Mat4x4::operator*(Vec3 const& v) const {
	Vec3 prod;
	int i, j;
	float sum;

	for (i = 0; i < 3; i++) {
		sum = 0.0;
		for (j = 0; j < 3; j++) {
			sum += getCell(i,j) * v.get(j);
		}
		prod.set(i, sum);
	}

	return prod;
}

Vec4 Mat4x4::operator*(Vec4 const& v) const {
	Vec4 prod;
	int i, j;
	float sum;

	for (i = 0; i < 4; i++) {
		sum = 0.0;
		for (j = 0; j < 4; j++) {
			sum += getCell(i,j) * v.get(j);
		}
		prod.set(i, sum);
	}

	return prod;
}

Mat4x4 Mat4x4::operator*(Mat4x4 const& m) const {
	Mat4x4 prod;
	int i, j, k;
	float sum;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			sum = 0.0;
			for (k = 0; k < 4; k++) {
				sum += getCell(i, k) * m.getCell(k, j);
			}
			prod.setCell(i, j, sum);
		}
	}

	return prod;
}

Mat4x4 Mat4x4::transposed() const {
	Mat4x4 m;
	int i, j;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			m.setCell(j, i, getCell(i,j));
		}
	}

	return m;
}

bool Mat4x4::invert() {
	float inv[16];
	
	inv[0]	=  A[5] * A[10] * A[15] - A[5] * A[11] * A[14] - A[9] * A[6] * A[15] + A[9] * A[7] * A[14] + A[13] * A[6] * A[11] - A[13] * A[7] * A[10];
	inv[4]	= -A[4] * A[10] * A[15] + A[4] * A[11] * A[14] + A[8] * A[6] * A[15] - A[8] * A[7] * A[14] - A[12] * A[6] * A[11] + A[12] * A[7] * A[10];
	inv[8]	=  A[4] * A[9] * A[15] - A[4] * A[11] * A[13] - A[8] * A[5] * A[15] + A[8] * A[7] * A[13] + A[12] * A[5] * A[11] - A[12] * A[7] * A[9];
	inv[12]	= -A[4] * A[9] * A[14] + A[4] * A[10] * A[13] + A[8] * A[5] * A[14] - A[8] * A[6] * A[13] - A[12] * A[5] * A[10] + A[12] * A[6] * A[9];
	inv[1]	= -A[1] * A[10] * A[15] + A[1] * A[11] * A[14] + A[9] * A[2] * A[15] - A[9] * A[3] * A[14] - A[13] * A[2] * A[11] + A[13] * A[3] * A[10];
	inv[5]	=  A[0] * A[10] * A[15] - A[0] * A[11] * A[14] - A[8] * A[2] * A[15] + A[8] * A[3] * A[14] + A[12] * A[2] * A[11] - A[12] * A[3] * A[10];
	inv[9]	= -A[0] * A[9] * A[15] + A[0] * A[11] * A[13] + A[8] * A[1] * A[15] - A[8] * A[3] * A[13] - A[12] * A[1] * A[11] + A[12] * A[3] * A[9];
	inv[13] =  A[0] * A[9] * A[14] - A[0] * A[10] * A[13] - A[8] * A[1] * A[14] + A[8] * A[2] * A[13] + A[12] * A[1] * A[10] - A[12] * A[2] * A[9];
	inv[2]	=  A[1] * A[6] * A[15] - A[1] * A[7] * A[14] - A[5] * A[2] * A[15] + A[5] * A[3] * A[14] + A[13] * A[2] * A[7] - A[13] * A[3] * A[6];
	inv[6]	= -A[0] * A[6] * A[15] + A[0] * A[7] * A[14] + A[4] * A[2] * A[15] - A[4] * A[3] * A[14] - A[12] * A[2] * A[7] + A[12] * A[3] * A[6];
	inv[10]	=  A[0] * A[5] * A[15] - A[0] * A[7] * A[13] - A[4] * A[1] * A[15] + A[4] * A[3] * A[13] + A[12] * A[1] * A[7] - A[12] * A[3] * A[5];
	inv[14]	= -A[0] * A[5] * A[14] + A[0] * A[6] * A[13] + A[4] * A[1] * A[14] - A[4] * A[2] * A[13] - A[12] * A[1] * A[6] + A[12] * A[2] * A[5];
	inv[3]	= -A[1] * A[6] * A[11] + A[1] * A[7] * A[10] + A[5] * A[2] * A[11] - A[5] * A[3] * A[10] - A[9] * A[2] * A[7] + A[9] * A[3] * A[6];
	inv[7]	=  A[0] * A[6] * A[11] - A[0] * A[7] * A[10] - A[4] * A[2] * A[11] + A[4] * A[3] * A[10] + A[8] * A[2] * A[7] - A[8] * A[3] * A[6];
	inv[11]	= -A[0] * A[5] * A[11] + A[0] * A[7] * A[9] + A[4] * A[1] * A[11] - A[4] * A[3] * A[9] - A[8] * A[1] * A[7] + A[8] * A[3] * A[5];
	inv[15] =  A[0] * A[5] * A[10] - A[0] * A[6] * A[9] - A[4] * A[1] * A[10] + A[4] * A[2] * A[9] + A[8] * A[1] * A[6] - A[8] * A[2] * A[5];

	float det = A[0] * inv[0] + A[1] * inv[4] + A[2] * inv[8] + A[3] * inv[12];
	if(det == 0.0f)
		return false;

	det = 1.0f / det;

	for(int i = 0; i < 16; i++)
		A[i] = inv[i] * det;

	return true;
}

Mat4x4& Mat4x4::operator=(Mat4x4 const& m) {
	memcpy(this->A, m.getA(), sizeof(float) * 16);
	return *this;
}

void Mat4x4::print(const char *s) const {
	std::printf("%s\n", s);
	print();
}

bool Mat4x4::operator==(Mat4x4 const& m) const {
	bool same = true;
	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			if (getCell(i,j) != m.getCell(i, j))
				same = false;
		}
	}
	return same;
}

//==============================================================================
Mat4x4 Mat4x4::translationMatrix(Vec3 &t) {
	Mat4x4 tmx = identity;
	int i;
	for (i = 0; i < 3; i++)
		tmx.setCell(i, 3, t.get(i));
	return tmx;
}

Mat4x4 Mat4x4::scaleMatrix(Vec3 &s) {
	Mat4x4 smx;
	int i;
	for (i = 0; i < 3; i++)
		smx.setCell(i, i, s.get(i));

	smx.setCell(3, 3, 1);
	return smx;
}

Mat4x4 Mat4x4::rotationMatrix(Vec3 axis, float alpha) {
	Mat4x4 rmx;
	float x, y, z, c, s;
	x = axis.x;
	y = axis.y;
	z = axis.z;
	c = (float)cosf(alpha);
	s = (float)sinf(alpha);

	// derivation found in "Mathematics for Game Developers" page 139 ff
	rmx.setCell(0, 0, x * x * (1 - c) + c);
	rmx.setCell(0, 1, x * y * (1 - c) - z * s);
	rmx.setCell(0, 2, x * z * (1 - c) + y * s);
	rmx.setCell(1, 0, y * x * (1 - c) + z * s);
	rmx.setCell(1, 1, y * y * (1 - c) + c);
	rmx.setCell(1, 2, y * z * (1 - c) - x * s);
	rmx.setCell(2, 0, x * z * (1 - c) - y * s);
	rmx.setCell(2, 1, y * z * (1 - c) + x * s);
	rmx.setCell(2, 2, z * z * (1 - c) + c);
	rmx.setCell(3, 3, 1);

	return rmx;
}

//==============================================================================

Vec3 Mat4x4::translateVecWithMatrix(Vec3 &v, Vec3 &t) {
	Mat4x4 tmx = translationMatrix(t);
	return tmx * v;
}

Vec3 Mat4x4::scaleVecWithMatrix(Vec3 &v, Vec3 &s) {
	Mat4x4 smx = scaleMatrix(s);
	return smx * v;
}
//==============================================================================

}
