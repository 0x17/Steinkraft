// Terrain.cpp

#include "StdAfx.h"
#pragma hdrstop

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <fstream>

#include "Framework/Utilities.hpp"
#include "Framework/Math/Noise.hpp"

#include "Terrain.hpp"
#include "Constants.h"

//===========================================================================
// Constants
//===========================================================================
namespace as {
#define NEAR_DIST nearDist

const char *DEF_FILENAME = "terrain.dump";

inline float LDIST(BlockPos pos, float x, float y, float z) {
	return (Vec3(pos.x + 0.5f, pos.y, pos.z + 0.5f) - Vec3(x, y, z)).length();
}

//===========================================================================
// Globals
//===========================================================================
int nearDist;
int nblocks_near;

static DATA_TYPE FAV_TEX_IDS[] = {
	0, 1, 4, 5, 6, 9, 11, 12, 15, 16, 17, 18, 32, 33, 34, 4
};

//===========================================================================
// Methods
//===========================================================================

inline bool Terrain::isEmptyOrGlass(int x, int y, int z) const {
	DATA_TYPE val = get(x,y,z);
	return (!val || isInvisible(val) || (val == Terrain::FENCE_TEX_INDEX + 1));
}

Terrain::Terrain(TerrainSource source, int _seed)
:	entityUpdate(false),
	seed(_seed),
	lastEntity(NULL),
	deleteEntity(false),
	numWaterBlocks(0)
{
	if (visualDetail == DETAIL_VERY_LOW)
		nearDist = 4;
	else if (visualDetail == DETAIL_LOW)
		nearDist = 6;
	else
		nearDist = 8;

	switch (source) {
	case TS_EMPTY:
		// do nothing.
		break;
	case TS_FILE:
		loadTerrainFromFile(NULL);
		break;
	case TS_SPHERE:
		generateSpherishTerrain();
		break;
	case TS_PYRAMID:
		generatePyramidTerrain();
		break;
	case TS_RANDOM:
		generateRandomTerrain(true);
		break;
	case TS_FLAT:
		generateFlatTerrain();
		break;
	case TS_PERLIN:
		generatePerlinTerrain();
		break;
	default:
		error("Unknown terrain source!");
		break;
	}
}

Terrain::~Terrain() {
	SAFE_DELETE(lastEntity);
}

void Terrain::loadTerrainFromFile(const char *filename) {
#if IPHONE && 0
	// Check for old uncompressed world on IOS
	if(IOS_FileExists(filename, false)) {
		DATA_TYPE smallBuf[256*256*16];
		binaryRead(filename, smallBuf, sizeof(DATA_TYPE) * 256*256*16);
		clearTerrain();
		for(int x=0; x<256; x++)
			for(int y=0; y<16; y++)
				for(int z=0; z<256; z++)
					quickSet(x, y, z, smallBuf[x*(16*256)+y*256+z]);
		return;
	}
#endif
	binaryRead((!filename ? DEF_FILENAME : filename), (char *)data, sizeof(DATA_TYPE) * MAX_BLOCKS);
}

void Terrain::saveTerrainToFile(const char *filename) const {
	binaryWrite((!filename ? DEF_FILENAME : filename), (char *)data, sizeof(DATA_TYPE) * MAX_BLOCKS);
}

void Terrain::loadEntitiesFromFile(const char *filename) {
	char entFilename[BUF_LEN], entDescrFilename[BUF_LEN];
	int l;

	strcpy(entFilename, filename);
	strcpy(entDescrFilename, filename);

	strcat(entFilename, ".entities");
	strcat(entDescrFilename, ".edescr");

	if (!fileExists(entFilename) || !fileExists(entDescrFilename)) return;

	binaryRead(entDescrFilename, &l, sizeof(int));

	Entity *entArray = new Entity[l];
	binaryRead(entFilename, entArray, sizeof(Entity) * l);

	entities.clear();

	for (int i = 0; i < l; i++) {
		//entities.push_back(entArray[i]);
		addEntity(entArray[i]);
	}

	SAFE_DELETE_ARRAY(entArray);
}

void Terrain::saveEntitiesToFile(const char *filename) const {
	char entFilename[BUF_LEN], entDescrFilename[BUF_LEN];
	size_t l = entities.size();

	//if(!l) return;

	strcpy(entFilename, filename);
	strcpy(entDescrFilename, filename);

	strcat(entFilename, ".entities");
	strcat(entDescrFilename, ".edescr");

	binaryWrite(entDescrFilename, &l, sizeof(int));

	Entity *entArray = new Entity[l];

	std::list<Entity>::const_iterator it;
	int i = 0;
	for (it = entities.begin(); it != entities.end(); ++it) {
		entArray[i++] = (*it);
	}

	binaryWrite(entFilename, entArray, sizeof(Entity) * l);

	SAFE_DELETE_ARRAY(entArray);
}

void Terrain::blocksNear(Camera *cam, std::list<BlockPos> *blocksNear) const {
	static float alpha;
	static Vec3 *camPos, camToPoint;
	static int x, y, z;

	camPos = cam->getPosPtr();

	x = (int)camPos->x;
	y = (int)camPos->y;
	z = (int)camPos->z;

	for (int a = x - NEAR_DIST; a <= x + NEAR_DIST; a++) {
		for (int b = y - NEAR_DIST; b <= y + NEAR_DIST; b++) {
			for (int c = z - NEAR_DIST; c <= z + NEAR_DIST; c++) {
				if (a < 0 || b < 0 || c < 0 || a >= MAX_X || b >= MAX_Y || c >= MAX_Z)
					continue;

				// also allow selection of invisible blocks (for entities)
				if (get(a, b, c) != 0) {
					// heuristic to drop blocks outside of FOV
					camToPoint.setTo(a + 0.5f, b + 0.5f, c + 0.5f);
					camToPoint.subInPlace(camPos);
					alpha = camToPoint.angleBetweenNor(cam->getNormal());
					if (alpha >= FOV_RAD + PI / 10.0f)
						continue;

					blocksNear->push_back(BlockPos(a, b, c));
				}
			}
		}
	}
}

VisibleFaces Terrain::determineVisibleFaces( int x, int y, int z ) const {
	VisibleFaces tmpVisFaces;

	tmpVisFaces.front = (z == MAX_Z - 1 || isEmptyOrGlass(x, y, z + 1));
	tmpVisFaces.back = (z == 0 || isEmptyOrGlass(x, y, z - 1));

	tmpVisFaces.left = (x == 0 || isEmptyOrGlass(x - 1, y, z));
	tmpVisFaces.right = (x == MAX_X - 1 || isEmptyOrGlass(x + 1, y, z));

	tmpVisFaces.bottom = (y == 0 || isEmptyOrGlass(x, y - 1, z));
	tmpVisFaces.top = (y == MAX_Y - 1 || isEmptyOrGlass(x, y + 1, z));

	return tmpVisFaces;
}

void Terrain::clearTerrain() {
	memset(data, 0, sizeof(DATA_TYPE) * MAX_BLOCKS);
}

void Terrain::generateSpherishTerrain() {
	int x, y, z;
	Vec3 center(MAX_X / 2, MAX_Y / 2, MAX_Z / 2);

	for (x = 0; x < MAX_X; x++) {
		for (y = 0; y < MAX_Y; y++) {
			for (z = 0; z < MAX_Z; z++) {
				Vec3 tmp((float)x, (float)y, (float)z);
				Vec3 diff = tmp - center;
				float dst = diff.length();
				quickSet(x, y, z, (y == 0) ? 1 : (dst > MAX_Y / 2 - 2 && dst < MAX_Y / 2) ? FAV_TEX_IDS[y/4] + 1 : 0);
			}
		}
	}
}

void Terrain::generateRandomTerrain(bool smallSteps) {
	int x, y, z, height, oldHeight = DEFAULT_HEIGHT;

	std::srand((int)std::time(NULL));

	for (x = 0; x < MAX_X; x++) {
		for (z = 0; z < MAX_Z; z++) {
			if (smallSteps) {
				height = oldHeight + (rand() % (MAX_SMALL_STEP_DIFF) - 1);
				// clamp between 0 and TERRAIN_D-1
				height = (height >= MAX_Y) ? MAX_Y - 1 : height;
				height = (height < 0) ? 0 : height;
			} else {
				height = rand() % (MAX_RAND_HEIGHT);
			}

			for (y = 0; y < MAX_Y; y++) {
				quickSet(x, y, z, (y <= height) ? randomlyChooseTexId() : 0);
			}
		}
	}

}

enum BlockTexIndices {
	TID_WATER = 10,
	TID_SAND = 12,
	TID_GRASS = 0,
	TID_DIRT = 1,
	TID_SNOW = 6,
	TID_BRIGHT_STONE = 19,
	TID_DARK_STONE = 25,
	TID_GOLD_STONE = 20,
	TID_SAND_BRICKS = 35,
	TID_STONE_VAR = 33
};

DATA_TYPE Terrain::chooseTexForHeight(int blockHeight, int colHeight) {
	float k = (float)blockHeight / (float)colHeight;
	float l = (float)blockHeight / TMAX_Y;
	float m = (float)colHeight / TMAX_Y;
	DATA_TYPE r;
	
	float rval = 0.01f * (rand() % 5 - 2);

	if (l >= 0.3f) {
		if (k <= 0.3f + rval) r = (rand() % 5 == 0) ? TID_BRIGHT_STONE : TID_STONE_VAR;
		else if (k <= 0.45f + rval) r = TID_DARK_STONE;
		else if (k <= (0.75f + (rand() % 8)*0.01f)) r = (rand() % 100 == 0) ? TID_GOLD_STONE : TID_BRIGHT_STONE;
		else if (k <= 0.90f + rval) r = TID_DIRT;
		else r = (TMAX_Y - colHeight < 2) ?  TID_SNOW : (blockHeight == colHeight) ? TID_GRASS : TID_DIRT;
	} else if (l <= 0.1f && colHeight <= 1) {
		r = (numWaterBlocks < MAX_WATER_BLOCKS) ? TID_WATER : TID_SAND;
		numWaterBlocks++;
	}
	else if (l <= 0.2f + rval && m < 0.2f) r = (rand() % 5 == 0) ? TID_SAND_BRICKS : TID_SAND;
	else r = (rval == 0) ? TID_BRIGHT_STONE : TID_DARK_STONE;

	return r + 1;
}

void Terrain::addTree(int baseX, int baseY, int baseZ) {
	if (!isValidIndex(baseX - 1, baseY, baseZ - 1) || !isValidIndex(baseX + 1, baseY + 5 + 3, baseZ + 1))
		return;

	if (baseY + 3 + 4 + 1 >= TMAX_Y) return;
	
	if(rand() % 2 == 0) return;

	int treeBaseHeight = rand() % 4 + 4;

	for (int i = 1; i <= treeBaseHeight; i++) {
		quickSet(baseX, baseY + i, baseZ, TREE_BASE_TEX);
	}

	// tree top
	for(int i = -2; i <= 2; i++) {	
		for(int j= -2; j <=2; j++) {
			if ((i == 0 && j == 0) || (i == 2 && j == 2) || (i == -2 && j == -2) || (i == -2 && j == 2) || (i == 2 && j == -2))
				continue;
			
			quickSet(baseX + i, baseY + treeBaseHeight - 1, baseZ + j, TREE_TOP_TEX);

			if (i > 1 || i < -1 || j > 1 || j < -1 || rand() % 3 == 0)
				continue;

			quickSet(baseX + i, baseY + treeBaseHeight - 2, baseZ + j, TREE_TOP_TEX);
			quickSet(baseX + i, baseY + treeBaseHeight, baseZ + j, TREE_TOP_TEX);
		}
	}

	if (rand() % 5 == 0) return;

	quickSet(baseX - 1, baseY + treeBaseHeight + 1, baseZ, TREE_TOP_TEX);
	quickSet(baseX + 1, baseY + treeBaseHeight + 1, baseZ, TREE_TOP_TEX);
	quickSet(baseX, baseY + treeBaseHeight + 1, baseZ - 1, TREE_TOP_TEX);
	quickSet(baseX, baseY + treeBaseHeight + 1, baseZ + 1, TREE_TOP_TEX);
	quickSet(baseX, baseY + treeBaseHeight + 1, baseZ, TREE_TOP_TEX);
}

int Terrain::roughness(int x, int z) {
	const float zoom = 60;

	static float freq;
	static float amp;
	static int rval = rand();
	
	freq = 4;
	amp = 8;
	
	return (int)(noise(((float)x) * freq / zoom, ((float)z) / zoom * freq, rval) * amp);
}

void Terrain::generatePerlinTerrain() {
	int x, y, z, k, height;
	DATA_TYPE texNr = 0;
	float n, freq, amp;

	float persistence = 0.3f;
	float zoom = 90;

	clearTerrain();
	
	srand(seed);
	int rval = rand();

	zoom -= rval % 10;
	persistence += (rval % 10) * 0.01f;

	for (x = 0; x < MAX_X; x++) {
		for (z = 0; z < MAX_Z; z++) {
			n = 0;

			for (k = 0; k < NUM_OCTAVES - 1; k++) {
				freq = std::pow(2.0f, (float)k); // powf?
				amp = std::pow(persistence, (float)k); // powf?
				n += noise(((float)x) * freq / zoom, ((float)z) / zoom * freq, rval) * amp;
			}

			height = (int)(n * ((float)TMAX_Y / 2.0f) + (float)TMAX_Y / 2.0f);
			
			height += roughness(x, z);
			
			height = (height > TMAX_Y) ? TMAX_Y : height;
			height = (height <= 0) ? 1 : height;

			for (y = 0; y < TMAX_Y; y++) {
				if (y <= height) {
					texNr = chooseTexForHeight(y, height);
					quickSet(x, y, z, texNr);

					// swiss cheese
					if (rand() % 20 == 0 && y == height && y > 1) {
						DATA_TYPE tmp = get(x, y, z);
						quickSet(x, y, z, 0);
						quickSet(x, y - 1, z, tmp);
					}
				}
			}

			if (x % 10 == 0 && z % 10 == 0 && height >= (TMAX_Y*0.25f)) {
				// no trees on water
				if (texNr != 11) {
					addTree(x, height, z);
				}
			}
		}
	}
}

void Terrain::generatePyramidTerrain() {
	int x, y, z, p, q;
	int xOffset = MAX_X / 2;
	int zOffset = MAX_Z / 2;
	
	const DATA_TYPE tid = 13;

	generateFlatTerrain(tid);

	for (x = 0; x < MAX_Y; x++) {
		for (z = 0; z < MAX_Y; z++) {
			// always fill ground
			quickSet(x + xOffset, 0, z + zOffset, tid);

			for (y = 0; y < MAX_Y; y++) {
				if (x < MAX_Y / 2 && z < MAX_Y / 2) {
					p = x;
					q = z;
					quickSet(x + xOffset, p < q ? p : q, z + zOffset, tid);
				} else if (x >= MAX_Y / 2 && z >= MAX_Y / 2) {
					p = MAX_Y - 1 - x;
					q = MAX_Y - 1 - z;
					quickSet(x + xOffset, p < q ? p : q, z + zOffset, tid);
				} else if (x <= MAX_Y / 2 && z >= MAX_Y / 2) {
					p = x;
					q = MAX_Y - 1 - z;
					quickSet(x + xOffset, p < q ? p : q, z + zOffset, tid);
				} else if (x >= MAX_Y / 2 && z <= MAX_Y / 2) {
					p = MAX_Y - 1 - x;
					q = z;
					quickSet(x + xOffset, p < q ? p : q, z + zOffset, tid);
				}
			}
		}
	}
}

void Terrain::generateFlatTerrain(DATA_TYPE tid) {
	int x, z;

	clearTerrain();

	for (x = 0; x < MAX_X; x++) {
		for (z = 0; z < MAX_Z; z++) {
			quickSet(x, 0, z, tid);
			quickSet(x, 1, z, tid);
		}
	}
}

DATA_TYPE Terrain::randomlyChooseTexId() const {
	return (DATA_TYPE)(rand() % 64 + 1);
}

int Terrain::numBlocksAbove(int x, int y, int z) const {
	int nba = 0;
	for (int k = y + 1; k < MAX_Y; k++) {
		if (get(x, k, z) != 0 && !isInvisible(get(x, k, z)))
			nba++;
	}
	return nba;
}

bool Terrain::isBlockAbove(int x, int y, int z) const {
	if (x < 0 || y < 0 || z < 0 || x >= MAX_X || y >= MAX_Y || z >= MAX_Z)
		return false;

	for (int k = y + 1; k < MAX_Y; k++) {
		if (get(x, k, z) != 0 && !isInvisible(get(x, k, z)) && get(x,k,z) != (Terrain::FENCE_TEX_INDEX+1))
			return true;
	}
	return false;
}

void Terrain::lazySet(int x, int y, int z, DATA_TYPE val) {
	set(x, y, z, val);
	BlockPos bpos(x, y, z);
	changedBlocks.push_back(bpos);
}

void Terrain::lazySetFlush() {
	if (changedBlocks.empty()) return;

	entityUpdate = false;

	std::list<BlockPos>::iterator it;
	for (it = changedBlocks.begin(); it != changedBlocks.end(); ++it) {
		notifyObservers(&(*it));
	}

	changedBlocks.clear();
}

int Terrain::getYOfBlockBelow(int x, int y, int z) const {
	for (int cy = y - 1; cy > 0; cy--) {
		if (!isEmptyPos(x, cy, z))
			return cy;
	}
	return 0;
}

float Terrain::calcDensityAroundPos(int x, int y, int z, int envSize) const {
	float density = 0.0f;
	for (int xc = x - envSize; xc <= x + envSize; xc++) {
		for (int yc = y - envSize; yc <= y + envSize; yc++) {
			for (int zc = z - envSize; zc <= z + envSize; zc++) {
				if (!isEmptyPos(xc, yc, zc))
					density++;
			}
		}
	}
	density /= ((envSize * 2 + 1) * (envSize * 2 + 1) * (envSize * 2 + 1));
	return density;
}

bool Terrain::addEntity(Entity entity) {
	std::list<Entity>::iterator it;
	bool alreadyExists = false;

	// make sure this entity isn't already part of our entity list
	for (it = entities.begin(); it != entities.end(); ++it) {
		if ((*it) == entity) {
			alreadyExists = true;
			break;
		}
	}

	if (!alreadyExists) {
		entities.push_back(entity);
		SAFE_DELETE(lastEntity);
		lastEntity = new Entity(entity);
		entityUpdate = true;
		notifyObservers(&entity.pos);

		// torches light up surrounding area
		if (entity.type == Entity::TORCH) {
			entityUpdate = false;
			notifyObservers(&entity.pos);
		}

		return true;
	}

	return false;
}

std::list<Entity> Terrain::getEntitiesInArea(int minX, int maxX, int minZ, int maxZ, int *numGlass, int *numStanding, int *numDoors) const {
	std::list<Entity>::const_iterator it;
	const BlockPos *bpos;
	std::list<Entity> eia;
	for (it = entities.begin(); it != entities.end(); ++it) {
		//if((*it).type == ENTITY_RAIL) continue;

		bpos = &(*it).pos;
		if (bpos->x >= minX && bpos->x <= maxX && bpos->z >= minZ && bpos->z <= maxZ) {
			eia.push_back(*it);
			Entity::EntityType etype = (*it).type;
			if (etype == Entity::GLASS) {
				(*numGlass)++;
			} else if (etype == Entity::FLOWER || etype == Entity::MUSHROOM
					   || (etype == Entity::TORCH && (*it).cface == CF_TOP)) {
				(*numStanding)++;
			} else if (Entity::isDoorIndex(etype)) {
				(*numDoors)++;
			}
		}
	}
	return eia;
}

bool Terrain::removeEntityAt(int x, int y, int z, CubeFace cface) {
	bool dontRemBlock = false;
	std::list<Entity>::iterator it;
	BlockPos dpos(x, y, z);

	for (it = entities.begin(); it != entities.end();) {
		if ((*it).pos == dpos) {
			dontRemBlock = true;

			if ((*it).cface == cface || cface == (CubeFace)23) {
				deleteEntity = true;
				SAFE_DELETE(lastEntity);
				lastEntity = new Entity(*it);

				entityUpdate = true;
				notifyObservers(&dpos);

				// torches light up surrounding area
				if ((*it).type == Entity::TORCH) {
					entityUpdate = false;
					notifyObservers(&dpos);
				}

				entities.erase(it++);
				deleteEntity = false;

				continue;
			}
		}
		++it;
	}

	return dontRemBlock;
}

float Terrain::distToNearestLight(float x, float y, float z) const {
	std::list<Entity>::const_iterator it;
	const Entity *et;
	float minDist = (float)TERRAIN_SIZE, dst;
	for (it = entities.begin(); it != entities.end(); ++it) {
		et = &(*it);
		if (et->type != Entity::TORCH) continue;

		if (et->cface == CF_LEFT && x > et->pos.x) continue;
		if (et->cface == CF_RIGHT && x < et->pos.x) continue;
		if (et->cface == CF_FRONT && z < et->pos.z) continue;
		if (et->cface == CF_BACK && z > et->pos.z) continue;

		dst = LDIST(et->pos, x, y, z);
		if (dst < minDist)
			minDist = dst;
	}

	return minDist;
}

std::list<Entity> Terrain::getEntitiesAtPos(int x, int y, int z, Entity::EntityType type) const {
	std::list<Entity>::const_iterator it;
	BlockPos dpos(x, y, z);
	std::list<Entity> eap;

	for (it = entities.begin(); it != entities.end(); ++it) {
		if ((*it).pos == dpos && ((*it).type == type || type == (Entity::EntityType)23)) {
			eap.push_back((*it));
		}
	}

	return eap;
}

bool Terrain::hasLadderOnFace(int x, int y, int z, CubeFace face) const {
	std::list<Entity>::const_iterator it;
	BlockPos dpos(x, y, z);
	for (it = entities.begin(); it != entities.end(); ++it) {
		if ((*it).pos == dpos && (*it).type == Entity::LADDER && (*it).cface == face) {
			return true;
		}
	}
	return false;
}

bool Terrain::openDoorAt(int x, int y, int z) const {
	std::list<Entity> eap = getEntitiesAtPos(x, y, z);
	std::list<Entity> eapBelow = getEntitiesAtPos(x, y - 1, z);
	std::list<Entity>::const_iterator it;

	for (it = eap.begin(); it != eap.end(); ++it) {
		if ((*it).type == Entity::DOOR_X_OPEN ||
				(*it).type == Entity::DOOR_Z_OPEN)
			return true;
	}

	for (it = eapBelow.begin(); it != eapBelow.end(); ++it) {
		if ((*it).type == Entity::DOOR_X_OPEN ||
				(*it).type == Entity::DOOR_Z_OPEN)
			return true;
	}

	return false;
}

std::list<Entity> Terrain::getEntitiesOfType( Entity::EntityType etype ) const
{
	std::list<Entity> result;
	std::list<Entity>::const_iterator it;
	for(it = entities.begin(); it != entities.end(); ++it) {
		if((*it).type == etype) 
			result.push_back((*it));
	}

	return result;
}

} /* namespace as */
