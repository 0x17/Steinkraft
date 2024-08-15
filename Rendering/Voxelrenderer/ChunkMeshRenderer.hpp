// WorldMeshRenderer.hpp

#ifndef WORLDMESHRENDERER_HPP_
#define WORLDMESHRENDERER_HPP_

#include <list>
#include <queue>

#include "../../Terrain.hpp"

#include "../Meshes/ChunkMesh.hpp"
#include "../Meshes/EntityBatch.hpp"

#include "IVoxelRenderer.hpp"

#include "../../Managers/AnimalManager.hpp"

namespace as {

#define ADJ_CHUNK_DIST gAdjChunkDist

extern int gAdjChunkDist;

class Camera;
class Frustum;
class SkyMesh;

class ScheduledChunk {
public:
	int x, z;
	bool entityUpdate;
	int changedPosY;

	ScheduledChunk(int _x, int _z, bool _entityUpdate, int _changedPosY = -1)
	: x(_x), z(_z), entityUpdate(_entityUpdate), changedPosY(_changedPosY) {}

	bool operator==(const ScheduledChunk &other) const {
		return x == other.x && z == other.z;
	}
};

class DirtyChunk : public ScheduledChunk {
public:
	int dmx, dmz;

	DirtyChunk(int _x, int _z, int _dmx, int _dmz, bool _entityUpdate)
	: ScheduledChunk(_x, _z, _entityUpdate), dmx(_dmx), dmz(_dmz) {}
};

struct ChunkIndex {
	int x, z;
	
	ChunkIndex(int _x, int _z) : x(_x), z(_z) {}
};

class ChunkMeshRenderer : public IVoxelRenderer, Observer<Vec3> {
public:
	ChunkMeshRenderer(Terrain *t, RailManager *rm, Camera *cam, AnimalManager *animalManager);
	virtual ~ChunkMeshRenderer();

	virtual void update(BlockPos *bposChanged);
	virtual void render();

	virtual void update(Vec3 *newCamPos);
	
	enum ChunkDimensions {
		CHUNK_X_SIZE = Terrain::CHUNK_SIZE,
		CHUNK_Z_SIZE = Terrain::CHUNK_SIZE,
		NUM_CHUNKS_X_Z = Terrain::TERRAIN_SIZE / Terrain::CHUNK_SIZE
	};

private:
	bool camInBox(BoundingBox *bbox) const;
	bool chunkIsAdjacent(int chunkX, int chunkZ, int camX, int camZ) const;
	void renderNonOccluded();
	void reset();
	void updateChunks(int cix, int ciz);
	void setupSeaPlane();

	bool chunkAlreadyDirty(int cmx, int cmz, bool entityUpdate);

	void flushChunkUpdates();

	void allocateChunk(int x, int z);
	void tryToRenderChunk(int x, int z) const;

	void allocateIfNeeded(int x, int z);
	void freeIfNeeded(int x, int z);
	void manageChunk(int x, int z, int k);

	void addDirtyChkNoDups(int cmx, int cmz, int changedPosY = -1);
	
	void freeAllMeshes();
	
	void updateDaylight();
	
	ChunkMesh *chunkMeshes[NUM_CHUNKS_X_Z][NUM_CHUNKS_X_Z];
	EntityBatch *entityBatches[NUM_CHUNKS_X_Z][NUM_CHUNKS_X_Z];

	Terrain *t;
	RailManager *rm;
	Camera *cam;
	Frustum &frustum;

	std::list<ScheduledChunk> toAllocate, toFree, dirtyChunks;
	std::queue<ChunkIndex> scheduledDaylightUpdates;

	AnimalManager *animalManager;

	ticks_t lastSceneUpdate, ticksBetweenChunkUpdates;
	ticks_t startTicks;
	
	int camPosY;
	
	enum Consts {
		MAX_CHUNK_UPDATES = 1,
		EDGE_DIST = 2,
		WORLD_EDGE_DIST = 2
	};
};

inline bool ChunkMeshRenderer::chunkIsAdjacent(int chunkX, int chunkZ, int camX, int camZ) const {
	return (chunkX >= camX - ADJ_CHUNK_DIST && chunkX <= camX + ADJ_CHUNK_DIST
			&& chunkZ >= camZ - ADJ_CHUNK_DIST && chunkZ <= camZ + ADJ_CHUNK_DIST);
}

} /* namespace as */
#endif /* WORLDMESHRENDERER_HPP_ */
