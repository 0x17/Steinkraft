// HudRenderer.cpp



#include <cstdio>
#include <cmath>

#include "../Constants.h"

#include "../Framework/PGL.h"
#include "../Framework/Utilities.hpp"
#include "../Framework/Camera.hpp"
#include "../Framework/SpriteBatch.hpp"

#include "Meshes/QuadMesh.hpp"
#include "Meshes/BlockMesh.hpp"

#include "../Managers/NetManager.hpp"

#include "../States/LandscapeScene.hpp"

#include "../Terrain.hpp"
#include "../InputMethods.hpp"

#include "HudRenderer.hpp"
#include "Animation.hpp"

#include "../Managers/AnimalManager.hpp"

namespace as {
//===========================================================================
// Constants/Macros
//===========================================================================

//! Texture coordinate rectangles
const TexCoordRect	TXR_TEX(0.0f, 128.0f, 0.0f, 128.0f);
const TexCoordRect	TXR_ENTITIES(128.0f, 144.0f, 0.0f, 16.0f * (Entity::NUM_ENTITIES + 1));
const TexCoordRect	TXR_OUTER_CIRC(128.0f, 256.0f, 128.0f, 256.0f);
const TexCoordRect	TXR_INNER_CIRC(0.0f, 32.0f, 128.0f, (128.0f + 32.0f));
const TexCoordRect	TXR_DIG_BTN(32.0f, 64.0f, 128.0f, (128.0f + 32.0f));
const TexCoordRect	TXR_PUT_BTN(64.0f, 96.0f, 128.0f, (128.0f + 32.0f));
const TexCoordRect	TXR_JUMP_BTN(96.0f, 128.0f, 128.0f, 160.0f);
const TexCoordRect	TXR_MENU_BTN(0.0f, 64.0f, 160.0f, 176.0f);
const TexCoordRect	TXR_FBACK_INNER(48.0f, 64.0f, 192.0f, 208.0f);
const TexCoordRect	TXR_FBACK_OUTER(64.0f, 96.0f, 160.0f, 192.0f);
const TexCoordRect	TXR_DPAD_LEFT(0.0f, 16.0f, 176.0f, 192.0f);
const TexCoordRect	TXR_DPAD_RIGHT(48.0f, 64.0f, 176.0f, 192.0f);
const TexCoordRect	TXR_DPAD_UP(16.0f, 32.0f, 192.0f, 208.0f);
const TexCoordRect	TXR_DPAD_DOWN(0.0f, 16.0f, 192.0f, 208.0f);
const TexCoordRect	TXR_DPAD_CENTER(32.0f, 48.0f, 192.0f, 208.0f);

// crosshair related
const int	CSH_TX_ROW = 0;
const int	CSH_TX_COL = 7;
const int	CSH_SIZE = 32;

#define CENTERED(w,h)	SCR_W/2-w/2, SCR_H/2-h/2
	
HudRenderer *HudRenderer::instance;

//===========================================================================
// Methods
//===========================================================================

/**
 Constructs new heads up display renderer.
*/
HudRenderer::HudRenderer(Terrain *_terrain, Camera *_cam, AnimalManager *_animalMgr)
:	cam(_cam),
	terrain(_terrain),
	animalMgr(_animalMgr),

	texPreviewMesh(NULL),
	blockPreviewMesh(NULL),
	darkBlockPreviewMesh(NULL),

	fsb(NULL),

	texOverlayMesh(NULL),
	entityOverlayMesh(NULL),

	entitySelected(false),
	drawfcircles(false),

