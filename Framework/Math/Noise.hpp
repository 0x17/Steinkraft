// Noise.hpp

#ifndef NOISE_HPP
#define NOISE_HPP

#include <cmath>

#include "../Utilities.hpp"

namespace as {

float noise(float x, float y, int rval);

inline float noiseFunc(float x, float y, int rval) {
	int n = (int)x + (int)y * 57;
	n = (n << 13) ^ n;
	int nn = (n * (n * n * 60493 + rval % 90303 + 199000) + 1376312589) & 0x7fffffff;
	return 1.0f -((float)nn / 1073741824.0f);
}

inline float trigInterp(float a, float b, float x) {
	float ft = x * PI, f = (1.0f - (float)cosf((float)ft)) * 0.5f;
	return a * (1.0f - f) + b * f;
}

} // namespace as

#endif
