// LandscapeRenderer.h

#ifndef LANDSCAPERENDERER_HPP
#define LANDSCAPERENDERER_HPP

#include <list>
#include <vector>

#include "../Framework/Camera.hpp"
#include "../Framework/Texture.hpp"
#include "../Framework/Observable.hpp"

#include "../Managers/AnimalManager.hpp"
#include "../Terrain.hpp"

#include "Voxelrenderer/IVoxelRenderer.hpp"
#include "Meshes/BlockMesh.hpp"
#include "BlockExplAnim.hpp"

namespace as {

class RailManager;

class LandscapeRenderer : public Observer<BlockPos> {
public:
	LandscapeRenderer(Terrain *terrain, RailManager *rm, Camera *cam, AnimalManager *animalManager);
	virtual ~LandscapeRenderer();

	virtual void update(BlockPos *bposChanged);
	void render(bool highlightSelBlock);

	BlockPos *getSelectedBlock();
	CubeFace getSelectedFace();

	void addExplAt(int x, int y, int z, int texId, int blockBelowY);

	void updateSelectedBlock(std::list<BlockPos> *blocksNearCam, int x, int y);
	
	static void setupFog(float factor = 1.0f);

private:
	void highlightSelectedBlock();
	std::vector<float> *genTrianglesOfBlocksAtPositions(std::list<BlockPos> *blocksNear, Ray *ray);
	void highlightFace(CubeFace visFace);
	CubeFace determineSelectedFace();
	
	IVoxelRenderer *voxelRenderer;
	CubeFace selectedFace;
	BlockPos *selectedBlock;
	Terrain *terrain;
	Camera *cam;
	BlockMesh *highlightBlock;

	BlockExplAnimManager *explAnimMgr;

	Vec3 isect;
};

inline BlockPos *LandscapeRenderer::getSelectedBlock() {
	return selectedBlock;
}

inline CubeFace LandscapeRenderer::getSelectedFace() {
	return selectedFace;
}

inline void LandscapeRenderer::addExplAt(int x, int y, int z, int texId, int blockBelowY) {
	explAnimMgr->addBlockExplAnim(x, y, z, texId, blockBelowY);
}

} /* namespace as */
#endif /* LANDSCAPERENDERER_HPP */
