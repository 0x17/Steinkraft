// Terrain.hpp

#ifndef TERRAIN_HPP_
#define TERRAIN_HPP_

#include <list>
#include <string>
#include <map>

#include "Framework/Math/Vector.hpp"

#include "Framework/Observable.hpp"
#include "Framework/Camera.hpp"

#include "BlockPos.hpp"
#include "VisibleFaces.hpp"
#include "Entity.hpp"

//===========================================================================
// Constants/Macros
//===========================================================================
namespace as {

typedef uchar DATA_TYPE;

extern int nblocks_near;

//===========================================================================
// Types
//===========================================================================
class Terrain : public Observable<BlockPos> {
public:
	enum TerrainSource {
		TS_EMPTY,
		TS_FILE,
		TS_SPHERE,
		TS_PYRAMID,
		TS_RANDOM,
		TS_PERLIN,
		TS_FLAT
	};
	
	Terrain(TerrainSource source, int seed);
	virtual ~Terrain();

	// terrain persistency
	void loadTerrainFromFile(const char *filename = NULL);
	void saveTerrainToFile(const char *filename = NULL) const;
	void loadEntitiesFromFile(const char *filename);
	void saveEntitiesToFile(const char *filename) const;

	void blocksNear(Camera *cam, std::list<BlockPos> *blocksNear) const;

	VisibleFaces determineVisibleFaces(int x, int y, int z) const;
	bool isValidIndex(int x, int y, int z) const;

	// terrain data (voxels) related methods
	DATA_TYPE get(int x, int y, int z) const;
	DATA_TYPE getValid(int x, int y, int z) const;
	DATA_TYPE *getDataPtr();

	void set(int x, int y, int z, DATA_TYPE val);
	void quickSet(int x, int y, int z, DATA_TYPE val);
	void lazySet(int x, int y, int z, DATA_TYPE val);
	void lazySetFlush();

	bool isEmptyPos(int x, int y, int z) const;
	bool isEmptyPos(float x, float y, float z) const;
	bool isEmptyPos(Vec3 v) const;

	int numBlocksAbove(int x, int y, int z) const;
	bool isBlockAbove(int x, int y, int z) const;
	int getYOfBlockBelow(int x, int y, int z) const;
	float calcDensityAroundPos(int x, int y, int z, int envSize) const;
	float dYtoSolidBelow(int sx, float sy, int sz) const;

	// entity (ladders, torches, ...) related methods
	std::list<Entity> *getEntitiesPtr();
	bool addEntity(Entity entity);
	std::list<Entity> getEntitiesInArea(int minX, int maxX, int minZ, int maxZ,
										int *numGlass, int *numStanding, int *numDoors) const;
	bool isEntityUpdate() const;
	bool removeEntityAt(int x, int y, int z, CubeFace cface = (CubeFace)23);
	float distToNearestLight(float x, float y, float z) const;
	std::list<Entity> getEntitiesAtPos(int x, int y, int z, Entity::EntityType type = (Entity::EntityType)23) const;
	bool hasLadderOnFace(int x, int y, int z, CubeFace face) const;
	bool hasEntities() const;
	Entity *getLastEntity();
	bool isEntityDeletion() const;
	bool openDoorAt(int x, int y, int z) const;
	bool isEmptyOrGlass(int x, int y, int z) const;

	std::list<Entity> getEntitiesOfType(Entity::EntityType etype) const;
	
	enum Dimensions {
		TERRAIN_SIZE = 256,
		CHUNK_SIZE = 16,
		MAX_Y = 64,

		MAX_X = TERRAIN_SIZE,
		MAX_Z = TERRAIN_SIZE		
	};
	
	enum TexIndices {
		INVIS_SOLID = 254,
		INVIS_DOOR	= 255,

