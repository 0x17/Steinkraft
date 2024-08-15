// LightSource.cpp



#include "LightSource.hpp"

namespace as {

Color Color::white(1.0f, 1.0f, 1.0f);

LightSource::LightSource(Vec3 _pos, Color _color, GLenum _lightId)
:	lightId(_lightId),
	color(_color),
	pos(_pos)
{
	float itsPos[4], itsCol[4];

	for(int i = 0; i < 4; i++)
		itsPos[i] = (float)pos.get(i);

	itsCol[0] = color.r;
	itsCol[1] = color.g;
	itsCol[2] = color.b;
	itsCol[3] = 1.0;

	glLightfv(lightId, GL_POSITION, itsPos);
	glLightfv(lightId, GL_DIFFUSE, itsCol);
	glLightfv(lightId, GL_SPECULAR, itsCol);

	glLightfv(lightId, GL_AMBIENT, itsCol);

	glLightf(lightId, GL_CONSTANT_ATTENUATION, 0);
	glLightf(lightId, GL_LINEAR_ATTENUATION, 0);
	glLightf(lightId, GL_QUADRATIC_ATTENUATION, 0.01f);
}

void LightSource::enable() {
	glEnable(lightId);
}

void LightSource::disable() {
	glDisable(lightId);
}

void LightSource::setPos(Vec3 _pos) {
	float itsPos[4];
	this->pos = _pos;
	for(int i = 0; i < 4; i++) {
		itsPos[i] = (float)_pos.get(i);
	}
	glLightfv(lightId, GL_POSITION, itsPos);
}

void LightSource::translate(Vec3 v) {
	setPos(pos + v);
}

}
