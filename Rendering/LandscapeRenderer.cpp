// LandscapeRenderer.cpp

#include "StdAfx.h"
#pragma hdrstop

#include <cassert>

#include "../Constants.h"

#include "../Framework/Math/Intersector.hpp"
#include "../Framework/Utilities.hpp"

#include "Voxelrenderer/ChunkMeshRenderer.hpp"

#include "Meshes/BlockMesh.hpp"
#include "Meshes/CubeVertices.hpp"

#include "LandscapeRenderer.hpp"
#include "BlockExplAnim.hpp"

#include "../Managers/RailManager.hpp"

namespace as {

const float HLIGHT_FACE_OFFSET = 0.01f;

LandscapeRenderer::LandscapeRenderer(Terrain *_terrain, RailManager *_railManager, Camera *_cam, AnimalManager *_animalManager)
:	voxelRenderer(NULL),
	selectedBlock(NULL),
	terrain(_terrain),
	cam(_cam),
	highlightBlock(NULL),
	explAnimMgr(NULL)
{
	voxelRenderer = new ChunkMeshRenderer(terrain, _railManager, cam, _animalManager);

	highlightBlock = new BlockMesh(0.25f);
	explAnimMgr = new BlockExplAnimManager(terrain);

	setupFog();
}

void LandscapeRenderer::setupFog(float factor) {
	float fogColor[3] = { 0.6289f * factor, 0.6953f * factor, 0.8633f * factor};
	glFogfv(GL_FOG_COLOR, fogColor);
	float fogStart, fogEnd;

	if (visualDetail == DETAIL_VERY_LOW) {
		fogStart = 15.0f;
		fogEnd = 20.0f;
	} else if (visualDetail == DETAIL_LOW) {
		fogStart = 20.0f;
		fogEnd = 40.0f;
	} else if (visualDetail == DETAIL_MEDIUM) {
		fogStart = 30.0f;
		fogEnd = 50.0f;
	} else if (visualDetail == DETAIL_HIGH) {
		fogStart = 40.0f;
		fogEnd = 60.0f;
	} else { // VERY_HIGH
		fogStart = 90.0f;
		fogEnd = 100.0f;
	}

	glFogf(GL_FOG_START, fogStart);
	glFogf(GL_FOG_END, fogEnd);
	glFogf(GL_FOG_MODE, GL_LINEAR);
	glHint(GL_FOG_HINT, GL_FASTEST);
	glEnable(GL_FOG);
}

LandscapeRenderer::~LandscapeRenderer() {
	SAFE_DELETE(explAnimMgr);
	SAFE_DELETE(voxelRenderer);
	SAFE_DELETE(selectedBlock);
	SAFE_DELETE(highlightBlock);
}

void LandscapeRenderer::update(BlockPos *bposChanged) {
	voxelRenderer->update(bposChanged);
}

void LandscapeRenderer::render(bool highlightSelBlock) {
	voxelRenderer->render();
	explAnimMgr->render();
#if !MOBILE_MODE
	highlightSelectedBlock();
#else
	if (highlightSelBlock)
		highlightSelectedBlock();
#endif
}

#if DEBUG_MODE
// FOR DEBUGGING
BlockPos lastSelBlock;
#endif

void LandscapeRenderer::updateSelectedBlock(std::list<BlockPos> *blocksNearCam, int x, int y) {
	Ray pRay = cam->getPickRay(x, y);

	std::vector<float> *tris = genTrianglesOfBlocksAtPositions(blocksNearCam, &pRay);

	SAFE_DELETE(selectedBlock);

	if (intersector::intersectRayTriangles(&pRay, tris, &isect)) {
		selectedFace = determineSelectedFace();
		selectedBlock = new BlockPos((int)(isect.x), (int)(isect.y), (int)(isect.z));

#if DEBUG_MODE
		// FOR DEBUGGING
		if (!(selectedBlock == lastSelBlock))) {
			std::printf("Selected block: (%d,%d,%d)\n", selectedBlock->x, selectedBlock->y, selectedBlock->z);
			fflush(stdout);
			lastSelBlock = BlockPos(selectedBlock);
		}
#endif

	} else {
		selectedBlock = NULL;
	}

