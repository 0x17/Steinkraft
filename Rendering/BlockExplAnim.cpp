// BlockExplAnim.cpp

#include "StdAfx.h"
#pragma hdrstop

#include <cstdlib>
#include <cassert>
#include <ctime>

#include "../Framework/PGL.h"
#include "../Framework/Utilities.hpp"

#include "BlockExplAnim.hpp"

#include "Meshes/ChunkMesh.hpp"

namespace as {
//===========================================================================
// Constants/Macros
//===========================================================================
const int		NUM_PARTICLES = 100;
const float 	POINT_SIZE = 24.0f;

//===========================================================================
// Methods
//===========================================================================
BlockExplAnim::BlockExplAnim(int _x, int _y, int _z, int texId, int _blockBelowY, ComponentInfo compInf, Terrain *t)
	:	Animation(Animation::INFINITE_DURATION),
	yAnim(0.0f),

	x(_x),
	y(_y),
	z(_z),

	mesh(compInf),
	blockBelowY(_blockBelowY)
{
	int cpv = compInf.getCompsPerVx();
	int l = cpv * NUM_PARTICLES;

	float *vx = new float[l];
	float u, v;

	int row, col;
	row = texId / NUM_TEX_PER_ROW;
	col = texId % NUM_TEX_PER_ROW;

	assert(cpv == 9);

	int k = 0;
	float kx, ky, kz;
	
	float brightness = ChunkMesh::getDaylightFactor() * (t->isBlockAbove(x, y, z) ? 0.5f : 1.0f);

	for (int i = 0; i < NUM_PARTICLES; i++) {
		kx = ((rand() % 98) + 1) * 0.01f;
		ky = ((rand() % 98) + 1) * 0.01f;
		kz = ((rand() % 98) + 1) * 0.01f;

		u = (kx + col) * TEX_COORD_FACTOR;
		v = (kz + row) * TEX_COORD_FACTOR;

		vx[k] = kx - 0.5f; // x
		vx[k+1] = ky - 0.5f; // y
		vx[k+2] = kz - 0.5f; // z
		vx[k+3] = u; // u
		vx[k+4] = v; // v
		vx[k+5] = brightness; // r
		vx[k+6] = brightness; // g
		vx[k+7] = brightness; // b
		vx[k+8] = 1.0f; // a
		k += cpv;
	}

	mesh.setVertices(vx, l);
	SAFE_DELETE_ARRAY(vx);

	start();
}

/**
 Destruct block explosion animation.
*/
BlockExplAnim::~BlockExplAnim() {
}

/**
 Render block explosion animation.
*/
bool BlockExplAnim::render() {
	static float sc, cy;

	glPushMatrix();
	sc = 1.0f + counter * 0.4f;
	sc = (sc >= 1.3f) ? 1.3f : sc;
	yAnim = -(counter * counter) * 0.2f + 0.25f;
	cy = yAnim + y + 0.5f;
	glTranslatef(x + 0.5f, cy, z + 0.5f);
	glScalef(sc, sc, sc);
	mesh.render(GL_POINTS);
	glPopMatrix();

	return (cy >= blockBelowY);
}

//==============================================================================

BlockExplAnimManager::BlockExplAnimManager(Terrain *t) : terrain(t), compInf(true, true, true) {
	GLfloat attenuation[] = { 0.0f, 0.5f, 1.0f };
	glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, attenuation);
	glPointSize(POINT_SIZE);
}

BlockExplAnimManager::~BlockExplAnimManager() {
	srand((uint)std::time(NULL));
	std::list<BlockExplAnim *>::iterator it;
	for (it = anims.begin(); it != anims.end(); ++it) {
		SAFE_DELETE(*it);
	}
	anims.clear();
}

void BlockExplAnimManager::addBlockExplAnim(int x, int y, int z, int texId, int blockBelowY) {
	anims.push_back(new BlockExplAnim(x, y, z, texId, blockBelowY, compInf, terrain));
}

void BlockExplAnimManager::render() {
	// TODO: Also add color information to particle meshes
	//glDisableClientState(GL_COLOR_ARRAY);
#if MOBILE
	glEnable(GL_POINT_SPRITE_OES);
#else
	glEnable(GL_POINT_SPRITE);
#endif

	std::list<BlockExplAnim *>::iterator it = anims.begin();
	while (it != anims.end()) {
		// TODO: do frustum culling on block explosions
		BlockExplAnim *a = (*it);

		a->update();

		// only limited by hitting the floor now
		if (!a->render()) { // || !a->update()
			SAFE_DELETE((*it));
			anims.erase(it++);
		} else {
			++it;
		}
	}

#if MOBILE
	glDisable(GL_POINT_SPRITE_OES);
#else
	glDisable(GL_POINT_SPRITE);
#endif
	//glEnableClientState(GL_COLOR_ARRAY);
}

} /* namespace as */
