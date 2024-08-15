// CubeVertices.hpp

#ifndef CUBEVERTICES_HPP_
#define CUBEVERTICES_HPP_

#include <vector>

#include "../../Constants.h"
#include "../../Terrain.hpp"

namespace as {

const int	NUM_POS_COORD_COMPS = 3;
const int	NUM_TX_COORD_COMPS	= 2;
const int	NUM_NOR_COORD_COMPS = 3;
const int	NUM_COL_COORD_COMPS = 4;

const float BC_MIN = 0.0f;
const float BC_MAX = 0.99f;

const float BCR_MIN = 0.0f;
const float BCR_MAX = 1.0f;

const int	TRIANGLES_PER_QUAD = 2;
const int	INDICES_PER_QUAD = 3 * TRIANGLES_PER_QUAD;
const int	UNIQUE_VERTICES_PER_QUAD = 4;
const int	VERTICES_PER_QUAD = 6;
const int	FACES_PER_BOX = 6;

// x,y,z, u,v, r,g,b,a
const int COMPONENTS_PER_VERTEX = 9;
const int COMPONENTS_PER_VERTEX_NOCOL = 5;

const ulong TRANS_POS_NORM_COL_VX_LEN = COMPONENTS_PER_VERTEX * FACES_PER_BOX * UNIQUE_VERTICES_PER_QUAD;
const ulong TRANS_POS_NORM_VX_LEN = COMPONENTS_PER_VERTEX_NOCOL * FACES_PER_BOX * UNIQUE_VERTICES_PER_QUAD;

#define USE_INDICES 0

static const float posCoords[] = {
	// front
	BC_MIN, BC_MAX, BC_MAX,
	BC_MIN, BC_MIN, BC_MAX,
	BC_MAX, BC_MIN, BC_MAX,
	BC_MAX, BC_MAX, BC_MAX,
	// back
	BC_MAX, BC_MAX, BC_MIN,
	BC_MAX, BC_MIN, BC_MIN,
	BC_MIN, BC_MIN, BC_MIN,
	BC_MIN, BC_MAX, BC_MIN,
	// left
	BC_MIN, BC_MAX, BC_MIN,
	BC_MIN, BC_MIN, BC_MIN,
	BC_MIN, BC_MIN, BC_MAX,
	BC_MIN, BC_MAX, BC_MAX,
	// right
	BC_MAX, BC_MAX, BC_MAX,
	BC_MAX, BC_MIN, BC_MAX,
	BC_MAX, BC_MIN, BC_MIN,
	BC_MAX, BC_MAX, BC_MIN,
	// bottom
	BC_MIN, BC_MIN, BC_MAX,
	BC_MIN, BC_MIN, BC_MIN,
	BC_MAX, BC_MIN, BC_MIN,
	BC_MAX, BC_MIN, BC_MAX,
	// top
	BC_MAX, BC_MAX, BC_MAX,
	BC_MAX, BC_MAX, BC_MIN,
	BC_MIN, BC_MAX, BC_MIN,
	BC_MIN, BC_MAX, BC_MAX
};

enum FaceIndices {
	FRONT_INDEX,
	BACK_INDEX,
	LEFT_INDEX,
	RIGHT_INDEX,
	BOTTOM_INDEX,
	TOP_INDEX
};

static const float posCoordsRender[] = {
	// front
	BCR_MIN, BCR_MAX, BCR_MAX,
	BCR_MIN, BCR_MIN, BCR_MAX,
	BCR_MAX, BCR_MIN, BCR_MAX,
	BCR_MAX, BCR_MAX, BCR_MAX,
	// back
	BCR_MAX, BCR_MAX, BCR_MIN,
	BCR_MAX, BCR_MIN, BCR_MIN,
	BCR_MIN, BCR_MIN, BCR_MIN,
	BCR_MIN, BCR_MAX, BCR_MIN,
	// left
	BCR_MIN, BCR_MAX, BCR_MIN,
	BCR_MIN, BCR_MIN, BCR_MIN,
	BCR_MIN, BCR_MIN, BCR_MAX,
	BCR_MIN, BCR_MAX, BCR_MAX,
	// right
	BCR_MAX, BCR_MAX, BCR_MAX,
	BCR_MAX, BCR_MIN, BCR_MAX,
	BCR_MAX, BCR_MIN, BCR_MIN,
	BCR_MAX, BCR_MAX, BCR_MIN,
	// bottom
	BCR_MIN, BCR_MIN, BCR_MAX,
	BCR_MIN, BCR_MIN, BCR_MIN,
	BCR_MAX, BCR_MIN, BCR_MIN,
	BCR_MAX, BCR_MIN, BCR_MAX,
	// top
	BCR_MAX, BCR_MAX, BCR_MAX,
	BCR_MAX, BCR_MAX, BCR_MIN,
	BCR_MIN, BCR_MAX, BCR_MIN,
	BCR_MIN, BCR_MAX, BCR_MAX
};

// FIXME: Expand normals procedurally!
static const float normals[] = {
	// front
	0, 0, 1,
	0, 0, 1,
	0, 0, 1,
	0, 0, 1,
	// back
	0, 0, -1,
	0, 0, -1,
	0, 0, -1,
	0, 0, -1,
	// left
	-1, 0, 0,
	-1, 0, 0,
	-1, 0, 0,
	-1, 0, 0,
	// right
	1, 0, 0,
	1, 0, 0,
	1, 0, 0,
	1, 0, 0,
	// bottom
	0, -1, 0,
	0, -1, 0,
	0, -1, 0,
	0, -1, 0,
	// top
	0, 1, 0,
	0, 1, 0,
	0, 1, 0,
	0, 1, 0,
};

static const GLuint indices[] = {
	// front
	0, 1, 2, 2, 3, 0,
	// back
	4, 5, 6, 6, 7, 4,
	// left
	8, 9, 10, 10, 11, 8,
	// right
	12, 13, 14, 14, 15, 12,
	// bottom
	16, 17, 18, 18, 19, 16,
	// top
	20, 21, 22, 22, 23, 20
};

class TexCell {
public:
	int row, col;

	TexCell(int _row, int _col)
			: row(_row), col(_col) {}
};

class TexCoordRect;

int buildTri(int faceOffset, int vi1, int vi2, int vi3, float *vx,
			 int startIndex, TexCell *cell, float scale = 1.0f, bool centeredOrigin = false,
			 float *offsets = NULL, float brightness = 1.0f);

void genPosTxCoordCubeVertices(TexCell **cells, int numCells, float *vx,
							   float scale = 1.0f, bool centeredOrigin = false, float brightness = 1.0f);

std::vector<float> *genNormals();
void genTexCoords(TexCoordRect *tcr, float *txCoords);
void translatedPositions(float x, float y, float z, float *coords);
void genTranslatedPosTexNormalColVertices(float x, float y, float z,
		TexCoordRect *tcr, float r = 1.0f, float g = 1.0f, float b = 1.0f, float alpha = 1.0f, float *out = NULL);
void genTranslatedPosTexNormalColVerticesFast(float x, float y, float z, TexCoordRect *tcr, float *out = NULL);

std::vector<float> *genTrianglesOfBlockAtPos(int x, int y, int z, VisibleFaces vfaces);
ulong addCoords(std::vector<float> *tris, ulong faceOffset, ulong i, int x, int y, int z);

} /* namespace as */
#endif /* CUBEVERTICES_HPP_ */
