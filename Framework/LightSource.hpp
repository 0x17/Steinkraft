// LightSource.hpp

#ifndef LIGHTSOURCE_HPP
#define LIGHTSOURCE_HPP

#include "Math/Vector.hpp"
#include "Utilities.hpp"

namespace as {

struct Color {
	Color(float _r, float _g, float _b)
			: r(_r), g(_g), b(_b) {}

	Color()
			: r(0), g(0), b(0) {}

	float r, g, b;
	
	static Color white;
};

class LightSource {
public:
	LightSource(Vec3 pos, Color color, GLenum lightId);

	void enable();
	void disable();

	void setPos(Vec3 pos);
	void translate(Vec3 v);

	// FIXME: Still ugly hack to transform with cam
	void update() { setPos(pos); }

	GLenum getId() { return lightId; }

private:
	GLenum lightId;
	Color color;
	Vec3 pos;
};

}

#endif
