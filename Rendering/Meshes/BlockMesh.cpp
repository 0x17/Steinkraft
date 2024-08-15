// BlockMesh.cpp

#include "StdAfx.h"
#pragma hdrstop

#include "../../Constants.h"
#include "../../Framework/Utilities.hpp"

#include "CubeVertices.hpp"
#include "BlockMesh.hpp"

#include "ChunkMesh.hpp"

namespace as {

BlockMesh::BlockMesh(int texRow, int texCol, float alpha, CubeFace face, float _bness) :	Mesh(), bness(_bness) {
	TexCoordRect tcr(TEX_COORD_FACTOR * texCol, TEX_COORD_FACTOR * (texCol + 1),
					 TEX_COORD_FACTOR * texRow, TEX_COORD_FACTOR * (texRow + 1));
	update(&tcr, alpha, &face);
}

BlockMesh::BlockMesh() : Mesh() {
	bness = 1.0f;
	TexCoordRect tcr(0, 0, 1, 1);
	update(&tcr);
}

BlockMesh::BlockMesh(int texRow, int texCol, float _bness) : Mesh(), bness(_bness) {
	TexCoordRect tcr(TEX_COORD_FACTOR * texCol, TEX_COORD_FACTOR * (texCol + 1),
					 TEX_COORD_FACTOR * texRow, TEX_COORD_FACTOR * (texRow + 1));
	update(&tcr);
}

BlockMesh::BlockMesh(TexCoordRect *tcr) : Mesh() {
	bness = 1.0f;
	update(tcr);
}

BlockMesh::BlockMesh(float alpha) : Mesh() {
	bness = 1.0f;
	TexCoordRect tcr(0, 0, 1, 1);
	update(&tcr, alpha);
}

BlockMesh::~BlockMesh() {
}

void BlockMesh::update(TexCoordRect *tcr) {
	update(tcr, 1.0f, NULL);
}

void BlockMesh::update(TexCoordRect *tcr, float alpha, CubeFace *face) {
	bness *= ChunkMesh::getDaylightFactor();
	
	// adding dups!
	// maybe cube vertices should already contain dups?
	float v[TRANS_POS_NORM_COL_VX_LEN];
	genTranslatedPosTexNormalColVertices(0, 0, 0, tcr, bness, bness, bness, alpha, v);
	std::vector<float>::size_type length = TRANS_POS_NORM_COL_VX_LEN;

	if (face) length /= 6;

	float *vp = new float[USE_INDICES ? length : length*3/2];
#if USE_INDICES
	GLuint *ip = new GLint[length / COMPONENTS_PER_VERTEX * 2];
	int g = 0;
#endif

	int i, j, k = 0;
	float dim;
	int curFace = 0;

	// for each face...
	for (j = 0; j < (signed)length; j += 4 * COMPONENTS_PER_VERTEX) {
		if (face && (*face != j))
			continue;

		if(curFace == LEFT_INDEX || curFace == RIGHT_INDEX) {
			dim = 0.2f;
		} else if(curFace == FRONT_INDEX || curFace == BACK_INDEX) {
			dim = 0.4f;
		} else {
			dim = 0.0f;
		}

		curFace++;

		// 0, 1, 2
		for (i = j; i < j + 3*COMPONENTS_PER_VERTEX; i++) {
			vp[k] = v[i];
			if(k % COMPONENTS_PER_VERTEX >= 5 && k % COMPONENTS_PER_VERTEX <= 7)
				vp[k] *= (1.0f-dim);
			k++;
		}

#if USE_INDICES
		// 3
		for (i = j + 3 * COMPONENTS_PER_VERTEX; i < j + 4*COMPONENTS_PER_VERTEX; i++) {
			vp[k] = (*v)[i];
			k++;
		}

		ip[g] =		j + 0 * COMPONENTS_PER_VERTEX;
		ip[g+1] =	j + 1 * COMPONENTS_PER_VERTEX;
		ip[g+2] = 	j + 2 * COMPONENTS_PER_VERTEX;

		ip[g+3] =	j + 2 * COMPONENTS_PER_VERTEX;
		ip[g+4] = 	j + 3 * COMPONENTS_PER_VERTEX;
		ip[g+5] =	j + 0 * COMPONENTS_PER_VERTEX;

		g += 6;
#else
		// 2, 3
		for (i = j + 2 * COMPONENTS_PER_VERTEX; i < j + 4*COMPONENTS_PER_VERTEX; i++) {
			vp[k] = v[i];
			if(k % COMPONENTS_PER_VERTEX >= 5 && k % COMPONENTS_PER_VERTEX <= 7)
				vp[k] *= (1.0f-dim);
			k++;
		}

		// 0
		for (i = j; i < j + 1*COMPONENTS_PER_VERTEX; i++) {
			vp[k] = v[i];
			if(k % COMPONENTS_PER_VERTEX >= 5 && k % COMPONENTS_PER_VERTEX <= 7)
				vp[k] *= (1.0f-dim);
			k++;
		}
#endif
	}

#if !USE_INDICES
	setVertices(vp, (int)length * 3 / 2);
#else
	//setIndexedVertices(vp, (int)length, ip, (int)length);
#endif

	SAFE_DELETE_ARRAY(vp);

#if USE_INDICES
	SAFE_DELETE_ARRAY(ip);
#endif
}

} /* namespace as */
