#ifndef DESKTOP_HPP
#define DESKTOP_HPP

#include "../PGL.h"

namespace as {

typedef enum keys_s {
	KEY_LEFT = 0,
	KEY_RIGHT,
	KEY_UP,
	KEY_DOWN,
	KEY_A,
	KEY_D,
	KEY_W,
	KEY_S,
	KEY_P,
	KEY_O,
	KEY_F,
	KEY_BACKSPACE,
	KEY_RETURN,
	KEY_SPACE
} key_t;

const int NUM_KEYS = KEY_SPACE + 1;

#if SDL
class LibSdl {
public:
	LibSdl();
	~LibSdl();	

	void mainLoop();

private:
	void processInput(ticks_t delta);
	void display(ticks_t delta);
};

#endif // SDL

}

#endif // DESKTOP_HPP
