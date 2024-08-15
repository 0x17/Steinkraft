// ChunkMesh.cpp



#include "../../Framework/Utilities.hpp"

#include "ChunkMesh.hpp"
#include "CubeVertices.hpp"

#include "../LandscapeRenderer.hpp"
#include "../HudRenderer.hpp"

namespace as {

const float MAX_LIGHT_DIST		= 2.0f;

const float FAKE_SHADOW_BNESS	= 0.5f;

const float FRONT_BACK_DIM		= 0.4f;
const float LEFT_RIGHT_DIM		= 0.2f;
const float BOTTOM_DIM			= 0.5f;

bool		keepMeshes			= false;

static float vxBuf[COMPONENTS_PER_VERTEX * VERTICES_PER_QUAD * FACES_PER_BOX * Terrain::CHUNK_SIZE * Terrain::CHUNK_SIZE * Terrain::CHUNK_SIZE];
#if INDEXED_CHK_MESH
    static ushort ixBuf[VERTICES_PER_QUAD * FACES_PER_BOX * Terrain::CHUNK_SIZE * Terrain::CHUNK_SIZE * Terrain::CHUNK_SIZE];
static int curIxIndex;
#endif

float ChunkMesh::daylightFactor = 1.0f;
ticks_t ChunkMesh::lastUpdate = 0;
ticks_t ChunkMesh::startTicks = getTicks();
	
void ChunkMesh::reset() {
	daylightFactor = 1.0f;
	lastUpdate = 0;
	startTicks = getTicks();
}
	
float ChunkMesh::getDaylightFactor() {
	return daylightFactor;
}
	
bool ChunkMesh::updateDaylightFactor() {
	const ticks_t DAYLIGHT_UPDATE_TICKS = 5000;
	
	if(noNight) return false;
	
	if(getTicks() - lastUpdate > DAYLIGHT_UPDATE_TICKS) {
		float factor = cosf((float)(getTicks() - startTicks) / (60000.0f * 4.0f) * PI) * 0.6f + 0.7f;
		if(factor < 0.15f) factor = 0.15f;
		if(factor > 1.0f) factor = 1.0f;
		
		lastUpdate = getTicks();
		
		if(ABS(factor - daylightFactor) >= 0.1f) {
			daylightFactor = factor;
			glClearColor(0.6289f * daylightFactor, 0.6953f * daylightFactor, 0.9f * daylightFactor, 1.0f);
			LandscapeRenderer::setupFog(daylightFactor);
			HudRenderer::getInstance()->updateTexPreview(-1);
			return true;
		}
	}
	
	return false;
}

ChunkMesh::ChunkMesh(Terrain *_t, int _minX, int _maxX, int _minZ, int _maxZ)
:		terrain(_t),

		minX(_minX),
		maxX(_maxX),
		minZ(_minZ),
		maxZ(_maxZ),

