// InputMethods.hpp

#ifndef INPUT_METHODS_HPP
#define INPUT_METHODS_HPP

#include "Framework/PGL.h"
#include "Framework/Rect.hpp"

namespace as {

class Movement;
class HudRenderer;
class LandscapeScene;
class Camera;

class InputMethod {
public:
	enum RectangleIndices {
		RI_MENU_BTN,
		RI_JUMP_BTN,
		RI_DIG_BTN,
		RI_TEX_SQUARE,
		RI_LEFT_JOYSTICK,
		RI_RIGHT_JOYSTICK,
		RI_DPAD_LEFT,
		RI_DPAD_RIGHT,
		RI_DPAD_UP,
		RI_DPAD_DOWN,
		RI_DPAD_CENTER,
		NUM_RECT_INDICES
	};

	InputMethod(LandscapeScene *lscene, HudRenderer *hudRenderer, Movement *mvmt);
	virtual ~InputMethod();

	virtual void processTouch(int tX, int tY, ticks_t delta);
	virtual void processNoTouch();

	void resetJoystickPos(bool left = true, bool right = true);

	Rect getRectWithIndex(RectangleIndices index) const;

	static InputMethod *chooseIM(InputMethodType method, LandscapeScene *lscene, HudRenderer *hudRenderer, Movement *mvmt);
	
protected:
	void addRect(RectangleIndices index, Rect rect);
	bool inRect(int x, int y, RectangleIndices index) const;
	void addRectangles();

	bool procMenuTexBtn(int tX, int tY);

	// callbacks
	virtual bool procRelease() = 0;
	virtual bool procMove(int tX, int tY, ticks_t delta) = 0;
	virtual bool procRot(int tX, int tY) = 0;
	virtual void procDig(int tX, int tY) = 0;

	bool processLeftDPadTouch(int tX, int tY, ticks_t delta);
	bool processLeftJoystickTouch(int tX, int tY, ticks_t delta);
	bool processRightJoystickTouch(int tX, int tY);
	bool processDigButton(int tX, int tY);

	// auxiliary
	template <class T> T scl(T x) const;

	void calcRightJoystick();
	bool nearLeftJoystick(int tX, int tY, int off) const;
	bool nearRightJoystick(int tX/*, int tY, int off*/) const;

	bool inButtonArea(int x, int y) const;
	bool inDPADArea(int x, int y) const;
	
	enum Consts {
		PAD_FC = 80,
		MAX_DRAG_DIST = 20
	};
	
	Movement *mvmt;
	HudRenderer *hudRenderer;
	LandscapeScene *lscene;
	Camera *cam;

	bool wasReleased;

	ticks_t lastJumpPress;
	ticks_t lastRipoffDig;

	bool movingFast;
	bool movingAround, lookingAround;

	ticks_t tapStarted;
	int tapX, tapY;
	int dragDist;

	float scaleFactor;
	float c;

	int lJoyCtrX, lJoyCtrY;
	int rJoyCtrX, rJoyCtrY;

	int leftBorder, rightBorder;
	int upperBorder, lowerBorder;

	Rect rectangles[NUM_RECT_INDICES];
};

extern InputMethod *gInputMethod;

template <class T>
inline T InputMethod::scl(T x) const {
	return (T)((float)x * scaleFactor);
}

void finishTexSelection(LandscapeScene *lscene, HudRenderer *hudRenderer);
void texSelectClickTouch(LandscapeScene *lscene, HudRenderer *hudRenderer, int tX, int tY);

class DefaultIM : public InputMethod {
	bool opposed;
public:
	DefaultIM(LandscapeScene *lscene, HudRenderer *hudRenderer, Movement *mvmt, bool _opposed = false)
	: InputMethod(lscene, hudRenderer, mvmt), opposed(_opposed) {}
	virtual ~DefaultIM() {}

	virtual bool procRelease();
	virtual bool procMove(int tX, int tY, ticks_t delta);
	virtual bool procRot(int tX, int tY);
	virtual void procDig(int tX, int tY);
};

class AltIM : public InputMethod {
	bool dpad;
public:
	AltIM(LandscapeScene *lscene, HudRenderer *hudRenderer, Movement *mvmt, bool _dpad)
	: InputMethod(lscene, hudRenderer, mvmt), dpad(_dpad) {}
	virtual ~AltIM() {}

	bool processRipoffBehaviour(int tX, int Y, float scaleFactor, float c);

	virtual bool procRelease();
	virtual bool procMove(int tX, int tY, ticks_t delta);
	virtual bool procRot(int tX, int tY);
	virtual void procDig(int tX, int tY);
};

class PcIM : public InputMethod {
public:
	PcIM(LandscapeScene *lscene, HudRenderer *hudRenderer, Movement *mvmt)
	: InputMethod(lscene, hudRenderer, mvmt) {}
	virtual ~PcIM() {}

	virtual bool procRelease();
	virtual bool procMove(int tX, int tY, ticks_t delta);
	virtual bool procRot(int tX, int tY);
	virtual void procDig(int tX, int tY);
};

}

#endif // INPUT_METHODS_HPP
