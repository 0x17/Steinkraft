// Mesh.cpp

#include "StdAfx.h"
#pragma hdrstop

#include "Mesh.hpp"
#include "Utilities.hpp"
#include "VertexBuffer.hpp"
#include "VertexArray.hpp"
#include "VertexStorage.hpp"

namespace as {

bool useVertexArray = false;

Mesh::Mesh(ComponentInfo compInf, bool isStatic)
:	vxStorage(NULL)
{
	selectStorage(compInf, isStatic);
}

Mesh::Mesh(bool isStatic) : vxStorage(NULL) {
	ComponentInfo compInf;
	selectStorage(compInf, isStatic);
}

void Mesh::selectStorage(ComponentInfo compInf, bool isStatic) {
	if (useVertexArray)
		vxStorage = new VertexArray(compInf);
	else
		vxStorage = new VertexBuffer(compInf, isStatic);
}

Mesh::~Mesh() {
	SAFE_DELETE(vxStorage);
}

void Mesh::setVertices(const float *coords, int numCoords) {
	vxStorage->setData(coords, numCoords);
}

void Mesh::render(int offset, int count) const {
	vxStorage->render(offset, count);
}

void Mesh::render() {
	vxStorage->render();
}

void Mesh::render(GLenum primitiveType) const {
	//assert(vxStorage);
	if (vxStorage) vxStorage->render(primitiveType);
}

void Mesh::render(int offset, int count, GLenum primitiveType) const {
	vxStorage->render(offset, count, primitiveType);
}

} /* namespace as */
