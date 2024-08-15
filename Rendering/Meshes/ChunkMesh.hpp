// ChunkMesh.hpp

#ifndef CHUNKMESH_HPP_
#define CHUNKMESH_HPP_

#define INDEXED_CHK_MESH 0

#include <list>
#include <vector>

#include "../../Constants.h"
#include "../../Terrain.hpp"

#include "../../Framework/Utilities.hpp"
#include "../../Framework/Math/Frustum.hpp"
#include "../../Framework/Mesh.hpp"
#include "../../Framework/IndexedMesh.hpp"

#include "BlockMesh.hpp"
#include "CubeVertices.hpp"

namespace as {

class PosTexVertexCol {
public:
	float x, y, z, u, v, r, g, b;

	PosTexVertexCol() {}

	PosTexVertexCol(float _x, float _y, float _z, // pos
		float _u, float _v, // tex coord
		float _r, float _g, float _b) // color
		:	x(_x), y(_y), z(_z),
		u(_u), v(_v),
		r(_r), g(_g), b(_b)
	{}
};

#if INDEXED_CHK_MESH
typedef IndexedMesh MeshType;
#else
typedef Mesh MeshType;
#endif

class ChunkMesh {
public:
	explicit ChunkMesh(Terrain *t, int minX = 0, int maxX = Terrain::MAX_X, int minZ = 0, int maxZ = Terrain::MAX_Z);
	virtual ~ChunkMesh();

	void update(int posY);

	BoundingBox *getBoundingBox();
	void renderBoundingBox() const;

	int getMinX() const;
	int getMaxX() const;
	int getMinZ() const;
	int getMaxZ() const;
	
	void render(int camY);
	
	enum Consts {
		CHK_SUBMESH_HEIGHT	= Terrain::CHUNK_SIZE,
		NUM_SUBMESHES		= Terrain::MAX_Y / CHK_SUBMESH_HEIGHT
	};
	
	static bool updateDaylightFactor();
	static float getDaylightFactor();
	
	static void reset();

private:
	void setupBuffers(int index);
	void processBlock(int x, int y, int z);
	void addFence(int x, int y, int z);
	void addBlock(VisibleFaces vfaces, int x, int y, int z, int texRow, int texCol);
	void addBlock(VisibleFaces vfaces, int x, int y, int z, TexCoordRect *tcr);
	void genFace(bool bottom, bool side, bool onTop);
	void genVx(float *verts, const uint offset, const float brightness);

	void setBrightnessMacro(int x, int y, int z, float &brightness) const;
	void frontBackMacro(float &brightness) const;
	void leftRightMacro(float &brightness) const;
	void bottomMacro(float &brightness) const;
	
	void pushCoords(PosTexVertexCol *vx);
	
	Terrain *terrain;	

	std::list<PosTexVertexCol> vertices;
	PosTexVertexCol curVertices[UNIQUE_VERTICES_PER_QUAD];

	int minX, maxX, minZ, maxZ;

	BoundingBox bbox;

	MeshType *meshes[NUM_SUBMESHES];
	ticks_t lastMeshInit;
	
	int curIndex;
	
	static float daylightFactor;
	static ticks_t lastUpdate, startTicks;
};

inline int ChunkMesh::getMinX() const { return minX; }
inline int ChunkMesh::getMaxX() const { return maxX; }
inline int ChunkMesh::getMinZ() const { return minZ; }
inline int ChunkMesh::getMaxZ() const { return maxZ; }

inline BoundingBox *ChunkMesh::getBoundingBox() {
	return &bbox;
}

} /* namespace as */
#endif /* CHUNKMESH_HPP_ */
