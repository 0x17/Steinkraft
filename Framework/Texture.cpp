// Texture.cpp



#include <cstdio>
#include <cstring>

#include "../Constants.h"

#include "PGL.h"
#include "Utilities.hpp"
#include "Texture.hpp"

#if !MOBILE

#if WIN32
#include "../stb_image.h"
#else
#include "stb_image.h"
#endif

namespace as {

const int PSIZE = 64;

Texture::Texture(const char *filename, bool nearest) {
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);

	// use procedural checker pattern when no filename is given
	if (!filename) {
		uchar *data = new uchar[PSIZE*PSIZE * 3];
		memset(data, 0, sizeof(uchar) * PSIZE*PSIZE * 3);

		int i, j;

		for (i = 0; i < PSIZE; i++)
			for (j = 0; j < PSIZE; j++)
				data[(i*PSIZE+j)*3] = data[(i*PSIZE+j)*3+1]
									  = data[(i*PSIZE+j)*3+2] =
											(((i & 0x8) == 0) ^((j & 0x8) == 0)) * 255;

		glTexImage2D(GL_TEXTURE_2D, 0, 3, PSIZE, PSIZE, 0, GL_RGB,
					 GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		delete [] data;
	} else {
		FILE *fp = fopen(filename, "rb");
		if (!fp) {
			char sbuf[256];
			std::sprintf(sbuf, "Unable to load texture: %s!", filename);
			error(sbuf);
		}
		int comp, width, height;
		uchar *data = stbi_load_from_file(fp, &width, &height, &comp, 0);
		fclose(fp);

		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA,
						  width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

		GLint p = nearest ? GL_NEAREST : GL_LINEAR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, p);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, p);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		delete [] data;
	}
}

Texture::~Texture() {
	glDeleteTextures(1, &texId);
}

} // namespace as
	
#endif // !MOBILE
