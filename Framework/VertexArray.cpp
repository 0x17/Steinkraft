// VertexArray.cpp

#include "StdAfx.h"
#pragma hdrstop

#include "VertexArray.hpp"
#include "Utilities.hpp"

namespace as {

VertexArray::VertexArray(ComponentInfo comps)
:	VertexStorage(comps),
	vx(NULL)
{}

VertexArray::~VertexArray() {
	SAFE_DELETE_ARRAY(vx);
}

void VertexArray::setData(const float *vxtxnxcl, int numVxTxNxCl) {
	SAFE_DELETE_ARRAY(vx);
	vx = new float[numVxTxNxCl];
	memcpy(vx, vxtxnxcl, (ulong)((int)sizeof(float)*numVxTxNxCl));
	ndraw = numVxTxNxCl / compsPerVx;
}

void VertexArray::render(int offset, int count) {
	render(offset, count, GL_TRIANGLES);
}

void VertexArray::render(int offset, int count, GLenum primitiveType) {
	static int k;

	k = 0;

	if (comps.usePos) {
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizePerVx, &vx[0]);
		k += 3;
	}

	if (comps.useTexCoord) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizePerVx, &vx[k]);
		k += 2;
	}

	if (comps.useColor) {
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, sizePerVx, &vx[k]);
		k += 4;
	}

	glDrawArrays(primitiveType, offset, (int)count);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

} /* namespace as */
