// Texture.hpp

#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#if !MOBILE

#include "PGL.h"

namespace as {

class Texture {
public:
	explicit Texture(const char *filename = NULL, bool nearest = false);
	~Texture();

	void bind() { glBindTexture(GL_TEXTURE_2D, texId); }

	GLuint getId() const { return texId; }

private:
	GLuint texId;
};

}

#endif // !MOBILE
#endif // TEXTURE_HPP
