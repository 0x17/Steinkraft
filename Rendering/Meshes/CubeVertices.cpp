// CubeVertices.cpp



#include "CubeVertices.hpp"
#include "../../Constants.h"
#include "../../Framework/Utilities.hpp"
#include "../../Framework/VertexStorage.hpp"

namespace as {

const ulong TRANSLATED_POS_LEN = NUM_POS_COORD_COMPS * UNIQUE_VERTICES_PER_QUAD * FACES_PER_BOX;

int buildTri(int faceOffset, int vi1, int vi2, int vi3, float *vx,
			 int startIndex, TexCell *cell, float scale, bool centeredOrigin,
			 float *offsets, float brightness) {
	int vi[3];
	float minU, maxU, minV, maxV;

	const float FTEX_SIZE = (float)TEX_SIZE;

	vi[0] = vi1;
	vi[1] = vi2;
	vi[2] = vi3;

	int r = cell->row, c = cell->col;
	minU = (c * FTEX_SIZE) / ACT_TEX_SIZE;
	maxU = ((c + 1) * FTEX_SIZE) / ACT_TEX_SIZE;
	minV = (r * FTEX_SIZE) / ACT_TEX_SIZE;
	maxV = ((r + 1) * FTEX_SIZE) / ACT_TEX_SIZE;

	for (int i = 0; i < 3; i++) { // for each vertex
		for (int j = 0; j < 3; j++) { // for each pos coord
			vx[startIndex++] = (posCoordsRender[vi[i] * 3 + faceOffset + j] - (centeredOrigin ? 0.5f : 0.0f)) * scale;
			if (offsets) { vx[startIndex-1] += offsets[j]; }
		}

		// tex coords
		switch (vi[i]) {
		case 0:
			vx[startIndex++] = minU;
			vx[startIndex++] = minV;
			break;
		case 1:
			vx[startIndex++] = minU;
			vx[startIndex++] = maxV;
			break;
		case 2:
			vx[startIndex++] = maxU;
			vx[startIndex++] = maxV;
			break;
		case 3:
			vx[startIndex++] = maxU;
			vx[startIndex++] = minV;
			break;
		}

		// color comps (rgba)
		vx[startIndex++] = brightness;
		vx[startIndex++] = brightness;
		vx[startIndex++] = brightness;
		vx[startIndex++] = 1.0f;
	}

	return startIndex;
}

void genPosTxCoordCubeVertices(TexCell **cells, int numCells, float *vx, float scale, bool centeredOrigin, float brightness) {
	TexCell *lcells[FACES_PER_BOX];
	
	if (numCells == 1) { // one texture for entire cube
		for (int i = 0; i < FACES_PER_BOX; i++) {
			lcells[i] = cells[0];
		}
	} else if (numCells == FACES_PER_BOX) { // one texture for each face
		memcpy(lcells, cells, sizeof(TexCell *) * FACES_PER_BOX);
	} else { // should not happen!
		assert(0);
		return;
	}

	const int numCoords = VERTICES_PER_QUAD * FACES_PER_BOX * (3 + 2 + 4);

	int k = 0, faceOffset;

	// for each face
	for (int i = 0; i < FACES_PER_BOX; i++) {
		// build face
		faceOffset = i * 12;
		// 0,1,2,2,3,0
		k = buildTri(faceOffset, 0, 1, 2, vx, k, lcells[i], scale, centeredOrigin, NULL, brightness);
		k = buildTri(faceOffset, 2, 3, 0, vx, k, lcells[i], scale, centeredOrigin, NULL, brightness);
	}

	assert(k == numCoords);
}

// no dups, can be used for indexed drawing
std::vector<float> *genNormals() {
	ulong i;
	const ulong l = NUM_NOR_COORD_COMPS * UNIQUE_VERTICES_PER_QUAD * FACES_PER_BOX;
	std::vector<float> *nors = new std::vector<float>(l);
	for (i = 0; i < l; i++) {
		(*nors)[i] = normals[i];
	}
	return nors;
}

// no dups, can be used for indexed drawing
void genTexCoords(TexCoordRect *tcr, float *result) {
	result[0] = tcr->minU;
	result[1] = tcr->minV;
	result[2] = tcr->minU;
	result[3] = tcr->maxV;
	result[4] = tcr->maxU;
	result[5] = tcr->maxV;
	result[6] = tcr->maxU;
	result[7] = tcr->minV;
}

// no dups, can be used for indexed drawing
void translatedPositions(float x, float y, float z, float *tpos) {
	for (ulong i = 0; i < TRANSLATED_POS_LEN; i++) {
		tpos[i] = posCoordsRender[i] + (i % 3 == 0 ? x : (i % 3 == 1) ? y : z);
	}
}

// no dups, can be used for indexed drawing
void genTranslatedPosTexNormalColVerticesFast(float x, float y, float z, TexCoordRect *tcr, float *verts) {
	static float txcoords[8];
	
	txcoords[0] = tcr->minU;
	txcoords[1] = tcr->minV;
	txcoords[2] = tcr->minU;
	txcoords[3] = tcr->maxV;
	txcoords[4] = tcr->maxU;
	txcoords[5] = tcr->maxV;
	txcoords[6] = tcr->maxU;
	txcoords[7] = tcr->minV;
	
	//genTexCoords(tcr, txcoords);	
	//float pCoords[TRANSLATED_POS_LEN];
	//translatedPositions(x, y, z, pCoords);

	ulong p, q = 0, k = 0;
	for (p = 0; p < TRANS_POS_NORM_VX_LEN; p += COMPONENTS_PER_VERTEX_NOCOL) {
		verts[p] = posCoordsRender[q] + x; // x
		verts[p+1] = posCoordsRender[q+1] + y; // y
		verts[p+2] = posCoordsRender[q+2] + z; // z
		verts[p+3] = txcoords[k % 8]; // u
		verts[p+4] = txcoords[(k+1) % 8]; // v

		q += NUM_POS_COORD_COMPS;
		k += NUM_TX_COORD_COMPS;
	}
}

void genTranslatedPosTexNormalColVertices(float x, float y, float z, TexCoordRect *tcr, float red, float green, float blue, float alpha, float *verts) {
	static float txcoords[8];
	
	txcoords[0] = tcr->minU;
	txcoords[1] = tcr->minV;
	txcoords[2] = tcr->minU;
	txcoords[3] = tcr->maxV;
	txcoords[4] = tcr->maxU;
	txcoords[5] = tcr->maxV;
	txcoords[6] = tcr->maxU;
	txcoords[7] = tcr->minV;
	
	//genTexCoords(tcr, txcoords);	
	//float pCoords[TRANSLATED_POS_LEN];
	//translatedPositions(x, y, z, pCoords);

	ulong p, q = 0, k = 0;
	for (p = 0; p < TRANS_POS_NORM_COL_VX_LEN; p += COMPONENTS_PER_VERTEX) {
		verts[p] = posCoordsRender[q] + x; // x
		verts[p+1] = posCoordsRender[q+1] + y; // y
		verts[p+2] = posCoordsRender[q+2] + z; // z
		verts[p+3] = txcoords[k % 8]; // u
		verts[p+4] = txcoords[(k+1) % 8]; // v
		verts[p+5] = red;
		verts[p+6] = green;
		verts[p+7] = blue;
		verts[p+8] = alpha;

		q += NUM_POS_COORD_COMPS;
		k += NUM_TX_COORD_COMPS;
	}
}

// used for ray->block face triangle intersection. triangles non-indexed therefore contain dups.
std::vector<float> *genTrianglesOfBlockAtPos(int x, int y, int z, VisibleFaces vfaces) {
	ulong i = 0;
	ulong l = vfaces.numVisibleFaces() * NUM_POS_COORD_COMPS * VERTICES_PER_QUAD;
	std::vector<float> *tris = new std::vector<float>(l);

	if (vfaces.left) {
		i = addCoords(tris, 24, i, x, y, z);
	}
	if (vfaces.right) {
		i = addCoords(tris, 36, i, x, y, z);
	}
	if (vfaces.front) {
		i = addCoords(tris, 0, i, x, y, z);
	}
	if (vfaces.back) {
		i = addCoords(tris, 12, i, x, y, z);
	}
	if (vfaces.bottom) {
		i = addCoords(tris, 48, i, x, y, z);
	}
	if (vfaces.top) {
		/*i = */addCoords(tris, 60, i, x, y, z);
	}

	return tris;
}

ulong addCoords(std::vector<float> *tris, ulong faceOffset, ulong i, int x, int y, int z) {
	ulong p = faceOffset, k;
	// add 0,1,2
	for (k = 0; k < 3;k++) {
		(*tris)[i] = posCoords[p] + x;
		(*tris)[i+1] = posCoords[p+1] + y;
		(*tris)[i+2] = posCoords[p+2] + z;
		p += 3;
		i += 3;
	}
	p -= 3;
	// add 2,3
	for (k = 0; k < 2; k++) {
		(*tris)[i] = posCoords[p] + x;
		(*tris)[i+1] = posCoords[p+1] + y;
		(*tris)[i+2] = posCoords[p+2] + z;
		p += 3;
		i += 3;
	}
	// add 0
	p = faceOffset;
	(*tris)[i] = posCoords[p] + x;
	(*tris)[i+1] = posCoords[p+1] + y;
	(*tris)[i+2] = posCoords[p+2] + z;
	i += 3;

	return i;
}

} /* namespace as */
