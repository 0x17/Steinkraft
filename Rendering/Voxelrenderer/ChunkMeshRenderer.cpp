// WorldMeshRenderer.cpp

#include "StdAfx.h"
#pragma hdrstop

#include <cassert>

#include "../../Framework/Utilities.hpp"
#include "../../Framework/Camera.hpp"
#include "../../Framework/Math/Frustum.hpp"

#include "../Meshes/ChunkMesh.hpp"

#include "ChunkMeshRenderer.hpp"

namespace as {

int gAdjChunkDist;
int numChangedChunks;

ChunkMeshRenderer::ChunkMeshRenderer(Terrain *_t, RailManager *_rm, Camera *_cam, AnimalManager *_animalManager)
:		t(_t),
		rm(_rm),
		cam(_cam),
		frustum(*(_cam->getFrustumPtr())),
		animalManager(_animalManager),
		lastSceneUpdate(0),
		startTicks(getTicks()),
		camPosY(0)
{
	if (visualDetail == DETAIL_VERY_LOW) {
		gAdjChunkDist = 1;
		ticksBetweenChunkUpdates = 800;
	} else if (visualDetail == DETAIL_LOW) {
		gAdjChunkDist = 2;
		ticksBetweenChunkUpdates = 400;
	} else if (visualDetail == DETAIL_MEDIUM) {
		gAdjChunkDist = 3; // visualDetail;
		ticksBetweenChunkUpdates = 200;
	} else if (visualDetail == DETAIL_HIGH) {
		ticksBetweenChunkUpdates = 100;
		gAdjChunkDist = 4;
	} else { // DETAIL_VERY_HIGH
		ticksBetweenChunkUpdates = 50;
		gAdjChunkDist = 10;
	}
	
	ChunkMesh::reset();

	cam->addObserver(this);
	reset();
	
	if(keepMeshes) {
		for (int x = 0; x < NUM_CHUNKS_X_Z; x++) {
			for (int z = 0; z < NUM_CHUNKS_X_Z; z++) {
				allocateChunk(x, z);
			}
		}
	}
}
	
void ChunkMeshRenderer::freeAllMeshes() {
	for (int x = 0; x < NUM_CHUNKS_X_Z; x++) {
		for (int z = 0; z < NUM_CHUNKS_X_Z; z++) {
			SAFE_DELETE(chunkMeshes[x][z]);
			SAFE_DELETE(entityBatches[x][z]);
		}
	}
}

ChunkMeshRenderer::~ChunkMeshRenderer() {
	freeAllMeshes();
}

void ChunkMeshRenderer::reset() {
	Vec3 camPos = cam->getPos();
	memset(chunkMeshes, 0, sizeof(ChunkMesh *) * NUM_CHUNKS_X_Z * NUM_CHUNKS_X_Z);
	memset(entityBatches, 0, sizeof(EntityBatch *) * NUM_CHUNKS_X_Z * NUM_CHUNKS_X_Z);
	lastSceneUpdate = 0;
	update(&camPos);
}

inline void ChunkMeshRenderer::addDirtyChkNoDups(int cmx, int cmz, int changedPosY) {
	if (!chunkAlreadyDirty(cmx, cmz, t->isEntityUpdate())) {
		dirtyChunks.push_back(ScheduledChunk(cmx, cmz, t->isEntityUpdate(), changedPosY));
	}
}

// terrain changed (buffer this, because it is expensive!)
void ChunkMeshRenderer::update(BlockPos *bposChanged) {
	// chunk containing the changed terrain
	int cmx = (int)((float)bposChanged->x / (float)CHUNK_X_SIZE);
	int cmz = (int)((float)bposChanged->z / (float)CHUNK_Z_SIZE);

	addDirtyChkNoDups(cmx, cmz, bposChanged->y);

	int dmx, dmz;
	dmx = bposChanged->x % CHUNK_X_SIZE;
	dmz = bposChanged->z % CHUNK_Z_SIZE;

	// also update adjacent ones if we're on the edge
	if (dmx == 0 && cmx > 0) {
		addDirtyChkNoDups(cmx - 1, cmz);
	} else if (dmx == CHUNK_X_SIZE - 1 && cmx + 1 < Terrain::MAX_X / CHUNK_X_SIZE) {
		addDirtyChkNoDups(cmx + 1, cmz);
	}
	if (dmz == 0 && cmz > 0) {
		addDirtyChkNoDups(cmx, cmz - 1);
	} else if (dmz == CHUNK_Z_SIZE - 1 && cmz + 1 < Terrain::MAX_Z / CHUNK_Z_SIZE) {
		addDirtyChkNoDups(cmx, cmz + 1);
	}
}

bool ChunkMeshRenderer::chunkAlreadyDirty(int cmx, int cmz, bool entityUpdate) {
	static std::list<ScheduledChunk>::iterator it;
	static ScheduledChunk *chk;

	for (it = dirtyChunks.begin(); it != dirtyChunks.end(); ++it) {
		chk = &(*it);
		if (chk->x == cmx && chk->z == cmz && chk->entityUpdate == entityUpdate) {
			return true;
		}
	}

	return false;
}

void ChunkMeshRenderer::flushChunkUpdates() {
	static std::list<ScheduledChunk>::iterator it;
	static ScheduledChunk *chk;
	static int cmx, cmz;

	for (it = dirtyChunks.begin(); it != dirtyChunks.end(); ++it) {
		chk = &(*it);
		cmx = chk->x;
		cmz = chk->z;

		if (!chunkMeshes[cmx][cmz] || !entityBatches[cmx][cmz]) return;

		if (chk->entityUpdate) {
			entityBatches[cmx][cmz]->update();
		} else {
			chunkMeshes[cmx][cmz]->update(chk->changedPosY);
		}
	}

	dirtyChunks.clear();
}

inline void ChunkMeshRenderer::allocateChunk(int x, int z) {
	int minX = x * CHUNK_X_SIZE;
	int maxX = (x + 1) * CHUNK_X_SIZE;

	int minZ = z * CHUNK_Z_SIZE;
	int maxZ = (z + 1) * CHUNK_Z_SIZE;

	chunkMeshes[x][z] = new ChunkMesh(t, minX, maxX, minZ, maxZ);
	entityBatches[x][z] = new EntityBatch(t, rm, minX, maxX, minZ, maxZ);
}

// Execute scheduled allocations and frees
void ChunkMeshRenderer::updateChunks(int cix, int ciz) {
	std::list<ScheduledChunk>::iterator it;

	// load many stuff on setup!
	if (getTicks() - lastSceneUpdate >= ticksBetweenChunkUpdates || getTicks() - startTicks < 5000) { 
		numChangedChunks = 0;
		lastSceneUpdate = getTicks();
	}

	for (it = toAllocate.begin(); it != toAllocate.end();) {
		if (numChangedChunks >= MAX_CHUNK_UPDATES) return;

		int x = (*it).x;
		int z = (*it).z;

		// adding duplicates is faster than checking for them.
		if (!chunkMeshes[x][z] && chunkIsAdjacent(x, z, cix, ciz)) {
			allocateChunk(x, z);
			numChangedChunks++;
		}

		toAllocate.erase(it++);
	}

	for (it = toFree.begin(); it != toFree.end();) {
		if (numChangedChunks >= MAX_CHUNK_UPDATES) return;

		int x = (*it).x;
		int z = (*it).z;

		// adding duplicates is faster than checking for them.
		if (chunkMeshes[x][z] && !chunkIsAdjacent(x, z, cix, ciz)) {
			SAFE_DELETE(chunkMeshes[x][z]);
			SAFE_DELETE(entityBatches[x][z]);
			numChangedChunks++;
		}

		toFree.erase(it++);
	}
}

inline void ChunkMeshRenderer::tryToRenderChunk(int x, int z) const {
	if (chunkMeshes[x][z]) {
		chunkMeshes[x][z]->render(camPosY);
	}
}
	
void ChunkMeshRenderer::updateDaylight() {
	static ticks_t lastUpdate = 0;
	//const ticks_t DAYLIGHT_UPDATE_TICKS = 500;
	
	if(ChunkMesh::updateDaylightFactor()) {
		for(int x=0; x<NUM_CHUNKS_X_Z; x++) {
			for(int z=0; z<NUM_CHUNKS_X_Z; z++) {
				if(chunkMeshes[x][z]) {
					scheduledDaylightUpdates.push(ChunkIndex(x, z));
				}
			}
		}
		
		AnimalManager::setupAnimalMeshes(ChunkMesh::getDaylightFactor());
	}
	
	if(!scheduledDaylightUpdates.empty() && getTicks() - lastUpdate > ticksBetweenChunkUpdates) {
		ChunkIndex chunkIndex = scheduledDaylightUpdates.front();
		int x = chunkIndex.x;
		int z = chunkIndex.z;
		
		if(chunkMeshes[x][z])
			chunkMeshes[x][z]->update(-1);
		if(entityBatches[x][z])
			entityBatches[x][z]->update();
		
		scheduledDaylightUpdates.pop();
		lastUpdate = getTicks();
	}
}

void ChunkMeshRenderer::render() {
	static int cx, cy, cz, cix, ciz;
	
	updateDaylight();

	// Block digged/put, Entity removed/added
	flushChunkUpdates();

	Vec3 *camPos = cam->getPosPtr();

	cx = (int)camPos->x;
	cy = (int)camPos->y;
	cz = (int)camPos->z;

	cix = cx / CHUNK_X_SIZE;
	ciz = cz / CHUNK_Z_SIZE;

	updateChunks(cix, ciz);

	frustum.update();

	std::list<ScheduledChunk> ebToDraw;
	//ebToDraw.clear();

	for (int x = 0; x < NUM_CHUNKS_X_Z; x++) {
		for (int z = 0; z < NUM_CHUNKS_X_Z; z++) {
			if (!chunkMeshes[x][z] || !chunkIsAdjacent(x, z, cix, ciz))
				continue;

			BoundingBox* bbox = chunkMeshes[x][z]->getBoundingBox();

			if (camInBox(bbox) || frustum.boxInFrustum(bbox)) {
				chunkMeshes[x][z]->render(camPosY);
				ebToDraw.push_back(ScheduledChunk(x, z, true));
				animalManager->renderAnimalsInChk(x, z);				
			}
		}
	}

	static ScheduledChunk *eb;

	if (!ebToDraw.empty()) {
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
	}

	for (std::list<ScheduledChunk>::iterator it = ebToDraw.begin(); it != ebToDraw.end(); ++it) {
		eb = &(*it);
		entityBatches[eb->x][eb->z]->render();
	}

	if (!ebToDraw.empty()) {
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
	}
}

inline void ChunkMeshRenderer::allocateIfNeeded(int x, int z) {
	if (!chunkMeshes[x][z]) {
		toAllocate.push_back(ScheduledChunk(x, z, false));
	}
}

inline void ChunkMeshRenderer::freeIfNeeded(int x, int z) {
	if (chunkMeshes[x][z]) {
		toFree.push_back(ScheduledChunk(x, z, false));
	}
}

inline void ChunkMeshRenderer::manageChunk(int x, int z, int k) {
	if (x < 0 || z < 0 || x >= NUM_CHUNKS_X_Z || z >= NUM_CHUNKS_X_Z) {
		return;
	}

	if (k <= ADJ_CHUNK_DIST) {
		allocateIfNeeded(x, z);
	} else {
		freeIfNeeded(x, z);
	}
}

// Shedule allocation/frees on camera movement
void ChunkMeshRenderer::update(Vec3 *newCamPos) {
	if(keepMeshes)
		return;

	static int cix, ciz;
	
	camPosY = (int)newCamPos->y;

	cix = (int)newCamPos->x / CHUNK_X_SIZE;
	ciz = (int)newCamPos->z / CHUNK_Z_SIZE;

	if (cix < 0 || ciz < 0 || cix > (NUM_CHUNKS_X_Z + 1) * CHUNK_X_SIZE || ciz > (NUM_CHUNKS_X_Z + 1) * CHUNK_Z_SIZE) {
		return;
	}

	// always make sure chunk containing cam is allocated
	if (!chunkMeshes[cix][ciz]) {
		allocateChunk(cix, ciz);
	}

	// go from near to far for others
	for (int k = 1; k <= ADJ_CHUNK_DIST + 1; k++) {

		// left right vertical lines
		for (int x = cix - k; x <= cix + k; x += 2 * k) {
			for (int z = ciz - k; z <= ciz + k; z++) {
				manageChunk(x, z, k);
			}
		}

		// top bottom horizontal lines
		for (int z = ciz - k; z <= ciz + k; z += 2 * k) {
			for (int x = cix - (k - 1); x <= cix + (k - 1); x++) {
				manageChunk(x, z, k);
			}
		}

	}
}

bool ChunkMeshRenderer::camInBox(BoundingBox *bbox) const {
	Vec3 camPos = cam->getPos();
	return camPos.x >= bbox->min.x
		   && camPos.y >= bbox->min.y
		   && camPos.z >= bbox->min.z
		   && camPos.x <= bbox->max.x
		   && camPos.y <= bbox->max.y
		   && camPos.z <= bbox->max.z;
}


} /* namespace as */
