// Movement.hpp

#ifndef MOVEMENT_HPP_
#define MOVEMENT_HPP_

#include "Framework/Utilities.hpp"
#include "Framework/Camera.hpp"

#include "Terrain.hpp"

namespace as {

class RailManager;

class Movement {
public:
	Movement(Camera *camera, Terrain *terrain, RailManager *rm, bool flyMode);
	virtual ~Movement();

	void mouseLook(int dX, int dY, ticks_t delta);
	void rotateLeft(ticks_t delta);
	void rotateRight(ticks_t delta);
	void rotateUp(ticks_t delta);
	void rotateDown(ticks_t delta);
	void strafeLeft(ticks_t delta, Vec3 *t, bool crouching = false);
	void strafeRight(ticks_t delta, Vec3 *t, bool crouching = false);
	void moveBackward(ticks_t delta, bool crouching = false);
	void moveForward(ticks_t delta, bool crouching = false);
	void update(ticks_t delta, bool crouching = false);
	void jump(bool forceJump = false);

	void toggleFlyMode();	

private:
	Vec3 genAhead(Vec3 dir, float c) const;
	void commonMovement(Vec3 dir, float d, bool crouching, ticks_t delta);
	bool resetPosIfCrashed(const Vec3 ahead);
	bool resetIfFalling(const Vec3 ahead);
	void updateFall(ticks_t delta);
	void updateJump(ticks_t delta);

	void climbUp(ticks_t delta);
	void climbDown(ticks_t delta);

	void restrictMovement();
	bool isLadderAhead(int cx, int cy, int cz) const;

	void enterCart(Vec3 cartPos);
	void exitCart();
	void cartForward(ticks_t delta);
	void cartBackward(ticks_t delta);
	
	Camera *cam;
	Terrain *terrain;
	RailManager *railManager;
	bool flyMode;

	Vec3 oldPos;

	bool falling, jumping;
	float velFallY, accelFallY;
	float velJumpY, accelJumpY;

	bool autoJumping;
	bool tryingToAutoJump;
	ticks_t jumpTryStarted;

	ticks_t lastWalkSoundTriggered;
	bool walking;

	bool onLadder;
	int ladderOffsets[2];
	CubeFace ladderFace;

	bool inCart;
};

inline void Movement::rotateLeft(ticks_t delta) {
	cam->rotate(Vec3::yaxis, delta * 0.00025f);
}

inline void Movement::rotateRight(ticks_t delta) {
	cam->rotate(Vec3::yaxis, delta * -0.00025f);
}

inline void Movement::rotateUp(ticks_t delta) {
	cam->rotate(cam->getU(), delta * 0.00025f);
}

inline void Movement::rotateDown(ticks_t delta) {
	cam->rotate(cam->getU(), delta * -0.00025f);
}

inline void Movement::strafeLeft(ticks_t delta, Vec3 *t, bool crouching) {
	if (onLadder || inCart) return;

	Vec3 dir = Vec3(t->x, t->y, t->z);
	commonMovement(dir, -1.0f, crouching, delta);
}

inline void Movement::strafeRight(ticks_t delta, Vec3 *t, bool crouching) {
	if (onLadder || inCart) return;

	Vec3 dir = Vec3(t->x, t->y, t->z);
	commonMovement(dir, 1.0f, crouching, delta);
}

inline void Movement::moveBackward(ticks_t delta, bool crouching) {
	if(inCart) { cartBackward(delta); return; }
	if (onLadder) { climbDown(delta); return; }

	Vec3 dir = cam->getNormal();
	commonMovement(dir, -1.0f, crouching, delta);
}

inline void Movement::moveForward(ticks_t delta, bool crouching) {
	if(inCart) { cartForward(delta); return; }
	if (onLadder) { climbUp(delta); return; }

	Vec3 dir = cam->getNormal();
	commonMovement(dir, 1.0f, crouching, delta);
}

inline void Movement::toggleFlyMode() {
	flyMode = !flyMode;
}

} /* namespace as */
#endif /* MOVEMENT_HPP_ */
