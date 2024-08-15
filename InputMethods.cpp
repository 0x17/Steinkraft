// InputMethods.cpp

#include "StdAfx.h"
#pragma hdrstop

#include "Framework/Camera.hpp"
#include "Rendering/HudRenderer.hpp"
#include "States/LandscapeScene.hpp"
#include "States/MenuState.hpp"

#include "InputMethods.hpp"
#include "Movement.hpp"

namespace as {

//==============================================================================
// Globals
//==============================================================================
int lastLeftJoystickTouch[2], lastRightJoystickTouch[2];

//==============================================================================
// Constants/Macros
//==============================================================================
bool isDefaultIM;

const float		ROT_FC = 4.0f;
const float		MOV_FC = 0.2f;

const ticks_t	SHORT_TAP_DUR = 300;
const ticks_t	LONG_TAP_DUR = 600;

const int MAX_DRAG_DELTA = (int)((float)SCR_W / 4.0f);

//==============================================================================
// Methods
//==============================================================================

// select texture from touch coordinates
// (or do nothing if outside texture rectangle).
void finishTexSelection(LandscapeScene *lscene, HudRenderer *hudRenderer) {
	lscene->texSelOverlayFinished = true;
	hudRenderer->updateTexPreview(lscene->selectedTexture);
	stopMouseMode();
}

void texSelectClickTouch(LandscapeScene *lscene, HudRenderer *hudRenderer, int tX, int tY) {
	if (tX >= (SCR_W - SCR_H) / 2.0f && tX <= (SCR_W + SCR_H) / 2.0f) {

		DATA_TYPE row = (DATA_TYPE)(tY / (SCR_H / 8.0f));
		DATA_TYPE col = (DATA_TYPE)((tX - (SCR_W - SCR_H) / 2.0f) / (SCR_H / 8.0f));

		lscene->selectedTexture = row * 8 + col;
		finishTexSelection(lscene, hudRenderer);

	} else if (tX >= SCR_W - 24.0f * hudRenderer->getScaleFactor()
			   && tY <= (24.0f*8) * hudRenderer->getScaleFactor()) {

		DATA_TYPE row = (DATA_TYPE)(tY / (24.0f * hudRenderer->getScaleFactor()));

		if (row == 0 || row == 1) row = Entity::DOOR_X; /* door */
		else row -= 2;

		lscene->selectedTexture = Entity::LADDER_TEX_INDEX + row;

		finishTexSelection(lscene, hudRenderer);
	} else if(tX >= SCR_W - 16.0f * hudRenderer->getScaleFactor() && tY >= SCR_H - 16.0f * hudRenderer->getScaleFactor()) {
		lscene->selectedTexture = Terrain::CART_TEX_INDEX;
		finishTexSelection(lscene, hudRenderer);
	} else if(tX <= 16.0f * hudRenderer->getScaleFactor() && tY >= SCR_H - 16.0f * hudRenderer->getScaleFactor()) {
		lscene->selectedTexture = Terrain::FENCE_TEX_INDEX;
		finishTexSelection(lscene, hudRenderer);
	}
}

inline void InputMethod::calcRightJoystick() {
	leftBorder = (int)((isDefaultIM) ? SCR_W - scl(64) : SCR_W - scl(64 + 32));
	rightBorder = (int)((isDefaultIM) ? SCR_W : SCR_W - scl(32));
	upperBorder = (int)((isDefaultIM) ? SCR_H - scl(64) : 0);
	lowerBorder = (int)((isDefaultIM) ? SCR_H : scl(64));
}

inline bool InputMethod::nearLeftJoystick(int tX, int tY, int off) const {
	return	(activeInputMethod == IM_RIPOFF) ?
		   (tX < (64 + off) * scaleFactor && tY >= SCR_H - (64 + off) * scaleFactor)
		   : (tX < SCR_W / 2);
}

inline bool InputMethod::nearRightJoystick(int tX/*, int tY, int off*/) const {
	return tX > SCR_W / 2;
}

inline bool InputMethod::inButtonArea(int x, int y) const {
	return inRect(x, y, InputMethod::RI_MENU_BTN) || inRect(x, y, InputMethod::RI_JUMP_BTN) || inRect(x, y, InputMethod::RI_TEX_SQUARE);
}

inline bool InputMethod::inDPADArea(int x, int y) const {
	return x <= DPAD_BTN_SIZE * 3 && y >= SCR_H - DPAD_BTN_SIZE * 3;
}

/**
 * Factory method.
 */
InputMethod *InputMethod::chooseIM(InputMethodType method, LandscapeScene *lscene, HudRenderer *hudRenderer, Movement *mvmt) {
	InputMethod *im;

	isDefaultIM = (activeInputMethod == IM_DEFAULT) || (activeInputMethod == IM_PC);

	switch (method) {
	default:
	case IM_DEFAULT:
		im = new DefaultIM(lscene, hudRenderer, mvmt);
		break;
	case IM_OPPOSED:
		im = new DefaultIM(lscene, hudRenderer, mvmt, true);
		break;
	case IM_RIPOFF:
	case IM_RIPOFF_DPAD:
		im = new AltIM(lscene, hudRenderer, mvmt, method == IM_RIPOFF_DPAD);
		break;
	case IM_PC:
		im = new PcIM(lscene, hudRenderer, mvmt);
		break;
	}

	return im;
}

//==============================================================================
InputMethod *gInputMethod = NULL;

InputMethod::InputMethod(LandscapeScene *_lscene, HudRenderer *_hudRenderer, Movement *_mvmt)
:	mvmt(_mvmt),
	hudRenderer(_hudRenderer),
	lscene(_lscene),
	cam(_lscene->getCam()),

