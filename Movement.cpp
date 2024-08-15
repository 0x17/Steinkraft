// Movement.cpp



#include "Framework/Math/Vector.hpp"

#include "Managers/RailManager.hpp"

#include "Movement.hpp"

namespace as {

static const float		MOV_SPEED = 0.005f;
static const ticks_t	AUTO_JUMP_DELAY = 200;

const char *soundFilenames[NUM_SOUNDS] = {
	"put.wav",
	"fall.wav",
	"jump.wav",
	"dig.wav",
	"step1.wav",
	"step2.wav",
	"step3.wav",
	"step4.wav",
};

//===========================================================================
// Methods
//===========================================================================

inline Vec3 Movement::genAhead(Vec3 dir, float c) const {
	return cam->getPos() + (dir * c);
}

inline void Movement::restrictMovement() {
	cam->restrictToBox(1, 1, 1, Terrain::MAX_X - 1, Terrain::MAX_Y + 5, Terrain::MAX_Z - 1);
}

inline bool Movement::isLadderAhead(int cx, int cy, int cz) const {
	return terrain->hasLadderOnFace(cx + ladderOffsets[0], cy, cz + ladderOffsets[1], ladderFace);
}

Movement::Movement(Camera *_camera, Terrain *_terrain, RailManager *_rm, bool _flyMode)
:		cam(_camera),
		terrain(_terrain),
		railManager(_rm),
		flyMode(_flyMode),

		falling(false),
		jumping(false),

		velFallY(0.0f),
		accelFallY(0.0f),
		velJumpY(0.0f),
		accelJumpY(0.0f),

		autoJumping(true),
		tryingToAutoJump(false),

		jumpTryStarted(0),

		lastWalkSoundTriggered(0),

		walking(false),
		onLadder(false),