	hidden(false),
	lastSelectedTexture(0)
{
	instance = this;
	digAnim = new Animation(500);
	putAnim = new Animation(250);
	scaleFactor = SCR_H / 320.0f * 1.5f;
}

void HudRenderer::initMeshes() {
	fsb = new FontSpriteCache(false);

	crosshairMesh = new QuadMesh(CSH_TX_ROW, CSH_TX_COL, CENTERED(CSH_SIZE, CSH_SIZE), CSH_SIZE, CSH_SIZE);
	texOverlayMesh = new QuadMesh(TXR_TEX, (int)((SCR_W - SCR_H) / 2.0f), 0, SCR_H, SCR_H);

	entityOverlayMesh = new QuadMesh(TXR_ENTITIES,
			(int)(SCR_W - 24 * scaleFactor), (int)(SCR_H - 24 * (Entity::NUM_ENTITIES + 1) * scaleFactor), // x,y
			(int)(24 * scaleFactor), (int)(24 * (Entity::NUM_ENTITIES + 1) * scaleFactor)); // w,h

	minecartOverlayMesh = new QuadMesh(7, 9, SCR_W - scl(16), 0, scl(16), scl(16));
	
	fenceOverlayMesh = new QuadMesh(4, 11, 0, 0, scl(16), scl(16));

#if MOBILE_MODE
	leftOuterCircleMesh = new QuadMesh(TXR_OUTER_CIRC, gInputMethod->getRectWithIndex(InputMethod::RI_LEFT_JOYSTICK), 0.5f);

	// right outer circle depends on default&pc vs. opposed
	bool defIM = (activeInputMethod == IM_DEFAULT) || (activeInputMethod == IM_PC);
	int rocX = (defIM) ? SCR_W - scl(64) : SCR_W - scl(64 + 32);
	int rocY = (defIM) ? 0 : SCR_H - scl(64);
	rightOuterCircleMesh = new QuadMesh(TXR_OUTER_CIRC, rocX, rocY, scl(64), scl(64), 0.5f);

	innerCircleMesh = new QuadMesh(TXR_INNER_CIRC, 0, 0, 32, 32);

	digBtnMesh = new QuadMesh(TXR_DIG_BTN, gInputMethod->getRectWithIndex(InputMethod::RI_DIG_BTN));
	putBtnMesh = new QuadMesh(TXR_PUT_BTN, gInputMethod->getRectWithIndex(InputMethod::RI_DIG_BTN));

	jumpBtnMesh = new QuadMesh(TXR_JUMP_BTN, gInputMethod->getRectWithIndex(InputMethod::RI_JUMP_BTN));
	menuBtnMesh = new QuadMesh(TXR_MENU_BTN, gInputMethod->getRectWithIndex(InputMethod::RI_MENU_BTN));

	innerFeedbackCircle = new QuadMesh(TXR_FBACK_INNER, 0, 0, scl(96), scl(96));
	outerFeedbackCircle = new QuadMesh(TXR_FBACK_OUTER, 0, 0, scl(96), scl(96));

	dpadLeftMesh	= new QuadMesh(TXR_DPAD_LEFT, gInputMethod->getRectWithIndex(InputMethod::RI_DPAD_LEFT), 0.5f);
	dpadRightMesh	= new QuadMesh(TXR_DPAD_RIGHT, gInputMethod->getRectWithIndex(InputMethod::RI_DPAD_RIGHT), 0.5f);
	dpadUpMesh		= new QuadMesh(TXR_DPAD_UP, gInputMethod->getRectWithIndex(InputMethod::RI_DPAD_UP), 0.5f);
	dpadDownMesh	= new QuadMesh(TXR_DPAD_DOWN, gInputMethod->getRectWithIndex(InputMethod::RI_DPAD_DOWN), 0.5f);
	dpadJumpMesh	= new QuadMesh(TXR_DPAD_CENTER, gInputMethod->getRectWithIndex(InputMethod::RI_DPAD_CENTER), 0.5f);
#endif
}

/**
 Destruct heads up display renderer object.
*/
HudRenderer::~HudRenderer() {
#if MOBILE_MODE
	SAFE_DELETE(dpadUpMesh);
	SAFE_DELETE(dpadDownMesh);
	SAFE_DELETE(dpadLeftMesh);
	SAFE_DELETE(dpadRightMesh);
	SAFE_DELETE(dpadJumpMesh);

	SAFE_DELETE(innerFeedbackCircle);
	SAFE_DELETE(outerFeedbackCircle);
	SAFE_DELETE(leftOuterCircleMesh);
	SAFE_DELETE(rightOuterCircleMesh);
	SAFE_DELETE(innerCircleMesh);
	SAFE_DELETE(digBtnMesh);
	SAFE_DELETE(putBtnMesh);
	SAFE_DELETE(jumpBtnMesh);
	SAFE_DELETE(menuBtnMesh);
#endif

	SAFE_DELETE(fenceOverlayMesh);
	SAFE_DELETE(minecartOverlayMesh);
	SAFE_DELETE(entityOverlayMesh);
	SAFE_DELETE(texOverlayMesh);
	SAFE_DELETE(fsb);
	SAFE_DELETE(putAnim);
	SAFE_DELETE(digAnim);
	SAFE_DELETE(texPreviewMesh);
	SAFE_DELETE(crosshairMesh);
	SAFE_DELETE(blockPreviewMesh);
	SAFE_DELETE(darkBlockPreviewMesh);
}

/**
 Called when starting drawing feedback circles at a new position.
*/
void HudRenderer::setDrawFeedbackCircles(bool drawFeedbackCircles, float _fcx, float _fcy, float _fciz) {
	static float sub = 96.0f * scaleFactor * 0.5f;

	this->fcx = _fcx;
	this->fcy = SCR_H - _fcy;

	this->fcy -= sub;
	this->fcx -= sub;

	this->fciz = _fciz;
	drawfcircles = drawFeedbackCircles;
}

/**
 Update block preview meshes (3D) and tex preview mesh (2D ortho).
*/
void HudRenderer::updateTexPreview(int selectedTexture) {
	SAFE_DELETE(texPreviewMesh);
	SAFE_DELETE(blockPreviewMesh);
	SAFE_DELETE(darkBlockPreviewMesh);
	
	if(selectedTexture == -1)
		selectedTexture = lastSelectedTexture;
	else
		lastSelectedTexture = selectedTexture;

	int row, col;
	row = selectedTexture / NUM_TEX_PER_ROW;
	col = selectedTexture % NUM_TEX_PER_ROW;

	if (selectedTexture >= 64 && selectedTexture < 128) { // entities
		row = (selectedTexture == 70) ? 0 : 2 + (selectedTexture - 64);
		col = 8;
		if (selectedTexture == 66) { // GLASS
			blockPreviewMesh = new BlockMesh(row, col, 1.0f);
			darkBlockPreviewMesh = new BlockMesh(row, col, 0.5f);
		} else {
			blockPreviewMesh = new BlockMesh(row, col, 1.0f, CF_FRONT, 1.0f);
			darkBlockPreviewMesh = new BlockMesh(row, col, 1.0f, CF_FRONT, 0.5f);
		}
		entitySelected = true;
	} else if (selectedTexture < 64) { // normal blocks
		blockPreviewMesh = new BlockMesh(row, col);
		darkBlockPreviewMesh = new BlockMesh(row, col, 0.5f);
		entitySelected = false;
	} else if(selectedTexture == Terrain::CART_TEX_INDEX) {
		blockPreviewMesh = new BlockMesh(7, 9, 1.0f, CF_FRONT, 1.0f);
		darkBlockPreviewMesh = new BlockMesh(7, 9, 1.0f, CF_FRONT, 0.5f);
		row = 7; 
		col = 9;
		entitySelected = true;
	} else if(selectedTexture == Terrain::FENCE_TEX_INDEX) {
		row = 4; 
		col = 11;
		blockPreviewMesh = new BlockMesh(row, col, 1.0f, CF_FRONT, 1.0f);
		darkBlockPreviewMesh = new BlockMesh(row, col, 1.0f, CF_FRONT, 0.5f);
		entitySelected = true;
	} else { // tools
		row = 2 + selectedTexture - 128;
		col = 9;
		blockPreviewMesh = new BlockMesh(row, col, 1.0f, CF_FRONT, 1.0f);
		darkBlockPreviewMesh = new BlockMesh(row, col, 1.0f, CF_FRONT, 0.5f);
		entitySelected = true; // for correct display
	}

	texPreviewMesh = new QuadMesh(row, col, 0, (int)(SCR_H - scl(32)),
						(int)(scl(32)), (int)(scl(32)));
}

/**
 Render player position and frames per second as text overlay.
*/
void HudRenderer::drawInfo() {
	static char fpsStr[BUF_LEN], posStr[BUF_LEN];
	static long fps, lastTicks = 0;
	long divisor = (getTicks() - lastTicks);
	int textX = (int)(scl(32)) + 10;

	Vec3 *camPos = cam->getPosPtr();

	if (divisor > 0)
		fps = 1000 / divisor;
	else fps = 99999;

	lastTicks = getTicks();
	std::sprintf(fpsStr, "FPS %ld", fps);
	fsb->addText(textX, SCR_H - 20, fpsStr);
	std::sprintf(posStr, "x %.0f", camPos->x);
	fsb->addText(textX, SCR_H - 40, posStr);
	std::sprintf(posStr, "y %.0f", camPos->y);
	fsb->addText(textX, SCR_H - 60, posStr);
	std::sprintf(posStr, "z %.0f", camPos->z);
	fsb->addText(textX, SCR_H - 80, posStr);
#if DEBUG_MODE
	std::sprintf(posStr, "blocks near %d", nblocks_near);
	fsb->addText(textX, SCR_H - 100, posStr);
#endif
}

void HudRenderer::drawAnimalOverlays()
{
	std::list<AnimalOverlay> *overlays = animalMgr->genOverlays();

	for(std::list<AnimalOverlay>::const_iterator it = overlays->begin(); it != overlays->end(); ++it) {
		AnimalOverlay ao = (*it);
		fsb->addText((int)ao.pos.x, (int)ao.pos.y, ao.text);
	}
	
	SAFE_DELETE(overlays);
}

/**
 Render all heads up display overlay graphics.
*/
void HudRenderer::render(bool digMode, bool drawTexSelOverlay) {
	if(hidden) return;

	glDisable(GL_FOG);

	drawBlockPreview();

	orthoCam.apply();
	glEnable(GL_BLEND);

//#if !ANDROID
	fsb->clear();
	drawInfo();
	drawAnimalOverlays();
	fsb->render();
//#endif

	drawHeadsUpDisplay();

	if (drawTexSelOverlay) {
		texOverlayMesh->render();
		entityOverlayMesh->render();
		minecartOverlayMesh->render();
		fenceOverlayMesh->render();
	}
#if MOBILE_MODE
	else {
		drawControlsOverlay(digMode);
	}
#endif

	glDisable(GL_BLEND);
	glEnable(GL_FOG);
}

/**
 Draw overlay for controls. Visual part depending on active input method.
*/
void HudRenderer::drawControlsOverlay(bool digMode) {
	// draw left virtual joystick or left virtual dpad
	if (activeInputMethod != IM_RIPOFF_DPAD) {
		leftOuterCircleMesh->render();
		glPushMatrix();
		glTranslatef((float)(lastLeftJoystickTouch[0] - 16), (float)(SCR_H - lastLeftJoystickTouch[1] - 16), 0.0f);
		innerCircleMesh->render();
		glPopMatrix();
	} else { // TODO: Merge into one mesh for more efficiency
		dpadLeftMesh->render();
		dpadRightMesh->render();
		dpadUpMesh->render();
		dpadDownMesh->render();
		dpadJumpMesh->render();
	}

	// draw right virtual joystick
	if (activeInputMethod != IM_RIPOFF && activeInputMethod != IM_RIPOFF_DPAD) {
		rightOuterCircleMesh->render();

		glPushMatrix();
		glTranslatef((float)(lastRightJoystickTouch[0] - 16), (float)(SCR_H - lastRightJoystickTouch[1] - 16), 0.0f);
		innerCircleMesh->render();
		glPopMatrix();

		if (!digMode)
			digBtnMesh->render();
		else
			putBtnMesh->render();
	}

	menuBtnMesh->render();

	if (activeInputMethod != IM_RIPOFF_DPAD)
		jumpBtnMesh->render();

	if (drawfcircles)
		drawFeedbackCircles();
}

/**
 Draw feedback circles showed when digging in ALT mode.
*/
void HudRenderer::drawFeedbackCircles() {
	static float sub;

	glPushMatrix();

	glTranslatef(fcx, fcy, 0.0f);
	outerFeedbackCircle->render();

	sub = (scaleFactor * 48.0f) * (1.0f - fciz);

	glTranslatef(sub, sub, 0.0f);
	glScalef(fciz, fciz, 1.0f);

	innerFeedbackCircle->render();

	glPopMatrix();
}

/**
 Draw tex preview mesh.
 Also draw crosshair on desktop version.
*/
void HudRenderer::drawHeadsUpDisplay() {
	if (texPreviewMesh != NULL) {
		texPreviewMesh->render();
	}

	if (!MOBILE_MODE || activeInputMethod == IM_PC) {
		crosshairMesh->render();
	}
}

/**
 Draw 3D animated block preview on bottom right.
*/
void HudRenderer::drawBlockPreview() {
	static float k;
	static int cx, cy, cz;
	static bool blockAbove;

	cx = (int)cam->getPos().x;
	cy = (int)cam->getPos().y;
	cz = (int)cam->getPos().z;

	blockAbove = terrain->isBlockAbove(cx, cy, cz);

	glDisable(GL_DEPTH_TEST);
	glPushMatrix();
	glLoadIdentity();

	if (!entitySelected)	{
		glTranslatef(3, -3, -4.5);
	} else {
		glTranslatef(2, -2, -4.5);
	}

	glScalef(2.0f, 2.0f, 2.0f);

	if (digAnim->update()) {
		k = (float)sinf(digAnim->counter);
		glTranslatef(0, -k, -k);
		glRotatef(k*16*PI, -1.0f, 0.5f, 0.0f);
	}

	if (putAnim->update()) {
		glTranslatef(0, -putAnim->counter*1.5f, 0);
	}

	if (entitySelected) {
		glEnable(GL_BLEND);
	}

	if (blockAbove) {
		darkBlockPreviewMesh->render();
	} else {
		blockPreviewMesh->render();
	}

	if (entitySelected) {
		glDisable(GL_BLEND);
	}

	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}

/**
 Start digging animation.
 Call when digging a block.
*/
void HudRenderer::startDigAnim() {
	putAnim->reset();
	digAnim->start();
}

/**
 Start putting block animation.
 Call when putting a new block in the terrain.
*/
void HudRenderer::startPutAnim() {
	digAnim->reset();
	putAnim->start();
}


} /* namespace as */
