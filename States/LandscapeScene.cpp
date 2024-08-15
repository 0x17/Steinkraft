// LandscapeScene.cpp



#include <cstdlib>
#include <ctime>
#include <cfloat>

#include <stdint.h>
#include <sys/types.h>

#include "../Constants.h"

#if IPHONE || MAC
#include <sys/sysctl.h>
#endif

#include "../Framework/Camera.hpp"
#include "../Framework/Utilities.hpp"
#include "../Framework/LightSource.hpp"
#include "../Framework/Texture.hpp"
#include "../Framework/Platforms/Desktop.hpp"

#include "../Rendering/HudRenderer.hpp"
#include "../Rendering/LandscapeRenderer.hpp"

#include "../Managers/NetManager.hpp"
#include "../Managers/RailManager.hpp"

#include "../Terrain.hpp"
#include "../Movement.hpp"
#include "../InputMethods.hpp"

#include "MenuState.hpp"
#include "LandscapeScene.hpp"

namespace as {
//===========================================================================
// Constants / Macros
//===========================================================================
const bool FLY_MODE = false;

const int	KMOVF = 15;
const int	KROTF = 10;

enum ToolTexIndices {
	HAND_TEX_INDEX = 128,
	SHOVEL_TEX_INDEX,
	PICKAXE_TEX_INDEX,
	AXE_TEX_INDEX
};

static char posFilename[BUF_LEN];

inline void DET_POS_FILE(const char *filename) {
	strcpy(posFilename, filename);
	strcat(posFilename, ".spawnpos");
}

//===========================================================================
// Globals
//===========================================================================

InputMethodType activeInputMethod = IM_RIPOFF_DPAD;
bool survival = false;

bool mouseWasReleased = false;

// Toggles
bool noNight = false;
bool noAnimals = false;
bool noSound = false;
bool classicTexture = false;
int activeTexMap = TEXMAP_CLASSIC;

//===========================================================================
// Methods
//===========================================================================
inline void LandscapeScene::saveWorld() {
#if LITE && IPHONE
	return;
#endif
	char saveFilename[BUF_LEN];
	std::sprintf(saveFilename, "World%d.dump", worldNum);
	terrain->saveTerrainToFile(saveFilename);
	terrain->saveEntitiesToFile(saveFilename);
	savePosToFile(saveFilename);
	animalManager->saveToFile(saveFilename);
	// TODO: Save survival mode stuff here too
}

LandscapeScene::LandscapeScene(const char *filename, StateManager *_g, bool mp, bool server)
:	g(_g),
	netManager(NULL),
	cam(FOV, Vec3(Terrain::MAX_X / 2.0f + 0.5f, Terrain::MAX_Y + 5, Terrain::MAX_Z / 2.0f + 0.5f), Vec3(0, 0, 1), Vec3(0, 1, 0))
{
	commonInit(0, filename, Terrain::TS_FILE, mp, server);
}

LandscapeScene::LandscapeScene(int seed, StateManager *_g, Terrain::TerrainSource tsource, bool mp, bool server)
:	g(_g),
	netManager(NULL),
	cam(FOV, Vec3(Terrain::MAX_X / 2.0f + 0.5f, Terrain::MAX_Y + 5, Terrain::MAX_Z / 2.0f + 0.5f), Vec3(0, 0, 1), Vec3(0, 1, 0))
{
	commonInit(seed, NULL, tsource, mp, server);
}

LandscapeScene::~LandscapeScene() {
#if !NO_NET
	if (!netManager || netManager->shouldSave())
		saveWorld();
	SAFE_DELETE(netManager);
#endif

	SAFE_DELETE(im);

	SAFE_DELETE(animalManager);
	SAFE_DELETE(tntManager);
	SAFE_DELETE(railManager);

	SAFE_DELETE(mvmt);
	SAFE_DELETE(terrain);
	SAFE_DELETE(hudRenderer);
	SAFE_DELETE(landscapeRenderer);
}

void LandscapeScene::persist() {
#if !NO_NET
	if(!netManager || netManager->shouldSave())
		saveWorld();
#endif
}

void LandscapeScene::commonInit(int seed, const char *filename, Terrain::TerrainSource tsource, bool mp, bool server) {
	cam.apply();

	posArray[0] = posArray[1] = posArray[2] = -1;
	sx = sy = worldNum = -1;

	lastCamPos.setTo(cam.getPosPtr());
	lastCamNormal.setTo(cam.getNormalPtr());

	texSelOverlayActive = texSelOverlayFinished = false;

	digMode = true;

	selectedTexture = (survival ? HAND_TEX_INDEX : 0);
	lastTexSwitch = lastBlockPlacementTicks = 0;

	if (!filename) {
		terrain = new Terrain(tsource, seed);
		worldNum = determineNextFreeSlot();
	} else {
		terrain = new Terrain(Terrain::TS_EMPTY, (int)std::time(NULL));
		terrain->loadTerrainFromFile(filename);
		terrain->loadEntitiesFromFile(filename);
		std::sscanf(filename, "World%d.dump", &worldNum);
		if (tryLoadPosFromFile(filename)) {
			cam.getPosPtr()->x = (float)posArray[0]; // x
			cam.getPosPtr()->y = (float)posArray[1]; // y
			cam.getPosPtr()->z = (float)posArray[2]; // z
		}
	}

#if !NO_NET
	if (mp) { netManager = new NetManager(server, terrain, &cam); }
#endif

	animalManager = new AnimalManager(&cam, terrain);
	if(filename) {
		animalManager->loadFromFile(filename);
	}

	railManager = new RailManager(terrain);

	mvmt = new Movement(&cam, terrain, railManager, FLY_MODE);
	
	landscapeRenderer = new LandscapeRenderer(terrain, railManager, &cam, animalManager);
	terrain->addObserver(landscapeRenderer);

	hudRenderer = new HudRenderer(terrain, &cam, animalManager);

	nearBlocksDirty = true;

	tntManager = new TNTManager(landscapeRenderer, terrain);

	highlightSelBlock = !MOBILE || activeInputMethod == IM_PC;

	glClearColor(0.6289f, 0.6953f, 0.9f, 1.0f);

	im = InputMethod::chooseIM(activeInputMethod, this, hudRenderer, mvmt);
	hudRenderer->initMeshes();
	hudRenderer->updateTexPreview(selectedTexture);
	
	if(!filename) {
		Vec3 *cposptr = cam.getPosPtr();
		cposptr->y = (float)terrain->getYOfBlockBelow((int)cposptr->x, (int)cposptr->y, (int)cposptr->z) + 5;
		animalManager->spawnAnimals();
	}

	crouching = false;
	
	touchWasReleased = false;
}

bool LandscapeScene::tryLoadPosFromFile(const char *filename) {
	DET_POS_FILE(filename);
	if (fileExists(posFilename)) {
		binaryRead(posFilename, posArray, sizeof(int) * 3);
	}
	return (posArray[0] != -1 && posArray[1] != -1 && posArray[2] != -1);
}

void LandscapeScene::savePosToFile(const char *filename) {
	DET_POS_FILE(filename);
	posArray[0] = (int)cam.getPosPtr()->x; // x
	posArray[1] = (int)cam.getPosPtr()->y; // y
	posArray[2] = (int)cam.getPosPtr()->z; // z
	binaryWrite(posFilename, posArray, sizeof(int) * 3);
}

void LandscapeScene::processKeyboardInput(bool *keys, SDLMod mod, ticks_t delta) {
	crouching = false;
#if !MOBILE && (SDL	|| MAC)
#if SDL
	crouching = (mod & KMOD_SHIFT) != 0;
#else
	crouching = false;
#endif

	Vec3 *t = cam.getUPtr();

	if (keys[KEY_A]) {
		mvmt->strafeLeft(delta * KMOVF, t, crouching);
	}
	if (keys[KEY_D]) {
		mvmt->strafeRight(delta * KMOVF, t, crouching);
	}
	if (keys[KEY_W] || keys[KEY_UP]) {
		mvmt->moveForward(delta * KMOVF, crouching);
	}
	if (keys[KEY_S] || keys[KEY_DOWN]) {
		mvmt->moveBackward(delta * KMOVF, crouching);
	}

	if (keys[KEY_LEFT]) {
		mvmt->rotateLeft(delta * KROTF);
	}
	if (keys[KEY_RIGHT]) {
		mvmt->rotateRight(delta * KROTF);
	}

	if (keys[KEY_O]/* || keys[KEY_P]*/) {
		g->setState(new MenuState(g, true));
		return;
	}

	if(keys[KEY_P]) {
		hudRenderer->toggleVisibility();
	}

	if (keys[KEY_RETURN] || keys[KEY_BACKSPACE]) {
		texSelOverlayActive = true;
		texSelOverlayFinished = false;
		startMouseMode();
		return;
	}

	if (keys[KEY_SPACE])
		mvmt->jump(false);

	// Toggle fly mode
	if (keys[KEY_F])
		mvmt->toggleFlyMode();

#endif
}

inline float LandscapeScene::calcDistToSelBlock() {
	float distToSelBlock = FLT_MAX;
	BlockPos *selBlock = landscapeRenderer->getSelectedBlock();
	if(selBlock) {
		Vec3 sbpos((float)selBlock->x, (float)selBlock->y, (float)selBlock->z);
		distToSelBlock = (cam.getPos() - sbpos).length();
	}
	return distToSelBlock;
}

void LandscapeScene::processMouseInput(int dX, int dY, int wheel, MouseButtons *mb, ticks_t delta) {
#if !MOBILE_TEST && (SDL || MAC)
	if (texSelOverlayActive && !texSelOverlayFinished) {
		if (mb->lmb) {
			int tX, tY;
#if SDL
			SDL_GetMouseState(&tX, &tY);
#else
			OSX_GetMousePos(&tX, &tY);
#endif
			texSelectClickTouch(this, hudRenderer, tX, tY);
			mouseWasReleased = false;
		} else mouseWasReleased = true;
	} else {
		if ((mb->lmb || mb->rmb)) {
			if (mouseWasReleased) {
				digMode = mb->lmb;
				
				if(animalManager->tryToHitAnimal(SCR_W / 2, SCR_H / 2, calcDistToSelBlock())) {
					mouseWasReleased = false;
					return;
				}
				doDigOrPut();
				mouseWasReleased = false;
			}
		} else mouseWasReleased = true;

		mvmt->mouseLook(dX, dY, delta);
	}

	if (texSelOverlayFinished && texSelOverlayActive)
		texSelOverlayActive = false;
#endif
}

void LandscapeScene::refreshBlocksNearCam() {
	// refresh blocksNearCam-list if neccessary.
	if (blocksNearCam.empty()
			|| !(lastCamPos == cam.getPos())
			|| !(lastCamNormal == cam.getNormal())
			|| nearBlocksDirty) {
		lastCamPos.setTo(cam.getPosPtr());
		blocksNearCam.clear();
		terrain->blocksNear(&cam, &blocksNearCam);
		if (nearBlocksDirty) nearBlocksDirty = false;
	}
}

inline bool LandscapeScene::isStandingEntity(int etype, bool isDoor, bool selDoor, CubeFace selectedFace) {
	return etype == Entity::GLASS || etype == Entity::FLOWER
		   || (etype == Entity::TORCH && selectedFace == CF_TOP)
		   || etype == Entity::MUSHROOM || (isDoor && !selDoor);
}

void LandscapeScene::doDigOrPut() {
	static bool selDoor;

	BlockPos *selectedBlock = landscapeRenderer->getSelectedBlock();
	CubeFace selectedFace = landscapeRenderer->getSelectedFace();

	if (selectedBlock != NULL) {
		int	actualX = selectedBlock->x;
		int actualY = selectedBlock->y;
		int actualZ = selectedBlock->z;

		// set actualX,Y,Z to the empty-position adjacent to the selected face.
		switch (selectedFace) {
		case CF_BACK:
			actualZ--;
			break;
		case CF_BOTTOM:
			actualY--;
			break;
		case CF_FRONT:
			actualZ++;
			break;
		case CF_LEFT:
			actualX--;
			break;
		case CF_RIGHT:
			actualX++;
			break;
		case CF_TOP:
			actualY++;
			break;
		}

		// put new block adjacent to selected face on right click
		if (!digMode &&  terrain->isValidIndex(actualX, actualY, actualZ)) {
			putNewBlock(selDoor, selectedBlock, actualX, actualY, actualZ, selectedFace);
			// on left/middle mouse button click remove entire selected block
		} else if (digMode && terrain->isValidIndex(selectedBlock->x, selectedBlock->y, selectedBlock->z)) {
			digExistingBlock(selectedBlock, selectedFace);
		} else return;

		lastBlockPlacementTicks = getTicks();
		nearBlocksDirty = true;
	}
}

void LandscapeScene::processTouch(int tX, int tY, ticks_t delta) {
	int tX2 = tX;
	int tY2 = tY;
	
#if IPHONE
	/*tX2 = SCR_H - tY;
	tY2 = SCR_W - tX;*/
	tX2 = tX;
	tY2 = SCR_H - tY;
	if (RETINA) { tX2 <<= 1; tY2 <<= 1; }
#endif

	if(!touchWasReleased) return;

	if(animalManager->tryToHitAnimal(tX2, tY2, calcDistToSelBlock())) {
		touchWasReleased = false;
		return;
	}
	
	im->processTouch(tX, tY, delta);
}

void LandscapeScene::processNoTouch() {
	im->processNoTouch();
	touchWasReleased = true;
}

void LandscapeScene::draw(ticks_t delta) {
	static int sx2, sy2;

#if !NO_NET
	if (netManager && netManager->sync()) {
		g->setState(new MenuState(g, true));
		return;
	}
#endif

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	cam.updateView();
	cam.apply();

	mvmt->update(delta, crouching);
	tntManager->update();
	animalManager->update(delta);

	// update selected block each frame on desktop
	if (!MOBILE_MODE || activeInputMethod == IM_PC) {
		refreshBlocksNearCam();
		int x = (int)(SCR_W / 2.0f), y = (int)(SCR_H / 2.0f);
#if IPHONE
		int tmp = x;
		//x = SCR_H - y;
		//y = SCR_W - tmp;
        x = tmp;
        y = SCR_H - y;
		if (RETINA) { x <<= 1; y <<= 1; }
#elif ANDROID
		y = SCR_H - y;
#endif
		landscapeRenderer->updateSelectedBlock(&blocksNearCam, x, y);
	} else { // update selected block only on dig/put on mobile
		if (sx != -1 && sy != -1) {
			refreshBlocksNearCam();

#if IPHONE
			// TODO: Find out why tX and tY are correct for touch
			// but messed up for picking...
			sx2 = sx;//SCR_H - sy;
			sy2 = SCR_H - sy;//SCR_W - sx;

			if (RETINA) { sx2 <<= 1; sy2 <<= 1; }
#elif (ANDROID || MOBILE_TEST) // TODO: Also investigate this workaround!
			sx2 = sx;
			sy2 = SCR_H - sy;
#endif

			landscapeRenderer->updateSelectedBlock(&blocksNearCam, sx2, sy2);
			highlightSelBlock = true;
		}
	}

	landscapeRenderer->render(highlightSelBlock);

	railManager->renderCart();

#if !NO_NET
	if (netManager) {
		Vec3 *otherPos = netManager->getOtherPosPtr();
		animalManager->renderPigAtPos(otherPos->x, otherPos->y, otherPos->z, netManager->getOtherYaw());
	}
#endif

	// FIXME: Isn't calling ortho end here unneccesary?
	hudRenderer->render(digMode, texSelOverlayActive);
}

void LandscapeScene::putNewBlock( bool &selDoor, BlockPos * selectedBlock, int actualX, int actualY, int actualZ, CubeFace selectedFace ) {
	int cx = (int)cam.getPos().x,
		cy = (int)(cam.getPos().y) - 1,
		cz = (int)cam.getPos().z;

	selDoor = (terrain->get(selectedBlock->x, selectedBlock->y, selectedBlock->z) == Terrain::INVIS_DOOR);

	if (terrain->get(actualX, actualY, actualZ) == 0) {
		if (cx == actualX && cz == actualZ
			&& (selectedTexture < 64 // and block-like entities
			|| selectedTexture == Entity::GLASS_TEX_INDEX
			|| selectedTexture == Entity::FLOWER_TEX_INDEX
			|| selectedTexture == Entity::MUSHROOM_TEX_INDEX
			|| selectedTexture == Entity::TORCH_TEX_INDEX
			|| selectedTexture == Terrain::FENCE_TEX_INDEX)) {
				if (cy == actualY) {
					if (terrain->isEmptyPos(cx, (int)(cam.getPos().y + 1.0f), cz))
						cam.setPos(cam.getPos() + Vec3(0, 1.0f, 0));
					else // don't push into block
						return;
				}
				// prevent block directly above us
				else if (cy == actualY - 1) {
					return;
				}
		}

		if (selDoor)
			selectedTexture = Entity::DOOR_TEX_INDEX;

		// entity handling
		if (selectedTexture >= Entity::LADDER_TEX_INDEX && selectedTexture < Entity::LADDER_TEX_INDEX + Entity::NUM_ENTITIES) {
			// TODO: adapt to observer pattern

			Entity::EntityType etype = Entity::LADDER;
			bool isDoor = false;

			if (selectedTexture != Entity::DOOR_TEX_INDEX)
				etype = (Entity::EntityType)(selectedTexture - Entity::LADDER_TEX_INDEX);
			else {
				if (!selDoor) // door orientation depends on camera position relative to selected block
					etype = (ABS(cx - actualX) > ABS(cz - actualZ)) ? Entity::DOOR_X : Entity::DOOR_Z;
				else {
					std::list<Entity> eap = terrain->getEntitiesAtPos(selectedBlock->x, selectedBlock->y, selectedBlock->z);
					std::list<Entity> eapBelow = terrain->getEntitiesAtPos(selectedBlock->x, selectedBlock->y - 1, selectedBlock->z);
					std::list<Entity>::iterator it;

					bool doBreak, isBelow = true;

					for (it = eap.begin(); it != eap.end(); ++it) {
						doBreak = true;
						switch ((*it).type) {
						case Entity::DOOR_X:
							etype = Entity::DOOR_X_OPEN;
							break;
						case Entity::DOOR_Z:
							etype = Entity::DOOR_Z_OPEN;
							break;
						case Entity::DOOR_X_OPEN:
							etype = Entity::DOOR_X;
							break;
						case Entity::DOOR_Z_OPEN:
							etype = Entity::DOOR_Z;
							break;
						default:
							doBreak = false;
							break;
						}
						if (doBreak) { isBelow = false; break; }
					}

					for (it = eapBelow.begin(); it != eapBelow.end(); ++it) {
						doBreak = true;
						switch ((*it).type) {
						case Entity::DOOR_X:
							etype = Entity::DOOR_X_OPEN;
							break;
						case Entity::DOOR_Z:
							etype = Entity::DOOR_Z_OPEN;
							break;
						case Entity::DOOR_X_OPEN:
							etype = Entity::DOOR_X;
							break;
						case Entity::DOOR_Z_OPEN:
							etype = Entity::DOOR_Z;
							break;
						default:
							doBreak = false;
							break;
						}
						if (doBreak) break;
					}

					if (isBelow)
						selectedBlock->y--;

					terrain->removeEntityAt(selectedBlock->x, selectedBlock->y, selectedBlock->z);

#if !NO_NET
					if(netManager) {
						netManager->sendEntityDeletion(selectedBlock->x, selectedBlock->y, selectedBlock->z);
					}
#endif
				}
				isDoor = true;
			}

			// ladders can only be placed on block sides
			if ((etype == Entity::LADDER) && (selectedFace == CF_TOP || selectedFace == CF_BOTTOM))
				return;

			// flowers, mushrooms and new doors only on ground
			if ((etype == Entity::FLOWER || etype == Entity::MUSHROOM || etype == Entity::RAIL || (isDoor && !selDoor)) && selectedFace != CF_TOP)
				return;

			// torches on ground and side (but not bottom)
			if (etype == Entity::TORCH && selectedFace == CF_BOTTOM)
				return;

			int ex, ey, ez;

			if (isStandingEntity(etype, isDoor, selDoor, selectedFace)) {
				ex = actualX;
				ey = actualY;
				ez = actualZ;
			} else {
				ex = selectedBlock->x;
				ey = selectedBlock->y;
				ez = selectedBlock->z;
			}

			if(etype == Entity::RAIL) {
				railManager->addRail(BlockPos(ex, ey, ez));
				/*hudRenderer->startPutAnim();
				playSound(SND_PUT);
				return;*/
			}

			if (!terrain->addEntity(Entity(ex, ey, ez, etype, selectedFace)))
				return;
			else {
				#if !NO_NET
				if(netManager)
					netManager->sendEntity(Entity(ex, ey, ez, etype, selectedFace));
				#endif
			}

			if (selDoor) return;

			if (isDoor) {
				terrain->set(actualX, actualY, actualZ, Terrain::INVIS_DOOR);
				terrain->set(actualX, actualY + 1, actualZ, Terrain::INVIS_DOOR);

				if(netManager) {
					#if !NO_NET
					netManager->sendSet(actualX, actualY, actualZ, Terrain::INVIS_DOOR);
					netManager->sendSet(actualX, actualY+1, actualZ, Terrain::INVIS_DOOR);
					#endif
				}

			} else if (isStandingEntity(etype, isDoor, selDoor, selectedFace)) {
				terrain->set(actualX, actualY, actualZ, Terrain::INVIS_SOLID);

				if(netManager) {
					#if !NO_NET
					netManager->sendSet(actualX, actualY, actualZ, Terrain::INVIS_DOOR);
					#endif
				}
			}
		}
		// putting fire lets adjacent TNT explode!
		else if (selectedTexture == TNTManager::FIRE_TEX_INDEX) {
			// no TNT explosions in multiplayer for now!
			if(netManager) return;

			int blockBelowY = terrain->getYOfBlockBelow(actualX, actualY, actualZ);
			landscapeRenderer->addExplAt(actualX, actualY, actualZ, TNTManager::FIRE_TEX_INDEX, blockBelowY);
			tntManager->fireAt(actualX, actualY, actualZ);
		}
		// putting pig face texture spawns a pig
		else if (selectedTexture == Animal::PIG_TEX_INDEX) {
			// no pigs in multiplayer for now!
			if(netManager) return;

			animalManager->addAnimalAtPos((float)actualX + ANIMAL_SCALE, (float)actualY + ANIMAL_SCALE, (float)actualZ + ANIMAL_SCALE);
			// putting any other block type (e.g. grass, dirt, ...)
		} else {
			if(selectedTexture == Terrain::CART_TEX_INDEX) {
				if(railManager->isRailAtPos(*selectedBlock)) {
					railManager->spawnCartAtPos(BlockPos(actualX, actualY, actualZ));
					hudRenderer->startPutAnim();
					playSound(SND_PUT);					
				}
				return;
			}

			terrain->set(actualX, actualY, actualZ, selectedTexture + 1);

#if !NO_NET
			if(netManager)
				netManager->sendSet(actualX, actualY, actualZ, selectedTexture+1);
#endif
		}

		hudRenderer->startPutAnim();
		playSound(SND_PUT);
	}
}

void LandscapeScene::digExistingBlock( BlockPos * selectedBlock, CubeFace selectedFace )
{
	// air can't be digged!
	if (!terrain->get(selectedBlock->x, selectedBlock->y, selectedBlock->z))
		return;

	hudRenderer->startDigAnim();

	if (selectedBlock->y > 0) {
		int sbx = selectedBlock->x,
			sby = selectedBlock->y,
			sbz = selectedBlock->z;

		int val = terrain->get(sbx, sby, sbz) - 1;
		int blockBelowY = terrain->getYOfBlockBelow(sbx, sby, sbz);

		bool digEntireBlock = false;

		// special handling for doors
		if (val + 1 == Terrain::INVIS_DOOR) {
			int yoff = (terrain->get(sbx, sby - 1, sbz) == Terrain::INVIS_DOOR) ? -1 : 1;
			terrain->removeEntityAt(sbx, sby + yoff, sbz);
			val = 6;
			landscapeRenderer->addExplAt(sbx, sby + yoff, sbz, val, blockBelowY);
			terrain->set(sbx, sby + yoff, sbz, 0);

#if !NO_NET
			if(netManager) {
				netManager->sendEntityDeletion(sbx, sby+yoff, sbz);
				netManager->sendSet(sbx, sby + yoff, sbz, 0);
			}
#endif

			digEntireBlock = true;

		} else if (val + 1 == Terrain::INVIS_SOLID || val == Terrain::FENCE_TEX_INDEX) {
			val = 6;
			digEntireBlock = true;
		}

		if(netManager) digEntireBlock = true;

		// remove all adj entities on this block
		if(digEntireBlock) {
			terrain->removeEntityAt(sbx, sby, sbz);
			railManager->tryToRemoveRail(*selectedBlock);
			landscapeRenderer->addExplAt(sbx, sby, sbz, val, blockBelowY);
			terrain->set(sbx, sby, sbz, 0);
			// if in multiplayer also send over local dig
#if !NO_NET
			if(netManager) {
				netManager->sendSet(sbx, sby, sbz, 0);
				netManager->sendEntityDeletion(sbx, sby, sbz);
			}
#endif
		}
		else {
			if(terrain->removeEntityAt(sbx, sby, sbz, selectedFace)) {
				if(selectedFace == CF_TOP)
					railManager->tryToRemoveRail(*selectedBlock);

				landscapeRenderer->addExplAt(sbx, sby, sbz, 6, blockBelowY);
			}
			else {
				landscapeRenderer->addExplAt(sbx, sby, sbz, val, blockBelowY);
				terrain->set(sbx, sby, sbz, 0);
			}
		}	
	} else if(selectedBlock->y == 0 && railManager->tryToRemoveRail(*selectedBlock)) {
		terrain->removeEntityAt(selectedBlock->x, selectedBlock->y, selectedBlock->z);		
	} else {
		return;
	}

	playSound(SND_DIG);
}



}