	wasReleased(false),

	lastJumpPress(0),
	lastRipoffDig(getTicks()),

	movingFast(false),
	movingAround(false),
	lookingAround(false),

	tapStarted(0),
	tapX(0), tapY(0),
	dragDist(0),

	scaleFactor(hudRenderer->getScaleFactor()),
	c(480.0f / SCR_H)
{
	gInputMethod = this;

	lscene->texSelOverlayActive = false;
	lscene->texSelOverlayFinished = false;

	// joystick positions to reset to on touch release
	lJoyCtrX = (scl(32));
	lJoyCtrY = (SCR_H - scl(32));
	rJoyCtrX = ((isDefaultIM) ? (SCR_W - scl(32)) : (SCR_W - scl(64)));
	rJoyCtrY = ((isDefaultIM) ? (SCR_H - scl(32)) : scl(32));

	calcRightJoystick();
	resetJoystickPos();
	addRectangles();
}

inline void InputMethod::addRect(RectangleIndices index, Rect rect) {
	rectangles[index] = rect;
}

inline bool InputMethod::inRect(int x, int y, RectangleIndices index) const {
	return rectangles[index].contains(x, y);
}

Rect InputMethod::getRectWithIndex(RectangleIndices index) const {
	return rectangles[index];
}

void InputMethod::addRectangles() {
	rectangles[InputMethod::RI_MENU_BTN] = Rect((SCR_W - scl(64)) / 2, 0, scl(64), scl(16));
	rectangles[InputMethod::RI_JUMP_BTN] = Rect(SCR_W - scl(32), scl(32), scl(32), scl(32));
	rectangles[InputMethod::RI_DIG_BTN] = Rect(SCR_W - scl(32), 0, scl(32), scl(32));
	rectangles[InputMethod::RI_TEX_SQUARE] = Rect(0, 0, scl(32), scl(32));
	rectangles[InputMethod::RI_LEFT_JOYSTICK] = Rect(0, SCR_H - scl(64), scl(64), scl(64));
	rectangles[InputMethod::RI_RIGHT_JOYSTICK] = Rect(leftBorder, upperBorder, rightBorder-leftBorder, lowerBorder-upperBorder);
	rectangles[InputMethod::RI_DPAD_LEFT] = Rect(0.0f, SCR_H - DPAD_BTN_SIZE * 2.0f, DPAD_BTN_SIZE, DPAD_BTN_SIZE);
	rectangles[InputMethod::RI_DPAD_RIGHT] = Rect(DPAD_BTN_SIZE * 2.0f, SCR_H - DPAD_BTN_SIZE * 2.0f, DPAD_BTN_SIZE, DPAD_BTN_SIZE);
	rectangles[InputMethod::RI_DPAD_UP] = Rect(DPAD_BTN_SIZE, SCR_H - DPAD_BTN_SIZE * 3.0f, DPAD_BTN_SIZE, DPAD_BTN_SIZE);
	rectangles[InputMethod::RI_DPAD_DOWN] = Rect(DPAD_BTN_SIZE, SCR_H - DPAD_BTN_SIZE, DPAD_BTN_SIZE, DPAD_BTN_SIZE);
	rectangles[InputMethod::RI_DPAD_CENTER] = Rect(DPAD_BTN_SIZE, SCR_H - DPAD_BTN_SIZE * 2.0f, DPAD_BTN_SIZE, DPAD_BTN_SIZE);
}

InputMethod::~InputMethod() {}

void InputMethod::resetJoystickPos(bool left, bool right) {
	if (left) {
		lastLeftJoystickTouch[0] = lJoyCtrX;
		lastLeftJoystickTouch[1] = lJoyCtrY;
		movingFast = false;
		movingAround = false;
	}
	if (right) {
		lastRightJoystickTouch[0] = rJoyCtrX;
		lastRightJoystickTouch[1] = rJoyCtrY;
	}
}

bool InputMethod::procMenuTexBtn(int tX, int tY) {
	// menu button (all input methods)
	if (inRect(tX, tY, InputMethod::RI_MENU_BTN)
			&& !movingAround && !lookingAround
			&& (activeInputMethod == IM_PC || !lscene->highlightSelBlock)) {
		StateManager *g = lscene->getGame();
		g->setState(new MenuState(g, true));
		return true;
	}

	// clicking texture square switches texture (all input methods)
	if (inRect(tX, tY, InputMethod::RI_TEX_SQUARE) && !movingAround && !lookingAround) {
		lscene->texSelOverlayActive = true;
		lscene->texSelOverlayFinished = false;
		return true;
	}

	return false;
}

bool InputMethod::processLeftDPadTouch(int tX, int tY, ticks_t delta) {
	if (inRect(tX, tY, InputMethod::RI_DPAD_LEFT)) {
		mvmt->strafeLeft((ticks_t)(delta*MOV_FC*PAD_FC), cam->getUPtr());
		movingAround = movingFast = true;
	} else if (inRect(tX, tY, InputMethod::RI_DPAD_RIGHT)) {
		mvmt->strafeRight((ticks_t)(delta*MOV_FC*PAD_FC), cam->getUPtr());
		movingAround = movingFast = true;
	} else if (inRect(tX, tY, InputMethod::RI_DPAD_UP)) {
		mvmt->moveForward((ticks_t)(delta*MOV_FC*PAD_FC));
		movingAround = movingFast = true;
	} else if (inRect(tX, tY, InputMethod::RI_DPAD_DOWN)) {
		mvmt->moveBackward((ticks_t)(delta*MOV_FC*PAD_FC));
		movingAround = movingFast = true;
	} else if (inRect(tX, tY, InputMethod::RI_DPAD_CENTER)) {
		// quick tapping toggles flyMode
		if (getTicks() - lastJumpPress < 200)
			mvmt->toggleFlyMode();
		else
			mvmt->jump(0);

		wasReleased = false;
		lastJumpPress = getTicks();
	}

	return inDPADArea(tX, tY);
}

bool InputMethod::processLeftJoystickTouch(int tX, int tY, ticks_t delta) {
	// touch in left joystick (all input methods (except with dpad) the same behaviour)
	if (inRect(tX, tY, InputMethod::RI_LEFT_JOYSTICK) || (movingAround && nearLeftJoystick(tX, tY, 10))) {
		// don't go out of cicle area
		if (tX >= scl(64)) tX = (int)(scl(64));
		if (tY < SCR_H - scl(64)) tY = (int)(SCR_H - scl(64));

		lastLeftJoystickTouch[0] = tX;
		lastLeftJoystickTouch[1] = tY;

		movingFast = MAX(ABS((lJoyCtrX - tX)), ABS((lJoyCtrY - tY))) > 5.0f * scaleFactor;
		movingAround = true;

		if (tX < lJoyCtrX) {
			mvmt->strafeLeft((ticks_t)(delta*(lJoyCtrX - tX)*MOV_FC*c), cam->getUPtr());
		} else {
			mvmt->strafeRight((ticks_t)(delta*(tX - lJoyCtrX)*MOV_FC*c), cam->getUPtr());
		}

		if (tY < lJoyCtrY) {
			mvmt->moveForward((ticks_t)(delta*(lJoyCtrY - tY)*MOV_FC*c));
		} else {
			mvmt->moveBackward((ticks_t)(delta*(tY - lJoyCtrY)*MOV_FC*c));
		}

		return true;
	}

	return false;
}

void InputMethod::processTouch(int tX, int tY, ticks_t delta) {
	if (!wasReleased) return;

	if (lscene->texSelOverlayActive) {
		if (!lscene->texSelOverlayFinished) {
			texSelectClickTouch(lscene, hudRenderer, tX, tY);
		}
		return;
	}

	// jump button (all input methods)
	if (inRect(tX, tY, InputMethod::RI_JUMP_BTN) && !movingAround && !lookingAround) {
		// quick tapping toggles flyMode
		if (getTicks() - lastJumpPress < 200)
			mvmt->toggleFlyMode();
		else
			mvmt->jump(0);

		wasReleased = false;
		lastJumpPress = getTicks();
		return;
	}

	// left joystick/dpad movement
	if (procMove(tX, tY, delta)) return;

	// right joystick / dig button / onscreen rotation depending on method
	if (procRot(tX, tY)) return;

	if (procMenuTexBtn(tX, tY)) return;

	// Every other touch position try to dig/put.
	procDig(tX, tY);
}

bool InputMethod::processDigButton(int tX, int tY) {
	// dig or put btn (only in default&opposed input method)
	if (inRect(tX, tY, InputMethod::RI_DIG_BTN) && wasReleased) {
		lscene->digMode = !lscene->digMode;
		wasReleased = false;
		return true;
	}

	return false;
}

bool InputMethod::processRightJoystickTouch(int tX, int tY) {
	// touch in right joystick (only in default&opposed. different pos in opposed).
	if (inRect(tX, tY, InputMethod::RI_RIGHT_JOYSTICK) || (lookingAround && nearRightJoystick(tX/*, tY, 10*/))) {
		// don't actually leave joystick (not faster than max speed)
		if (tX < leftBorder) tX = leftBorder;
		else if (tX > rightBorder) tX = rightBorder;
		if (tY < upperBorder) tY = upperBorder;
		else if (tY > lowerBorder) tY = lowerBorder;

		lastRightJoystickTouch[0] = tX;
		lastRightJoystickTouch[1] = tY;

		lookingAround = true;

		if (tX < rJoyCtrX)
			mvmt->rotateLeft((ticks_t)((rJoyCtrX - tX)*ROT_FC*c));
		else
			mvmt->rotateRight((ticks_t)((tX - rJoyCtrX)*ROT_FC*c));

		if (tY < rJoyCtrY)
			mvmt->rotateUp((ticks_t)((rJoyCtrY - tY)*ROT_FC*c));
		else
			mvmt->rotateDown((ticks_t)((tY - rJoyCtrY)*ROT_FC*c));

		return true;
	}

	return false;
}

void InputMethod::processNoTouch() {
	if (lscene->texSelOverlayFinished)
		lscene->texSelOverlayActive = false;

	resetJoystickPos();

	if (procRelease())
		lscene->highlightSelBlock = false;
	lscene->setSxSy(-1, -1);
	wasReleased = true;
	lookingAround = false;
}

//==============================================================================

bool AltIM::procRelease() {
	if (tapStarted && getTicks() - lastRipoffDig > SHORT_TAP_DUR) {
		if (getTicks() - tapStarted < SHORT_TAP_DUR) { // short tap
			lscene->digMode = false;
			if (dragDist < MAX_DRAG_DIST) {
				lscene->doDigOrPut();
			}
		}
	}
	tapStarted = 0;
	hudRenderer->setDrawFeedbackCircles(false);
	return true;
}

bool AltIM::procMove(int tX, int tY, ticks_t delta) {
	if (!dpad) return processLeftJoystickTouch(tX, tY, delta);
	else return processLeftDPadTouch(tX, tY, delta);
}

bool AltIM::procRot(int tX, int tY) {
	return processRipoffBehaviour(tX, tY, scaleFactor, c);
}

void AltIM::procDig(int tX, int tY) {
	if (inRect(tX, tY, InputMethod::RI_LEFT_JOYSTICK) || inRect(tX, tY, InputMethod::RI_RIGHT_JOYSTICK))
		return;
	lscene->setSxSy(tX, tY);
}

bool AltIM::processRipoffBehaviour(int tX, int tY, float scaleFactor, float c) {
	static const float k = 8.0f;

	if (inRect(tX, tY, InputMethod::RI_LEFT_JOYSTICK) || (inButtonArea(tX, tY) && !lookingAround))
		return false;

	lookingAround = true;

	if (!tapStarted) {
		// tap starts
		tapStarted = getTicks();
		tapX = tX;
		tapY = tY;
		dragDist = 0;
		return true;
	} else {
		// dragging

		// limit drag delta, don't rotate if over it
		if ((tX > tapX && tX - tapX > MAX_DRAG_DELTA) || (tX < tapX && tapX - tX > MAX_DRAG_DELTA))
			tX = tapX;
		if ((tY > tapY && tY - tapY > MAX_DRAG_DELTA) || (tY < tapY && tapY - tY > MAX_DRAG_DELTA))
			tY = tapY;

		if (tX > tapX) { mvmt->rotateRight((ticks_t)(ROT_FC*c*(tX - tapX)*k)); dragDist += tX - tapX;}
		else if (tX < tapX) { mvmt->rotateLeft((ticks_t)(ROT_FC*c*(tapX - tX)*k)); dragDist += tapX - tX; }
		if (tY > tapY) { mvmt->rotateDown((ticks_t)(ROT_FC*c*(tY - tapY)*k)); dragDist += tY - tapY; }
		else if (tY < tapY) { mvmt->rotateUp((ticks_t)(ROT_FC*c*(tapY - tY)*k)); dragDist += tapY - tY; }

		if (dragDist < MAX_DRAG_DIST  && !movingAround) {
			lscene->setSxSy(tX, tY);
		} else {
			lscene->highlightSelBlock = false;
			lscene->setSxSy(-1, -1);
		}

		if (getTicks() - tapStarted >= SHORT_TAP_DUR && lscene->getRenderer()->getSelectedBlock() && !movingFast) {
			ticks_t dt = getTicks() - (tapStarted + SHORT_TAP_DUR);
			float ratio = (float)dt / (float)(LONG_TAP_DUR - SHORT_TAP_DUR);
			hudRenderer->setDrawFeedbackCircles(true, (float)tX, (float)tY, ratio);
			if (ratio >= 1.0f) {
				lscene->digMode = true;
				if (dragDist < MAX_DRAG_DIST) {
					lscene->doDigOrPut();
				}
				lscene->highlightSelBlock = false;
				lscene->setSxSy(-1, -1);
				hudRenderer->setDrawFeedbackCircles(false);
				lastRipoffDig = getTicks();
				tapStarted = 0;
			} else {
				if (dragDist >= MAX_DRAG_DIST) {
					hudRenderer->setDrawFeedbackCircles(false);
					lscene->highlightSelBlock = false;
					lscene->setSxSy(-1, -1);
				}
			}
		}

		tapX = tX;
		tapY = tY;
		return true;
	}

	return false;
}

//================================================================================
bool PcIM::procRelease() {
	return false;
}

bool PcIM::procMove(int tX, int tY, ticks_t delta) {
	return processLeftJoystickTouch(tX, tY, delta);
}

bool PcIM::procRot(int tX, int tY) {
	return processRightJoystickTouch(tX, tY)
		   || processDigButton(tX, tY);
}

void PcIM::procDig(int tX, int tY) {
	if (wasReleased) {
		lscene->doDigOrPut();
		wasReleased = false;
	}
}

//================================================================================
bool DefaultIM::procRelease() {
	if (!lscene->texSelOverlayActive
			&& lscene->highlightSelBlock) {
		lscene->doDigOrPut();
	}
	return true;
}

// TODO: Merge somehow with PcIM::procRot.
bool DefaultIM::procMove(int tX, int tY, ticks_t delta) {
	return processLeftJoystickTouch(tX, tY, delta);
}
bool DefaultIM::procRot(int tX, int tY) {
	return processRightJoystickTouch(tX, tY)
		   || processDigButton(tX, tY);
}

void DefaultIM::procDig(int tX, int tY) {
	if (inRect(tX, tY, InputMethod::RI_LEFT_JOYSTICK) || inRect(tX, tY, InputMethod::RI_RIGHT_JOYSTICK)
			|| lookingAround || movingAround) {
		lscene->setSxSy(-1, -1);
		return;
	}

	lscene->setSxSy(tX, tY);
}


}