		inCart(false)
{
#if !MOBILE_MODE
	autoJumping = false;
#endif

	oldPos = cam->getPos();
}

Movement::~Movement() {
}

void Movement::mouseLook(int dX, int dY, ticks_t delta) {
	if (dX < 0) {
		cam->rotate(Vec3::yaxis, 0.00025f * ABS(dX) * delta);
	} else if (dX > 0) {
		cam->rotate(Vec3::yaxis, -0.00025f * ABS(dX) * delta);
	}

	Vec3 t = cam->getU();

	if (dY < 0) {
		cam->rotate(t,  0.00025f * ABS(dY) * delta);
	} else if (dY > 0) {
		cam->rotate(t, -0.00025f * ABS(dY) * delta);
	}
}

void Movement::update(ticks_t delta, bool crouching) {
	if(inCart) {
		Vec3 cartPos = railManager->cartIntertia(delta);
		cam->setPos(Vec3(cartPos.x+0.5f, cartPos.y+1.5f, cartPos.z+0.5f));
		return;
	} else if(railManager->nearCart(cam->getPos() + Vec3(0, -1.0f, 0))) {
		enterCart(railManager->getCartPos());
		return;
	}

	if(flyMode) {
		restrictMovement();
		return;
	}

	if(onLadder) return;

	if(!falling && terrain->isEmptyPos(cam->getPos().x, cam->getPos().y - 2, cam->getPos().z)) {
		float cx = cam->getPos().x;
		float cy = cam->getPos().y;
		float cz = cam->getPos().z;

		if (terrain->isEmptyPos(cx, cy - 1, cz) && !crouching) {
			falling = true;
			accelFallY = -0.00030f;
			velFallY = 0;
		}
	}

	if(falling && !jumping)
		updateFall(delta);
	if(jumping)
		updateJump(delta);

	// never fall infinitely down
	restrictMovement();

	if(!jumping && !falling && walking && getTicks() - lastWalkSoundTriggered >= 500) {
		// FIXME: TODO: Find reason for iOS speed problems and replace with better sound!
		//playSound(SND_STEP1 + rand() % 4);
		lastWalkSoundTriggered = getTicks();
	}
	walking = false;
}

bool Movement::resetPosIfCrashed(const Vec3 ahead) {
	static Vec3 aheadBelow;
	aheadBelow = Vec3(ahead.x, ahead.y - 1, ahead.z);
	if (!terrain->isEmptyPos(ahead) || !terrain->isEmptyPos(aheadBelow)) {
		Vec3 *camPos = cam->getPosPtr();

		Vec3 aheadX = Vec3(ahead.x, oldPos.y, oldPos.z);
		Vec3 aheadY = Vec3(oldPos.x, ahead.y, oldPos.z);
		Vec3 aheadZ = Vec3(oldPos.x, oldPos.y, ahead.z);

		Vec3 aheadXBelow = Vec3(ahead.x, oldPos.y - 1, oldPos.z);
		Vec3 aheadYBelow = Vec3(oldPos.x, ahead.y - 1, oldPos.z);
		Vec3 aheadZBelow = Vec3(oldPos.x, oldPos.y - 1, ahead.z);

		if (!terrain->isEmptyPos(aheadX) || !terrain->isEmptyPos(aheadXBelow))
			camPos->x = oldPos.x;
		if (!terrain->isEmptyPos(aheadY) || !terrain->isEmptyPos(aheadYBelow))
			camPos->y = oldPos.y;
		if (!terrain->isEmptyPos(aheadZ) || !terrain->isEmptyPos(aheadZBelow))
			camPos->z = oldPos.z;

		return true;
	}
	return false;
}

bool Movement::resetIfFalling(const Vec3 ahead) {
	if (!flyMode && ahead.y - 1 > 0
			&& terrain->get((int)ahead.x, (int)ahead.y - 1, (int)ahead.z) == 0) {
		cam->setPos(oldPos);
		return true;
	}
	return false;
}

void Movement::jump(bool forceJump) {
	static int cx, cz;
	static float fcy;

	if(inCart) {
		exitCart();
		return;
	}

	if (onLadder && !forceJump) {
		onLadder = false;
		cam->getPosPtr()->x = cam->getPosPtr()->x - ladderOffsets[0] * 0.25f;
		cam->getPosPtr()->z = cam->getPosPtr()->z - ladderOffsets[1] * 0.25f;
	}

	cx = (int)cam->getPosPtr()->x;
	fcy = cam->getPosPtr()->y;
	cz = (int)cam->getPosPtr()->z;

	if (((terrain->dYtoSolidBelow(cx, fcy, cz) < 3.0f || cam->getPos().y < 0.5f) && !jumping) || forceJump) {
		accelJumpY = -0.00010f;
		velJumpY = 0.08f;
		jumping = true;
		falling = true;
		accelFallY = -0.00030f;
		velFallY = 0;
		playSound(SND_JUMP);
	}
}

void Movement::updateFall(ticks_t delta) {
	oldPos = cam->getPos();
	cam->setPos(cam->getPos() + Vec3(0.0f, velFallY*delta*0.05f, 0.0f));
	velFallY += accelFallY * delta * 0.5f;
	if (resetPosIfCrashed(cam->getPos() + Vec3(0.0f, -0.5f, 0.0f))) {
		falling = false;
		accelFallY = velFallY = 0.0f;
		playSound(SND_FALL);
	}
}

void Movement::updateJump(ticks_t delta) {
	oldPos = cam->getPos();
	cam->setPos(cam->getPos() + Vec3(0, velJumpY*delta*0.05f, 0));
	velJumpY += accelJumpY * delta;
	Vec3 ahead = cam->getPos();
	ahead.y = ahead.y + 0.5f;
	if (resetPosIfCrashed(ahead) || velJumpY <= 0) {
		jumping = false;
		accelJumpY = velJumpY = 0;
	}
}

void Movement::climbUp(ticks_t delta) {
	static int cx, cy, cz;
	static Vec3 *cpos = cam->getPosPtr();

	cx = (int)cpos->x;
	cy = (int)cpos->y;
	cz = (int)cpos->z;

	if (!terrain->isEmptyPos(cx, (int)(cam->getPosPtr()->y + 0.25f), cz)) return;

	cam->moveUp(delta*0.0001f);

	if (!isLadderAhead(cx, cy, cz)) {
		onLadder = false;
		jump(true);
	}
}

void Movement::climbDown(ticks_t delta) {
	static int cx, cy, cz;
	static Vec3 *cpos = cam->getPosPtr();

	cx = (int)cpos->x;
	cy = (int)cpos->y;
	cz = (int)cpos->z;

	cam->moveDown(delta * 0.0001f);

	if (!terrain->isEmptyPos(cam->getPos() + Vec3(0, -1.5f, 0.0f)) || !isLadderAhead(cx, cy, cz)) {
		onLadder = false;
		cam->getPosPtr()->x = cx - ladderOffsets[0] * 0.25f;
		cam->getPosPtr()->z = cz - ladderOffsets[1] * 0.25f;
	}
}

void Movement::commonMovement(Vec3 dir, float d, bool crouching, ticks_t delta) {
	oldPos = cam->getPos();
	Vec3 tmp = dir;
	if (!flyMode) tmp.y = 0;
	tmp = tmp.normalized() * (d * MOV_SPEED * delta * 0.05f);
	Vec3 ahead = genAhead(tmp.normalized(), 0.3f);
	cam->setPos(cam->getPos() + (tmp * (crouching ? 0.25f : 1.0f)));

	if (resetPosIfCrashed(ahead)) {
		int ax = (int)ahead.x;
		int ay = (int)ahead.y - 1;
		int az = (int)ahead.z;

		Vec3 *camPos = cam->getPosPtr();

		int cx = (int)camPos->x;
		int cz = (int)camPos->z;

		if (!flyMode && terrain->hasEntities()) {
			// check if box ahead has ladder on it on correct face/side
			// switch to climbLadder=true (forward up/ back down)
			std::list<Entity> eap = terrain->getEntitiesAtPos(ax, ay, az, Entity::LADDER);
			std::list<Entity>::iterator it;
			for (it = eap.begin(); it != eap.end(); ++it) {
				CubeFace fc = (*it).cface;
				if ((fc == CF_LEFT && (ax == cx + 1 && az == cz))
					|| (fc == CF_RIGHT && (ax == cx - 1 && az == cz))
					|| (fc == CF_BACK && (ax == cx && az == cz + 1))
					|| (fc == CF_FRONT && (ax == cx && az == cz - 1)))
				{
					onLadder = true;
					ladderOffsets[0] = ladderOffsets[1] = 0;
					ladderFace = fc;
					switch (fc) {
					case CF_LEFT:
						ladderOffsets[0] = 1;
						break;
					case CF_RIGHT:
						ladderOffsets[0] = -1;
						break;
					case CF_FRONT:
						ladderOffsets[1] = -1;
						break;
					case CF_BACK:
						ladderOffsets[1] = 1;
						break;
					default:
						break;
					}
					return;
				}
			}
		}

		if (!terrain->get(ax, ay + 1, az) && autoJumping && !falling) {
			if (tryingToAutoJump) {
				if (getTicks() - jumpTryStarted > AUTO_JUMP_DELAY) {
					jump();
					tryingToAutoJump = false;
				}
			} else {
				tryingToAutoJump = true;
				jumpTryStarted = getTicks();
			}
		}
	} else walking = true;
}

void Movement::enterCart(Vec3 cartPos)
{
	inCart = true;
	float yaw = cam->getYaw();
	cam->rotate(Vec3(0,1,0), -yaw);
	cam->setPos(Vec3(cartPos.x+0.5f, cartPos.y+1.5f, cartPos.z+0.5f));
}

void Movement::cartForward( ticks_t delta )
{
	railManager->moveCartForward(delta);
}

void Movement::cartBackward( ticks_t delta )
{
	railManager->moveCartBackward(delta);
}

void Movement::exitCart()
{
	inCart = false;
	Vec3 oldCamPos = cam->getPos();

	for(int x = -1; x <= 1; x++) {			
		for(int z = -1; z <= 1; z++) {
			if(x == 0 && z == 0) continue;

			cam->getPosPtr()->x = cam->getPosPtr()->x + x;
			cam->getPosPtr()->z = cam->getPosPtr()->z + z;

			Vec3 newPos = cam->getPos();

			if(terrain->isEmptyPos(newPos) && !railManager->nearCart(newPos)) {
				break;
			} else {
				cam->setPos(oldCamPos);
			}
		}
	}

	return;
}


} /* namespace as */
