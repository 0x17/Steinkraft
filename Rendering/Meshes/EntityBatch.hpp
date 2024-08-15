// EntityBatch.hpp

#ifndef ENTITY_BATCH_HPP
#define ENTITY_BATCH_HPP

#include "../../Framework/Mesh.hpp"
#include "../../Terrain.hpp"
#include "CubeVertices.hpp"

#include <list>

namespace as {

class RailManager;

class EntityBatch {
public:
	EntityBatch(Terrain *t, RailManager *_rm, int minX, int maxX, int minZ, int maxZ);
	virtual ~EntityBatch();

	void update();
	void render();

	static int addQuadOnBlockFace(BlockPos *pos, CubeFace cface, int trow, int tcol,
						   float *coords, int startIndex, float brightness,
						   float offX = 0.0f, float offY = 0.0f, float offZ = 0.0f);
private:
	bool empty;
	Terrain *t;
	RailManager *railManager;
	int minX, maxX, minZ, maxZ;
	Mesh *mesh;
};

inline void EntityBatch::render() {
	if (!empty) {
		mesh->render();
	}
}

}

#endif // ENTITY_BATCH_HPP