		bbox((float)minX, (float)maxX, 0.0f, (float)Terrain::MAX_Y, (float)minZ, (float)maxZ),
		lastMeshInit(0)
{	
	memset(meshes, 0, sizeof(MeshType *) * NUM_SUBMESHES);
	
	if(keepMeshes) {
		for(int i=0; i<NUM_SUBMESHES; i++) {
			meshes[i] = new MeshType();
			setupBuffers(i);
		}
	}
}

ChunkMesh::~ChunkMesh() {
	for(int i=0; i<NUM_SUBMESHES; i++) {
		SAFE_DELETE(meshes[i]);
	}
}

void ChunkMesh::setupBuffers(int index) {
	int minY = index * CHK_SUBMESH_HEIGHT;
	int maxY = (index + 1) * CHK_SUBMESH_HEIGHT;
	
	curIndex = 0;
#if INDEXED_CHK_MESH
	curIxIndex = 0;
#endif

	int i, j, k;
	for (i = minX; i < maxX; i++) {
		for (j = minY; j < maxY; j++) {
			for (k = minZ; k < maxZ; k++) {
				processBlock(i, j, k);
			}
		}
	}

#if INDEXED_CHK_MESH
	meshes[index]->setVertices(vxBuf, curIndex, ixBuf, curIxIndex);
#else
	meshes[index]->setVertices(vxBuf, curIndex);
#endif
}

void ChunkMesh::update(int posY) {
	if(posY == -1) {
		for (int i = 0; i < NUM_SUBMESHES; i++) {
			if(meshes[i])
				setupBuffers(i);
		}
	}
	else {
		int index = posY/CHK_SUBMESH_HEIGHT;
		
		if (index < 0 || index >= NUM_SUBMESHES)
			return;

		if(!meshes[index])
			meshes[index] = new MeshType();

		setupBuffers(index);

		int mod = posY % CHK_SUBMESH_HEIGHT;
		if(mod == 0 && index - 1 >= 0) {
			if (!meshes[index-1]) meshes[index-1] = new MeshType();
			setupBuffers(index-1);
		}
		else if(mod == CHK_SUBMESH_HEIGHT - 1 && index + 1 < NUM_SUBMESHES) {
			if (!meshes[index+1]) meshes[index+1] = new MeshType();
			setupBuffers(index+1);
		}
	}
	
}

//===============================================================================
// Fence geometry
//===============================================================================
static float pillarCoords[] = {
	// front
	0.3f, 0.0f, 0.7f,	0.0f, 0.0f, FRONT_BACK_DIM,	 // 0
	0.7f, 0.0f, 0.7f,	1.0f, 0.0f, FRONT_BACK_DIM,  // 1
	0.7f, 1.0f, 0.7f,	1.0f, 1.0f, FRONT_BACK_DIM,  // 2
	0.3f, 1.0f, 0.7f,	0.0f, 1.0f, FRONT_BACK_DIM,  // 3
	
	// back
	0.7f, 0.0f, 0.3f,	0.0f, 0.0f, FRONT_BACK_DIM,  // 4
	0.3f, 0.0f, 0.3f,	1.0f, 0.0f, FRONT_BACK_DIM,  // 5
	0.3f, 1.0f, 0.3f,	1.0f, 1.0f, FRONT_BACK_DIM,  // 6
	0.7f, 1.0f, 0.3f,	0.0f, 1.0f, FRONT_BACK_DIM,  // 7
	
	// left
	0.3f, 0.0f, 0.3f,	0.0f, 0.0f, LEFT_RIGHT_DIM, // 8
	0.3f, 0.0f, 0.7f,	1.0f, 0.0f,  LEFT_RIGHT_DIM, // 9
	0.3f, 1.0f, 0.7f,	1.0f, 1.0f,  LEFT_RIGHT_DIM, // 10
	0.3f, 1.0f, 0.3f,	0.0f, 1.0f,  LEFT_RIGHT_DIM, // 11
	
	// right
	0.7f, 0.0f, 0.7f,	0.0f, 0.0f, LEFT_RIGHT_DIM,  // 12
	0.7f, 0.0f, 0.3f,	1.0f, 0.0f, LEFT_RIGHT_DIM,  // 13
	0.7f, 1.0f, 0.3f,	1.0f, 1.0f, LEFT_RIGHT_DIM,  // 14
	0.7f, 1.0f, 0.7f,	0.0f, 1.0f, LEFT_RIGHT_DIM,  // 15
	
	// top
	0.3f, 1.0f, 0.7f,	0.0f, 0.0f, 0.0f,	 // 16
	0.7f, 1.0f, 0.7f,	1.0f, 0.0f, 0.0f,	 // 17
	0.7f, 1.0f, 0.3f,	1.0f, 1.0f,	0.0f, // 18
	0.3f, 1.0f, 0.3f,	0.0f, 1.0f,	0.0f, // 19
	
	// bottom
	0.3f, 0.0f, 0.3f,	0.0f, 0.0f,	BOTTOM_DIM, // 20
	0.7f, 0.0f, 0.3f,	1.0f, 0.0f,	BOTTOM_DIM, // 21
	0.7f, 0.0f, 0.7f,	1.0f, 1.0f,	BOTTOM_DIM, // 22
	0.3f, 0.0f, 0.7f,	0.0f, 1.0f,	BOTTOM_DIM, // 23
};

static int pillarIndices[6*6] = {
	// front quad
	0, 1, 2,
	2, 3, 0,
	
	// back quad
	4, 5, 6,
	6, 7, 4,
	
	// left quad
	8, 9, 10,
	10, 11, 8,
	
	// right quad
	12, 13, 14,
	14, 15, 12,
	
	// top quad
	16, 17, 18,
	18, 19, 16,
	
	// bottom quad
	20, 21, 22,
	22, 23, 20,
};
	
static float toLeftRailCoords[] = {
	// front
	0.0f, 0.5f, 0.7f,		0.0f, 0.0f, FRONT_BACK_DIM, // 0
	0.3f, 0.5f, 0.7f,		1.0f, 0.0f, FRONT_BACK_DIM, // 1
	0.3f, 0.75f, 0.7f,		1.0f, 1.0f, FRONT_BACK_DIM, // 2
	0.0f, 0.75f, 0.7f,		0.0f, 1.0f, FRONT_BACK_DIM, // 3
	
	// back
	0.3f, 0.5f, 0.3f,		0.0f, 0.0f, FRONT_BACK_DIM, // 4
	0.0f, 0.5f, 0.3f,		1.0f, 0.0f, FRONT_BACK_DIM, // 5
	0.0f, 0.75f, 0.3f,		1.0f, 1.0f, FRONT_BACK_DIM, // 6
	0.3f, 0.75f, 0.3f,		0.0f, 1.0f, FRONT_BACK_DIM, // 7
	
	// top
	0.0f, 0.75f, 0.7f,		0.0f, 0.0f, 0.0f, // 8
	0.3f, 0.75f, 0.7f,		1.0f, 0.0f, 0.0f,  // 9
	0.3f, 0.75f, 0.3f,		1.0f, 1.0f,0.0f, // 10
	0.0f, 0.75f, 0.3f,		0.0f, 1.0f, 0.0f, // 11
	
	// bottom
	0.0f, 0.5f, 0.3f,		0.0f, 0.0f, BOTTOM_DIM, // 12
	0.3f, 0.5f, 0.3f,		1.0f, 0.0f, BOTTOM_DIM, // 13
	0.3f, 0.5f, 0.7f,		1.0f, 1.0f, BOTTOM_DIM, // 14
	0.0f, 0.5f, 0.7f,		0.0f, 1.0f, BOTTOM_DIM, // 15
};
	
static float toRightRailCoords[] = {
	// front
	0.7f, 0.5f, 0.7f,		0.0f, 0.0f, FRONT_BACK_DIM, // 0
	1.0f, 0.5f, 0.7f,		1.0f, 0.0f, FRONT_BACK_DIM, // 1
	1.0f, 0.75f, 0.7f,		1.0f, 1.0f, FRONT_BACK_DIM, // 2
	0.7f, 0.75f, 0.7f,		0.0f, 1.0f, FRONT_BACK_DIM, // 3
	
	// back
	1.0f, 0.5f, 0.3f,		0.0f, 0.0f, FRONT_BACK_DIM, // 4
	0.7f, 0.5f, 0.3f,		1.0f, 0.0f, FRONT_BACK_DIM, // 5
	0.7f, 0.75f, 0.3f,		1.0f, 1.0f, FRONT_BACK_DIM, // 6
	1.0f, 0.75f, 0.3f,		0.0f, 1.0f, FRONT_BACK_DIM, // 7
	
	// top
	0.7f, 0.75f, 0.7f,		0.0f, 0.0f, 0.0f, // 8
	1.0f, 0.75f, 0.7f,		1.0f, 0.0f, 0.0f, // 9
	1.0f, 0.75f, 0.3f,		1.0f, 1.0f, 0.0f, // 10
	0.7f, 0.75f, 0.3f,		0.0f, 1.0f, 0.0f, // 11
	
	// bottom
	0.7f, 0.5f, 0.3f,		0.0f, 0.0f, BOTTOM_DIM, // 12
	1.0f, 0.5f, 0.3f,		1.0f, 0.0f, BOTTOM_DIM, // 13
	1.0f, 0.5f, 0.7f,		1.0f, 1.0f, BOTTOM_DIM, // 14
	0.7f, 0.5f, 0.7f,		0.0f, 1.0f, BOTTOM_DIM, // 15
};

static float toBackRailCoords[] = {
	// left
	0.3f, 0.5f, 0.0f,		0.0f, 0.0f, LEFT_RIGHT_DIM, // 0
	0.3f, 0.5f, 0.3f,		1.0f, 0.0f, LEFT_RIGHT_DIM, // 1
	0.3f, 0.75f, 0.3f,		1.0f, 1.0f, LEFT_RIGHT_DIM, // 2
	0.3f, 0.75f, 0.0f,		0.0f, 1.0f, LEFT_RIGHT_DIM, // 3
	
	// right
	0.7f, 0.5f, 0.3f,		0.0f, 0.0f, LEFT_RIGHT_DIM, // 4
	0.7f, 0.5f, 0.0f,		1.0f, 0.0f, LEFT_RIGHT_DIM, // 5
	0.7f, 0.75f, 0.0f,		1.0f, 1.0f, LEFT_RIGHT_DIM, // 6
	0.7f, 0.75f, 0.3f,		0.0f, 1.0f, LEFT_RIGHT_DIM, // 7
	
	// top
	0.3f, 0.75f, 0.3f,		0.0f, 0.0f, 0.0f, // 8
	0.7f, 0.75f, 0.3f,		1.0f, 0.0f, 0.0f, // 9
	0.7f, 0.75f, 0.0f,		1.0f, 1.0f, 0.0f, // 10
	0.3f, 0.75f, 0.0f,		0.0f, 1.0f, 0.0f, // 11
	
	// bottom
	0.3f, 0.5f, 0.0f,		0.0f, 0.0f, BOTTOM_DIM, // 12
	0.7f, 0.5f, 0.0f,		1.0f, 0.0f, BOTTOM_DIM,  // 13
	0.7f, 0.5f, 0.3f,		1.0f, 1.0f, BOTTOM_DIM,  // 14
	0.3f, 0.5f, 0.3f,		0.0f, 1.0f, BOTTOM_DIM,  // 15
};

static float toFrontRailCoords[] = {
	// left
	0.3f, 0.5f, 0.7f,		0.0f, 0.0f, LEFT_RIGHT_DIM, // 0
	0.3f, 0.5f, 1.0f,		1.0f, 0.0f, LEFT_RIGHT_DIM, // 1
	0.3f, 0.75f, 1.0f,		1.0f, 1.0f, LEFT_RIGHT_DIM, // 2
	0.3f, 0.75f, 0.7f,		0.0f, 1.0f, LEFT_RIGHT_DIM, // 3
	
	// right
	0.7f, 0.5f, 1.0f,		0.0f, 0.0f, LEFT_RIGHT_DIM, // 4
	0.7f, 0.5f, 0.7f,		1.0f, 0.0f,  LEFT_RIGHT_DIM, // 5
	0.7f, 0.75f, 0.7f,		1.0f, 1.0f, LEFT_RIGHT_DIM, // 6
	0.7f, 0.75f, 1.0f,		0.0f, 1.0f, LEFT_RIGHT_DIM, // 7
	
	// top
	0.3f, 0.75f, 1.0f,		0.0f, 0.0f, 0.0f, // 8
	0.7f, 0.75f, 1.0f,		1.0f, 0.0f,0.0f, // 9
	0.7f, 0.75f, 0.7f,		1.0f, 1.0f,0.0f, // 10
	0.3f, 0.75f, 0.7f,		0.0f, 1.0f,0.0f, // 11
	
	// bottom
	0.3f, 0.5f, 0.7f,		0.0f, 0.0f, BOTTOM_DIM, // 12
	0.7f, 0.5f, 0.7f,		1.0f, 0.0f, BOTTOM_DIM, // 13
	0.7f, 0.5f, 1.0f,		1.0f, 1.0f, BOTTOM_DIM, // 14
	0.3f, 0.5f, 1.0f,		0.0f, 1.0f, BOTTOM_DIM, // 15
};

static int railIndices[] = {
	0, 1, 2,
	2, 3, 0,
	
	4, 5, 6,
	6, 7, 4,
	
	8, 9, 10,
	10, 11, 8,
	
	12, 13, 14,
	14, 15, 12,
};

void ChunkMesh::addFence(int x, int y, int z) {
	const int fix = Terrain::FENCE_TEX_INDEX + 1;
	bool isFenceLeft = terrain->get(x-1, y, z) == fix;
	bool isFenceRight = terrain->get(x+1, y, z) == fix;
	bool isFenceBack = terrain->get(x, y, z-1) == fix;
	bool isFenceFront = terrain->get(x, y, z+1) == fix;
	
	const int trow = 1;
	const int tcol = 1;
	TexCoordRect tcr(tcol * TEX_COORD_FACTOR, (tcol+1)*TEX_COORD_FACTOR, trow * TEX_COORD_FACTOR, (trow+1)*TEX_COORD_FACTOR);
	
	PosTexVertexCol vx;
	float brightness = daylightFactor * ((terrain->isBlockAbove(x, y, z)) ? 0.5f : 1.0f);
	
	// Add fence pillar
	for(int i=0; i<6*6; i++) { // for each vertex
		int ix = pillarIndices[i];
		vx.x = pillarCoords[ix*6+0] + x;
		vx.y = pillarCoords[ix*6+1] + y;
		vx.z = pillarCoords[ix*6+2] + z;
		vx.u = pillarCoords[ix*6+3] * (tcr.maxU - tcr.minU) + tcr.minU;
		vx.v = pillarCoords[ix*6+4] * (tcr.maxV - tcr.minV) + tcr.minV;
		vx.r = vx.g = vx.b = brightness * (1- pillarCoords[ix*6+5]);
		pushCoords(&vx);
	}
	
	// Add fence rails connecting pillars
	if(isFenceLeft) {
		for(int i=0; i<4*6; i++) {
			int ix = railIndices[i];
			vx.x = toLeftRailCoords[ix*6+0] + x;
			vx.y = toLeftRailCoords[ix*6+1] + y;
			vx.z = toLeftRailCoords[ix*6+2] + z;
			vx.u = toLeftRailCoords[ix*6+3] * (tcr.maxU - tcr.minU) + tcr.minU;
			vx.v = toLeftRailCoords[ix*6+4] * (tcr.maxV - tcr.minV) + tcr.minV;
			vx.r = vx.g = vx.b = brightness * (1- pillarCoords[ix*6+5]);
			pushCoords(&vx);
		}
	}
	if(isFenceRight) {
		for(int i=0; i<4*6; i++) {
			int ix = railIndices[i];
			vx.x = toRightRailCoords[ix*6+0] + x;
			vx.y = toRightRailCoords[ix*6+1] + y;
			vx.z = toRightRailCoords[ix*6+2] + z;
			vx.u = toRightRailCoords[ix*6+3] * (tcr.maxU - tcr.minU) + tcr.minU;
			vx.v = toRightRailCoords[ix*6+4] * (tcr.maxV - tcr.minV) + tcr.minV;
			vx.r = vx.g = vx.b = brightness * (1- pillarCoords[ix*6+5]);
			pushCoords(&vx);
		}
	}
	if(isFenceBack) {
		for(int i=0; i<4*6; i++) {
			int ix = railIndices[i];
			vx.x = toBackRailCoords[ix*6+0] + x;
			vx.y = toBackRailCoords[ix*6+1] + y;
			vx.z = toBackRailCoords[ix*6+2] + z;
			vx.u = toBackRailCoords[ix*6+3] * (tcr.maxU - tcr.minU) + tcr.minU;
			vx.v = toBackRailCoords[ix*6+4] * (tcr.maxV - tcr.minV) + tcr.minV;
			vx.r = vx.g = vx.b = brightness * (1- pillarCoords[ix*6+5]);
			pushCoords(&vx);
		}
	}
	if(isFenceFront) {
		for(int i=0; i<4*6; i++) {
			int ix = railIndices[i];
			vx.x = toFrontRailCoords[ix*6+0] + x;
			vx.y = toFrontRailCoords[ix*6+1] + y;
			vx.z = toFrontRailCoords[ix*6+2] + z;
			vx.u = toFrontRailCoords[ix*6+3] * (tcr.maxU - tcr.minU) + tcr.minU;
			vx.v = toFrontRailCoords[ix*6+4] * (tcr.maxV - tcr.minV) + tcr.minV;
			vx.r = vx.g = vx.b = brightness * (1- pillarCoords[ix*6+5]);
			pushCoords(&vx);
		}
	}
}

//===============================================================================

void ChunkMesh::processBlock(int x, int y, int z) {
	int val = terrain->get(x, y, z) - 1;

	if (val + 1 > 0 && !isInvisible(val + 1)) {
		
		if(val == Terrain::FENCE_TEX_INDEX) {
			addFence(x, y, z);
			return;
		}
		
		VisibleFaces vfaces = terrain->determineVisibleFaces(x, y, z);
		if (!vfaces.allInvisible()) {
			addBlock(vfaces, x, y, z, val / NUM_TEX_PER_ROW, val % NUM_TEX_PER_ROW);
		}
	}

}

void ChunkMesh::addBlock(VisibleFaces vfaces, int x, int y, int z, int texRow, int texCol) {
	TexCoordRect tcr(TEX_COORD_FACTOR * texCol, TEX_COORD_FACTOR * (texCol + 1),
					 TEX_COORD_FACTOR * texRow, TEX_COORD_FACTOR * (texRow + 1));
	addBlock(vfaces, x, y, z, &tcr);
}

inline void ChunkMesh::setBrightnessMacro(int x, int y, int z, float &brightness) const {
	brightness = terrain->isBlockAbove(x, y, z) ? FAKE_SHADOW_BNESS : 1.0f;
}

inline void ChunkMesh::frontBackMacro(float &brightness) const {
	brightness = brightness * (1.0f - FRONT_BACK_DIM);
	//if(brightness < 0.0f) brightness = 0.0f;
}

inline void ChunkMesh::leftRightMacro(float &brightness) const {
	brightness = brightness * (1.0f - LEFT_RIGHT_DIM);
	//if(brightness < 0.0f) brightness = 0.0f;
}

inline void ChunkMesh::bottomMacro(float &brightness) const {
	brightness = brightness * (1.0f - BOTTOM_DIM);
	//if(brightness < 0.0f) brightness = 0.0f;
}

void ChunkMesh::addBlock(VisibleFaces vfaces, int x, int y, int z, TexCoordRect *tcr) {
	static float verts[TRANS_POS_NORM_VX_LEN];
	genTranslatedPosTexNormalColVerticesFast((float)x, (float)y, (float)z, tcr, verts);

	// used for fake shadows (block on top of block adj to the face).
	float brightness;

	if (vfaces.front) {
		setBrightnessMacro(x, y, z + 1, brightness);
		frontBackMacro(brightness);		
		genVx(verts, 0*COMPONENTS_PER_VERTEX_NOCOL, brightness);
		genFace(false, true, vfaces.top);
	}
	if (vfaces.back) {
		setBrightnessMacro(x, y, z - 1, brightness);
		frontBackMacro(brightness);
		genVx(verts, 4*COMPONENTS_PER_VERTEX_NOCOL, brightness);
		genFace(false, true, vfaces.top);
	}
	if (vfaces.left) {
		setBrightnessMacro(x - 1, y, z, brightness);
		leftRightMacro(brightness);
		genVx(verts, 8*COMPONENTS_PER_VERTEX_NOCOL, brightness);
		genFace(false, true, vfaces.top);
	}
	if (vfaces.right) {
		setBrightnessMacro(x + 1, y, z, brightness);
		leftRightMacro(brightness);
		genVx(verts, 12*COMPONENTS_PER_VERTEX_NOCOL, brightness);
		genFace(false, true, vfaces.top);
	}
	if (vfaces.bottom) {
		setBrightnessMacro(x, y - 1, z, brightness);
		bottomMacro(brightness);
		genVx(verts, 16*COMPONENTS_PER_VERTEX_NOCOL, brightness);
		genFace(true, false, vfaces.top);
	}
	if (vfaces.top) {
		setBrightnessMacro(x, y + 1, z, brightness);
		genVx(verts, 20*COMPONENTS_PER_VERTEX_NOCOL, brightness);
		genFace(false, false, vfaces.top);
	}
}

void ChunkMesh::genFace(bool bottom, bool side, bool onTop) {
	const float sideTexMinU = 0;
	const float sideTexMaxU = TEX_COORD_FACTOR;
	const float sideTexMinV = TEX_COORD_FACTOR;
	const float sideTexMaxV = TEX_COORD_FACTOR * 2;

	const float dirtTexMinU = TEX_COORD_FACTOR;
	const float dirtTexMaxU = TEX_COORD_FACTOR * 2;
	const float dirtTexMinV = 0;
	const float dirtTexMaxV = TEX_COORD_FACTOR;

	// first tex has side grass tex on sides!
	bool specialBlock = (curVertices[0].u == 0 && curVertices[0].v == 0);

	if (specialBlock && (side || bottom)) {
		if (onTop && side) {
			curVertices[0].u = sideTexMinU;
			curVertices[0].v = sideTexMinV;
			curVertices[1].u = sideTexMinU;
			curVertices[1].v = sideTexMaxV;
			curVertices[2].u = sideTexMaxU;
			curVertices[2].v = sideTexMaxV;
			curVertices[3].u = sideTexMaxU;
			curVertices[3].v = sideTexMinV;
		} else {
			curVertices[0].u = dirtTexMinU;
			curVertices[0].v = dirtTexMinV;
			curVertices[1].u = dirtTexMinU;
			curVertices[1].v = dirtTexMaxV;
			curVertices[2].u = dirtTexMaxU;
			curVertices[2].v = dirtTexMaxV;
			curVertices[3].u = dirtTexMaxU;
			curVertices[3].v = dirtTexMinV;
		}
	}
	
#if INDEXED_CHK_MESH
	int indexOffset = curIndex / COMPONENTS_PER_VERTEX;
	ixBuf[curIxIndex++] = indexOffset+0;
	ixBuf[curIxIndex++] = indexOffset+1;
	ixBuf[curIxIndex++] = indexOffset+2;

	ixBuf[curIxIndex++] = indexOffset+2;
	ixBuf[curIxIndex++] = indexOffset+3;
	ixBuf[curIxIndex++] = indexOffset+0;

	pushCoords(&curVertices[0]);
	pushCoords(&curVertices[1]);
	pushCoords(&curVertices[2]);
	pushCoords(&curVertices[3]);
#else
	pushCoords(&curVertices[0]);
	pushCoords(&curVertices[1]);
	pushCoords(&curVertices[2]);
	
	pushCoords(&curVertices[2]);
	pushCoords(&curVertices[3]);
	pushCoords(&curVertices[0]);
#endif
}

void ChunkMesh::pushCoords(PosTexVertexCol *vx) {
	vxBuf[curIndex++] = vx->x;
	vxBuf[curIndex++] = vx->y;
	vxBuf[curIndex++] = vx->z;
	
	vxBuf[curIndex++] = vx->u;
	vxBuf[curIndex++] = vx->v;
	
	vxBuf[curIndex++] = vx->r;
	vxBuf[curIndex++] = vx->g;
	vxBuf[curIndex++] = vx->b;
	vxBuf[curIndex++] = 1.0f;
}

void ChunkMesh::genVx(float *verts, const uint offset, const float brightness) {
	static float bness;
	static float x, y, z;
	static float ldist;
	
	int j = 0;

	// 11 components per vertex, 4 vertices per face
	for (ulong i = offset; i < offset + UNIQUE_VERTICES_PER_QUAD*COMPONENTS_PER_VERTEX_NOCOL; i += COMPONENTS_PER_VERTEX_NOCOL) {
	
		// the more adjacent blocks it has the darker a block gets		
		bness = brightness * daylightFactor;

		x = verts[i];
		y = verts[i+1];
		z = verts[i+2];

		if (terrain->hasEntities()) {
			ldist = terrain->distToNearestLight(x, y, z);
			if (ldist <= MAX_LIGHT_DIST) {
				float b = (1.0f - ldist*ldist / MAX_LIGHT_DIST * 0.25f);
				if(b > bness) bness = b;
			}
		}

		curVertices[j++] = PosTexVertexCol(x, y, z, // position coordinates
			verts[i+3], verts[i+4], // texture coordinates u,v
			bness, bness, bness); // color r,g,b
	}
}

void ChunkMesh::renderBoundingBox() const {
	BlockMesh bm(0, 0);
	glPushMatrix();
	glTranslatef((float)minX, 0.0f, (float)minZ);
	glScalef((float)Terrain::CHUNK_SIZE, (float)Terrain::CHUNK_SIZE, (float)Terrain::CHUNK_SIZE);
	bm.render();
	glPopMatrix();
}

const ticks_t TICKS_BETWEEN_MESH_INITS = 1000;

void ChunkMesh::render(int camY) {
	for(int i = 0; i < NUM_SUBMESHES; i++) {
		MeshType *mesh = meshes[i];

		if(mesh) {
			mesh->render();			
		} else {
			if(getTicks() - lastMeshInit > TICKS_BETWEEN_MESH_INITS) {
				int j = meshes[camY / CHK_SUBMESH_HEIGHT] == NULL ? camY / CHK_SUBMESH_HEIGHT : i;
				meshes[j] = new MeshType();
				setupBuffers(j);
				meshes[j]->render();
				lastMeshInit = getTicks();
			}
		}
	}
}


} /* namespace as */