		CART_TEX_INDEX = 200,
		FENCE_TEX_INDEX = 201,
		WATER_TEX_INDEX = 10
	};

private:
	// terrain generation
	void clearTerrain();
	void generateSpherishTerrain();
	void generateRandomTerrain(bool smallSteps);
	void generatePerlinTerrain();
	void generatePyramidTerrain();
	void generateFlatTerrain(DATA_TYPE tid = 1);

	// auxiliary methods
	DATA_TYPE randomlyChooseTexId() const;
	DATA_TYPE chooseTexForHeight(int blockHeight, int colHeight);
	void addTree(int baseX, int baseY, int baseZ);
	int roughness(int x, int z);
	
	DATA_TYPE data[MAX_X * MAX_Y * MAX_Z];

	std::list<Entity> entities;
	bool entityUpdate;
	int seed;

	std::list<BlockPos> changedBlocks;

	Entity *lastEntity;
	bool deleteEntity;

	BlockPos setBlockPos;
	
	int numWaterBlocks;

	enum Consts {
		MAX_SMALL_STEP_DIFF	= 5,
		MAX_RAND_HEIGHT		= 5,
		DEFAULT_HEIGHT		= MAX_Y / 2,

		NUM_OCTAVES		= 6,
		TREE_TOP_TEX	= 6,
		TREE_BASE_TEX	= 10,

		MAX_BLOCKS = (MAX_X*MAX_Y*MAX_Z),
		MAX_WATER_BLOCKS = 320,

		// height used for perlin noise terrain generation
		TMAX_Y = MAX_Y / 2
	};
};

//===========================================================================
// Inlined implementations
//===========================================================================

inline bool isInvisible(int x) {
	return ((x) == Terrain::INVIS_SOLID || (x) == Terrain::INVIS_DOOR);
}

inline bool Terrain::hasEntities() const { return !entities.empty(); }
inline Entity *Terrain::getLastEntity() { return lastEntity; }
inline bool Terrain::isEntityDeletion() const { return deleteEntity; }
inline DATA_TYPE *Terrain::getDataPtr() { return data; }

inline void Terrain::quickSet(int x, int y, int z, DATA_TYPE val) {
	data[x*(MAX_Y*MAX_Z)+y*MAX_Z+z] = val;
}

inline void Terrain::set(int x, int y, int z, DATA_TYPE val) {
	quickSet(x, y, z, val);
	setBlockPos.x = x;
	setBlockPos.y = y;
	setBlockPos.z = z;
	entityUpdate = false;
	notifyObservers(&setBlockPos);
}

inline bool Terrain::isEntityUpdate() const {
	return entityUpdate;
}

inline std::list<Entity> *Terrain::getEntitiesPtr() {
	return &entities;
}

inline float Terrain::dYtoSolidBelow(int sx, float sy, int sz) const {
	int y = (int)sy;
	float dY = sy - y;
	while (y > 0 && isEmptyPos(sx, y, sz)) {
		dY++;
		y--;
	}
	return dY;
}

inline DATA_TYPE Terrain::get(int x, int y, int z) const {
	return data[x*(MAX_Y*MAX_Z)+y*MAX_Z+z];
}

inline DATA_TYPE Terrain::getValid(int x, int y, int z) const {
	return (isValidIndex(x, y, z)) ? get(x, y, z) : 0;
}

inline bool Terrain::isValidIndex(int x, int y, int z) const {
	return x >= 0 && y >= 0 && z >= 0 && x < MAX_X && y < MAX_Y && z < MAX_Z;
}

//==============================================================

inline bool Terrain::isEmptyPos(int x, int y, int z) const {
	if (!isValidIndex(x, y, z))
		return true;
	return (get(x, y, z) == 0 || (get(x, y, z) == INVIS_DOOR && openDoorAt(x, y, z)));
}
inline bool Terrain::isEmptyPos(float x, float y, float z) const {
	return isEmptyPos((int)x, (int)y, (int)z);
}
inline bool Terrain::isEmptyPos(Vec3 v) const {
	return isEmptyPos(v.x, v.y, v.z);
}

} /* namespace as */
#endif /* TERRAIN_HPP_ */