	SAFE_DELETE(tris);
}

void LandscapeRenderer::highlightSelectedBlock() {
	if (selectedBlock) {
		glPushMatrix();
		glTranslatef((float)(int)(isect.x), (float)(int)(isect.y), (float)(int)(isect.z));
		highlightFace(selectedFace);
		glPopMatrix();
	}
}

float rayPointDistanceSquared(Ray *ray, Vec3 *point) {
	Vec3 w = *point - ray->origin;
	float projectedDist = w * ray->direction;
	return w.lengthSquared() - projectedDist*projectedDist;
}

std::vector<float> *LandscapeRenderer::genTrianglesOfBlocksAtPositions(std::list<BlockPos> *blocksNearCam, Ray *ray) {
	std::vector<float> *tris = NULL;
	std::vector<float> *result = new std::vector<float>();
	Vec3 blockCenter;

	std::list<BlockPos>::iterator it;
	for (it = blocksNearCam->begin(); it != blocksNearCam->end(); ++it) {
		BlockPos pos(*it);

		int px = pos.x, py = pos.y, pz = pos.z;
		
		blockCenter = Vec3(px+0.5f, py+0.5f, pz+0.5f);
		if(rayPointDistanceSquared(ray, &blockCenter) > 0.5f)
			continue;

		VisibleFaces vfaces = terrain->determineVisibleFaces(px, py, pz);

		tris = genTrianglesOfBlockAtPos(px, py, pz, vfaces);

#if DEBUG_MODE
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
		drawModeWireframe();
		glPointSize(4);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		glBegin(GL_TRIANGLES);
		for (int b = 0; b < tris->size(); b += 3) {
			glVertex3f((*tris)[b], (*tris)[b+1] + 0.1f, (*tris)[b+2]);
		}
		glEnd();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		drawModeFilled();
		glEnable(GL_TEXTURE_2D);
#endif

		for (unsigned int q = 0; q < tris->size(); q++) {
			result->push_back((*tris)[q]);
		}

		SAFE_DELETE(tris);
	}

	return result;
}

void LandscapeRenderer::highlightFace(CubeFace visFace) {
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	switch (visFace) {
	case CF_BACK:
		glTranslatef(0, 0, -HLIGHT_FACE_OFFSET);
		highlightBlock->render(6, 6);
		break;
	case CF_BOTTOM:
		glTranslatef(0, -HLIGHT_FACE_OFFSET, 0);
		highlightBlock->render(24, 6);
		break;
	case CF_FRONT:
		glTranslatef(0, 0, HLIGHT_FACE_OFFSET);
		highlightBlock->render(0, 6);
		break;
	case CF_LEFT:
		glTranslatef(-HLIGHT_FACE_OFFSET, 0, 0);
		highlightBlock->render(12, 6);
		break;
	case CF_RIGHT:
		glTranslatef(HLIGHT_FACE_OFFSET, 0, 0);
		highlightBlock->render(18, 6);
		break;
	case CF_TOP:
		glTranslatef(0, HLIGHT_FACE_OFFSET, 0);
		highlightBlock->render(30, 6);
		break;
	}

	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
}

CubeFace LandscapeRenderer::determineSelectedFace() {
	float 	dX = isect.x - (int)isect.x;
	float	dY = isect.y - (int)isect.y;
	float	dZ = isect.z - (int)isect.z;

	if (dX < 0.1f)
		return CF_LEFT;
	else if (dX >= 0.9f)
		return CF_RIGHT;

	else if (dY < 0.1f)
		return CF_BOTTOM;
	else if (dY >= 0.9f)
		return CF_TOP;

	else if (dZ < 0.1f)
		return CF_BACK;
	else if (dZ >= 0.9f)
		return CF_FRONT;

	// there should be an INVALID value
	assert(0);
	return CF_FRONT;
}

} /* namespace as */
