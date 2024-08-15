// EntityBatch.cpp



#include "EntityBatch.hpp"
#include "CubeVertices.hpp"
#include "ChunkMesh.hpp"
#include "../../Framework/Utilities.hpp"
#include "../../Managers/RailManager.hpp"

namespace as {

const int ENT_QUAD_COMPS = 3 + 2 + 4;

EntityBatch::EntityBatch(Terrain *_t, RailManager *_rm, int _minX, int _maxX, int _minZ, int _maxZ)
		: empty(true), t(_t), railManager(_rm), minX(_minX), maxX(_maxX), minZ(_minZ), maxZ(_maxZ), mesh(NULL) {
	update();
}

EntityBatch::~EntityBatch() {
	SAFE_DELETE(mesh);
}

int EntityBatch::addQuadOnBlockFace(BlockPos *pos, CubeFace cface, int trow, int tcol,
									float *coords, int startIndex, float brightness, float offX, float offY, float offZ) {
	static float offsets[3];
	TexCell tcell(trow, tcol);
	offsets[0] = pos->x + offX;
	offsets[1] = pos->y + offY;
	offsets[2] = pos->z + offZ;
	startIndex = buildTri(cface * 12, 0, 1, 2, coords, startIndex, &tcell, 1.0f, false, offsets, brightness);
	startIndex = buildTri(cface * 12, 2, 3, 0, coords, startIndex, &tcell, 1.0f, false, offsets, brightness);
	return startIndex;
}

void EntityBatch::update() {
	int numGlass, numStanding, numDoors;
	numGlass = numStanding = numDoors = 0;

	std::list<Entity> entities = t->getEntitiesInArea(minX, maxX, minZ, maxZ, &numGlass, &numStanding, &numDoors);
	if (entities.empty()) {
		SAFE_DELETE(mesh);
		empty = true;
		return;
	}

	std::list<Entity>::iterator it;
	Entity *e;
	int trow, tcol;
	float brightness;
	int nx, nz;

	int l = (int)((entities.size() + numGlass * 5 + numStanding * 3 + numDoors * 3) * VERTICES_PER_QUAD * ENT_QUAD_COMPS);

	float *cs = new float[l];
	int k = 0;

	for (it = entities.begin(); it != entities.end(); ++it) {
		e = &(*it);

		nx = nz = 0;

		switch (e->cface) {
		case CF_FRONT:
			nz = 1;
			break;
		case CF_BACK:
			nz = -1;
			break;
		case CF_LEFT:
			nx = -1;
			break;
		case CF_RIGHT:
			nx = 1;
			break;
		default:
			break;
		}

		if (Entity::isDoorIndex(e->type)) {
			nx = nz = 0;
		}

		brightness = t->isBlockAbove(e->pos.x + nx, e->pos.y, e->pos.z + nz) ? 0.5f : 1.0f;

		tcol = 8;
		trow = 2 + e->type;
		brightness = (e->type == Entity::TORCH) ? 1.0f : brightness * ChunkMesh::getDaylightFactor();

		if (e->type == Entity::FLOWER
				|| (e->type == Entity::TORCH && e->cface == CF_TOP)
				|| e->type == Entity::MUSHROOM) {
			k = addQuadOnBlockFace(&e->pos, CF_LEFT, trow, tcol, cs, k, brightness, 0.5f, 0.0f, 0.0f);
			k = addQuadOnBlockFace(&e->pos, CF_RIGHT, trow, tcol, cs, k, brightness, -0.5f, 0.0f, 0.0f);
			k = addQuadOnBlockFace(&e->pos, CF_FRONT, trow, tcol, cs, k, brightness, 0.0f, 0.0f, -0.5f);
			k = addQuadOnBlockFace(&e->pos, CF_BACK, trow, tcol, cs, k, brightness, 0.0f, 0.0f, 0.5f);
		} else if (e->type == Entity::DOOR_X || e->type == Entity::DOOR_Z_OPEN) {
			trow = 1;
			k = addQuadOnBlockFace(&e->pos, CF_LEFT, trow, tcol, cs, k, brightness, 0.0f, 0.0f, 0.0f);
			k = addQuadOnBlockFace(&e->pos, CF_RIGHT, trow, tcol, cs, k, brightness, -1.0f, 0.0f, 0.0f);
			trow = 0;
			k = addQuadOnBlockFace(&e->pos, CF_LEFT, trow, tcol, cs, k, brightness, 0.0f, 1.0f, 0.0f);
			k = addQuadOnBlockFace(&e->pos, CF_RIGHT, trow, tcol, cs, k, brightness, -1.0f, 1.0f, 0.0f);
		} else if (e->type == Entity::DOOR_Z || e->type == Entity::DOOR_X_OPEN) {
			trow = 1;
			k = addQuadOnBlockFace(&e->pos, CF_FRONT, trow, tcol, cs, k, brightness, 0.0f, 0.0f, -1.0f);
			k = addQuadOnBlockFace(&e->pos, CF_BACK, trow, tcol, cs, k, brightness, 0.0f, 0.0f, 0.0f);
			trow = 0;
			k = addQuadOnBlockFace(&e->pos, CF_FRONT, trow, tcol, cs, k, brightness, 0.0f, 1.0f, -1.0f);
			k = addQuadOnBlockFace(&e->pos, CF_BACK, trow, tcol, cs, k, brightness, 0.0f, 1.0f, 0.0f);
		} else if (e->type == Entity::GLASS) {
			brightness = t->isBlockAbove(e->pos.x, e->pos.y, e->pos.z) ? 0.5f : 1.0f * ChunkMesh::getDaylightFactor();
			k = addQuadOnBlockFace(&e->pos, CF_LEFT, trow, tcol, cs, k, brightness);
			k = addQuadOnBlockFace(&e->pos, CF_RIGHT, trow, tcol, cs, k, brightness);
			k = addQuadOnBlockFace(&e->pos, CF_TOP, trow, tcol, cs, k, brightness);
			k = addQuadOnBlockFace(&e->pos, CF_BOTTOM, trow, tcol, cs, k, brightness);
			k = addQuadOnBlockFace(&e->pos, CF_FRONT, trow, tcol, cs, k, brightness);
			k = addQuadOnBlockFace(&e->pos, CF_BACK, trow, tcol, cs, k, brightness);
		} else if(e->type == Entity::RAIL) {
			railManager->determineRowColForPos(e->pos, &trow, &tcol);
			k = addQuadOnBlockFace(&e->pos, CF_TOP, trow, tcol, cs, k, brightness, 0.0f, 0.1f, 0.0f);
		} else { // ladders, torch on side
			k = addQuadOnBlockFace(&e->pos, e->cface, trow, tcol, cs, k, brightness);
		}
	}

	assert(k == l);

	if (!mesh) {
		mesh = new Mesh();
	}
	mesh->setVertices(cs, l);
	SAFE_DELETE_ARRAY(cs);

	empty = false;
}

}
