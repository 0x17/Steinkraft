// Animation.hpp

#ifndef ANIMATION_HPP_
#define ANIMATION_HPP_

#include "../Framework/PGL.h"

namespace as {

class Animation {
public:
	enum Consts {
		INFINITE_DURATION = 0
	};

	const ticks_t duration;
	ticks_t lticks;
	bool running;
	float counter;

	explicit Animation(ticks_t duration);
	virtual ~Animation() {}
	void reset();
	void start();
	bool update();
};

/**
 Constructs new Animation object.
*/
inline Animation::Animation(ticks_t _duration)
:	duration(_duration)
{
	reset();
}

/**
 Reset animation state (e.g. for restarting it).
*/
inline void Animation::reset() {
	lticks = getTicks();
	running = false;
	counter = 0.0f;
}

/**
 Wait... if reset is called before start isn't lticks=...,counter=0 redundant?
*/
inline void Animation::start() {
	running = true;
	lticks = getTicks();
	counter = 0.0f;
}

inline bool Animation::update() {
	if (!running) return false;

	if (duration == INFINITE_DURATION || getTicks() - lticks <= duration) {
		counter = (getTicks() - lticks) * 0.006f;
		return true;
	} else {
		reset();
		return false;
	}
	return false;
}

}

#endif /* ANIMATION_HPP_ */
