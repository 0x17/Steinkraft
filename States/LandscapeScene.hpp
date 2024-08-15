// LandscapeScene.hpp

#ifndef LANDSCAPESCENE_HPP
#define LANDSCAPESCENE_HPP

#include <list>
#include <string>

#include "../Framework/PGL.h"
#include "../Framework/State.hpp"
#include "../Framework/Camera.hpp"

#include "../Managers/TNTManager.hpp"

namespace as {
//===========================================================================
// Globals
//===========================================================================
extern int lastLeftJoystickTouch[2], lastRightJoystickTouch[2];

//===========================================================================
// Types
//===========================================================================
class Terrain;
class Movement;
class LandscapeRenderer;
class HudRenderer;
class LightSource;
class Vec3;
class BlockPos;
class Texture;
class StateManager;
class NetManager;
class InputMethod;
class Player;
class RailManager;

class LandscapeScene : public State {
public:
	LandscapeScene(int seed, StateManager *g, Terrain::TerrainSource tsource, bool mp = false, bool server = false);
	LandscapeScene(const char *filename, StateManager *g, bool mp = false, bool server = false);
	virtual ~LandscapeScene();
	virtual void persist();

	virtual void processKeyboardInput(bool *keys, SDLMod mod, ticks_t delta);
	virtual void processMouseInput(int dX, int dY, int wheel, MouseButtons *mb, ticks_t delta);

	virtual void processTouch(int tX, int tY, ticks_t delta);
	virtual void processNoTouch();

	virtual void draw(ticks_t delta);

	void doDigOrPut();	

	//! getters and setters
	void setSxSy(int _sx, int _sy);
	LandscapeRenderer *getRenderer();
	StateManager *getGame();

	//! bad bad public members instead of trivial getters/setters
	DATA_TYPE selectedTexture;
	bool highlightSelBlock;
	bool texSelOverlayActive, texSelOverlayFinished;
	bool digMode;
	
	Camera *getCam();

private:
	void putNewBlock( bool &selDoor, BlockPos * selectedBlock, int actualX, int actualY, int actualZ, CubeFace selectedFace );
	void digExistingBlock( BlockPos * selectedBlock, CubeFace selectedFace );

	void refreshBlocksNearCam();

	void commonInit(int seed, const char *filename, Terrain::TerrainSource tsource, bool mp, bool server);
	bool tryLoadPosFromFile(const char *filename);
	void savePosToFile(const char *filename);

	void saveWorld();
	bool isStandingEntity(int etype, bool isDoor, bool selDoor, CubeFace selectedFace);
	
	float calcDistToSelBlock();

	StateManager *g;

	InputMethod *im;

	Terrain *terrain;
	Movement *mvmt;
	LandscapeRenderer *landscapeRenderer;
	HudRenderer *hudRenderer;

	ticks_t lastBlockPlacementTicks, lastTexSwitch;

	Vec3 lastCamPos, lastCamNormal;
	std::list<BlockPos> blocksNearCam;
	bool nearBlocksDirty;

	LightSource *lsource;

	int sx, sy;

	int worldNum;

	int posArray[3];

	TNTManager *tntManager;
	AnimalManager *animalManager;
	NetManager *netManager;
	RailManager *railManager;

	bool crouching;
	
	Camera cam;
	
	bool touchWasReleased;
};

inline LandscapeRenderer* LandscapeScene::getRenderer() {
	return landscapeRenderer;
}

inline StateManager* LandscapeScene::getGame() {
	return g;
}

inline void LandscapeScene::setSxSy(int _sx, int _sy) {
	this->sx = _sx;
	this->sy = _sy;
}

inline Camera *LandscapeScene::getCam() {
	return &cam;
}

}

#endif /* LANDSCAPESCENE_HPP */
