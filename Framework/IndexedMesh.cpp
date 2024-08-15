// IndexedMesh.cpp



#include "PGL.h"
#include "IndexedMesh.hpp"
#include "Utilities.hpp"

namespace as {

bool IndexedMesh::clStates[3] = { false, false, false };
#define OFFSET(i) ((char *)NULL+(i))

IndexedMesh::IndexedMesh( ComponentInfo _comps, bool _isStatic )
:	isStatic(_isStatic),
	comps(_comps),
	compsPerVx(comps.getCompsPerVx()),
	sizePerVx(compsPerVx * (int)sizeof(float))
{
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);
	collectGlError();
}

IndexedMesh::~IndexedMesh()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	collectGlError();
}

void IndexedMesh::setVertices( const float *coords, int numCoords, const ushort *indices, int numIndices )
{
	ndraw = numIndices;
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	collectGlError();
	glBufferData(GL_ARRAY_BUFFER, (long)((int)sizeof(float) * numCoords), coords, isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
	collectGlError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	collectGlError();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (long)((int)sizeof(ushort) * numIndices), indices, isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
	collectGlError();
}

void IndexedMesh::render( int offset, int count, GLenum primitiveType ) const
{
	static ulong k;

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

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

#if IPHONE || ANDROID
	glDrawElements(primitiveType, count, GL_UNSIGNED_SHORT, 0);
#else
	glDrawRangeElements(GL_TRIANGLES, offset, offset+count, count, GL_UNSIGNED_SHORT, 0);
#endif

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


}
