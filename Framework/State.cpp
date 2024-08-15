// State.cpp

#include "StdAfx.h"
#pragma hdrstop

#include "State.hpp"

namespace as {
//===========================================================================
// State Manager
//===========================================================================
StateManager::StateManager() : state(NULL) {
	initGL();
}

void StateManager::setState(State *_state) {
	if (this->state) {
		SAFE_DELETE(this->state);
	}
	this->state = _state;
}

StateManager::~StateManager() {
	SAFE_DELETE(state);
}

} // namespace as
