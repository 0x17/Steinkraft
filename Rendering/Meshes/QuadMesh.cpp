// QuadMesh.cpp



#include "../../Constants.h"
#include "QuadMesh.hpp"
#include "CubeVertices.hpp"
#include "../../Framework/SpriteBatch.hpp"

namespace as {

QuadMesh::QuadMesh(int texRow, int texCol, int x, int y, int w, int h) : Mesh() {
	update(TEX_COORD_FACTOR * texCol, TEX_COORD_FACTOR * (texCol + 1),
		   TEX_COORD_FACTOR * texRow, TEX_COORD_FACTOR * (texRow + 1),
		   x, y, w, h);
}

QuadMesh::QuadMesh(const TexCoordRect tcr, int x, int y, int w, int h, float alpha) : Mesh() {
	update(tcr.minU / ACT_TEX_SIZE, tcr.maxU / ACT_TEX_SIZE,
		   tcr.minV / ACT_TEX_SIZE, tcr.maxV / ACT_TEX_SIZE,
		   x, y, w, h, alpha);
}

QuadMesh::QuadMesh(const TexCoordRect tcr, const Rect rect, float alpha) : Mesh() {
	update(tcr.minU / ACT_TEX_SIZE, tcr.maxU / ACT_TEX_SIZE,
			   tcr.minV / ACT_TEX_SIZE, tcr.maxV / ACT_TEX_SIZE,
			   rect.x, SCR_H - rect.y - rect.h, rect.w, rect.h, alpha);
}

QuadMesh::QuadMesh(float minU, float maxU, float minV, float maxV, int x, int y, int w, int h) : Mesh() {
	update(minU, maxU, minV, maxV, x, y, w, h);
}

QuadMesh::QuadMesh(float minU, float maxU, float minV, float maxV, int x, int y, int w, int h, float alpha) : Mesh() {
	update(minU, maxU, minV, maxV, x, y, w, h, alpha);
}

QuadMesh::~QuadMesh() {
}

void QuadMesh::update(float minU, float maxU, float minV, float maxV, int x, int y, int w, int h, float alpha) {
	float vertices[] = { // position coordinates, texture coordinates, normal coordinates, color(rgba)
		(float)x, (float)(y + h), 0,	minU, minV,
		1, 1, 1, alpha,
		(float)x, (float)y, 0,	minU, maxV,
		1, 1, 1, alpha,
		(float)(x + w), (float)y, 0,	maxU, maxV,
		1, 1, 1, alpha,
		(float)(x + w), (float)y, 0,	maxU, maxV,
		1, 1, 1, alpha,
		(float)(x + w), (float)(y + h), 0,	maxU, minV,
		1, 1, 1, alpha,
		(float)x, (float)(y + h), 0,	minU, minV,
		1, 1, 1, alpha
	};

	setVertices(vertices, COMPONENTS_PER_VERTEX*6);
}

}
