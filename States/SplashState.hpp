// SplashState.hpp

#ifndef SPLASH_STATE_HPP
#define SPLASH_STATE_HPP

#include "../Framework/State.hpp"
#include "../Framework/Camera.hpp"

namespace as {

//===========================================================================
// Types
//===========================================================================
class SpriteCache;
class StateManager;

class SplashState : public State {
public:
	explicit SplashState(StateManager *g);
	virtual ~SplashState();
	virtual void persist() {}

	virtual void processKeyboardInput(bool *keys, SDLMod mod, ticks_t delta);
	virtual void processMouseInput(int dX, int dY, int wheel, MouseButtons *mb, ticks_t delta);
	virtual void processTouch(int tX, int tY, ticks_t delta);
	virtual void processNoTouch();
	virtual void draw(ticks_t delta);

	void toMenu();
	
private:
	bool showingSpl;
	ticks_t startTicks;
	StateManager *g;
	SpriteCache *sb;
	OrthoCamera cam;
};

}

#endif
