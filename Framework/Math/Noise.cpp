// Noise.cpp



#include "Noise.hpp"

namespace as {

float noise(float x, float y, int rval) {
	float floorX = (float)((int)x), floorY = (float)((int)y);
	float s, t, u, v, a, b;

	s = noiseFunc(floorX, floorY, rval);
	t = noiseFunc(floorX + 1, floorY, rval);
	u = noiseFunc(floorX, floorY + 1, rval);
	v = noiseFunc(floorX + 1, floorY + 1, rval);

	a = trigInterp(s, t, x - floorX);
	b = trigInterp(u, v, x - floorX);

	return trigInterp(a, b, y - floorY);
}

} // namespace as
