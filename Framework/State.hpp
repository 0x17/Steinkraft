// State.hpp

#ifndef STATE_H
#define STATE_H

#include "PGL.h"
#include "Utilities.hpp"
#include "Singleton.hpp"

namespace as {

class MouseButtons {
public:
	bool lmb, mmb, rmb;

	MouseButtons(bool _lmb, bool _mmb, bool _rmb)
	: lmb(_lmb), mmb(_mmb), rmb(_rmb)
	{}

	MouseButtons()
	: lmb(false), mmb(false), rmb(false)
	{}
};

//===========================================================================
// State
//===========================================================================
class State {
public:
	explicit State() {}
	virtual ~State() {}
	virtual void persist() = 0;

	virtual void processKeyboardInput(bool *keys, SDLMod mod, ticks_t delta) = 0;
	virtual void processMouseInput(int dX, int dY, int wheel, MouseButtons *mb, ticks_t delta) = 0;
	virtual void processTouch(int tX, int tY, ticks_t delta) = 0;
	virtual void processNoTouch() = 0;
	virtual void draw(ticks_t delta) = 0;
};

//===========================================================================
// State Manager
//===========================================================================
class StateManager : public Singleton<StateManager> {
	friend class Singleton<StateManager>;
	State *state;
	
protected:
	StateManager();
	
public:
	~StateManager();
	void setState(State *_state);
	State* getState();
};

inline State* StateManager::getState() {
	return state;
}

} // namespace as

#endif
