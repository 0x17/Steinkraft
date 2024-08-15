// VertexBuffer.cpp



#include "VertexBuffer.hpp"
#include "Utilities.hpp"

namespace as {

bool VertexBuffer::clStates[3] = { false, false, false };

VertexBuffer::VertexBuffer(ComponentInfo comps, bool _isStatic)
:	VertexStorage(comps),
	isStatic(_isStatic)
{
	glGenBuffers(1, &vbo);
	collectGlError();
}

VertexBuffer::~VertexBuffer() {
	glDeleteBuffers(1, &vbo);
	collectGlError();
}

void VertexBuffer::setData(const float *coords, int numCoords) {
	ndraw = numCoords / compsPerVx;
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	collectGlError();
	glBufferData(GL_ARRAY_BUFFER, (long)((int)sizeof(float)*numCoords), coords,
				 isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
	collectGlError();
}

#define OFFSET(i) ((char *)NULL+(i))

void VertexBuffer::render(int offset, int count) {
	render(offset, count, GL_TRIANGLES);
}

void VertexBuffer::render(int offset, int count, GLenum primitiveType) {
	static ulong k;

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	k = 0;

	if (comps.usePos) {
		if (!clStates[0]) {
			glEnableClientState(GL_VERTEX_ARRAY);
			clStates[0] = true;
		}
		glVertexPointer(3, GL_FLOAT, sizePerVx, 0);
		k += 3;
	} else if (clStates[0]) {
		glDisableClientState(GL_VERTEX_ARRAY);
		clStates[0] = false;
	}

	if (comps.useTexCoord) {
		if (!clStates[1]) {
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			clStates[1] = true;
		}
		glTexCoordPointer(2, GL_FLOAT, sizePerVx, OFFSET(k*sizeof(float)));
		k += 2;
	} else if (clStates[1]) {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		clStates[1] = false;
	}

	if (comps.useColor) {
		if (!clStates[2]) {
			glEnableClientState(GL_COLOR_ARRAY);
			clStates[2] = true;
		}
		glColorPointer(4, GL_FLOAT, sizePerVx, OFFSET((k)*sizeof(float)));
		k += 4;
	} else if (clStates[2]) {
		glDisableClientState(GL_COLOR_ARRAY);
		clStates[2] = false;
	}

	glDrawArrays(primitiveType, offset, (int)count);
}


} /* namespace as */
