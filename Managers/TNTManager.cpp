// TNTManager.cpp



#include <cstdlib>
#include "TNTManager.hpp"

namespace as {

const ticks_t	TRIGGER_TO_FIRE_DELAY = 1000;
const ticks_t	TIME_BETWEEN_EXPL = 500;

const int EXPL_RADIUS = 4;

void TNTManager::explodePos(int a, int b, int c, int y) {
	static int texIndex;

	texIndex = terrain->getValid(a, b, c);
	if (!texIndex || b == 0) return;

	if (texIndex != TNT_TEX_INDEX + 1) {
		terrain->lazySet(a, b, c, 0);
		// FIXME: Fix staying ind mid air and not vanishing particles bug!
		renderer->addExplAt(a, b, c, texIndex, y - EXPL_RADIUS);
		terrain->removeEntityAt(a, b, c, (CubeFace)23);
	} else {
		fireAt(a, b, c);
	}
}

void TNTManager::explodeAt(int x, int y, int z) {
	for (int a = x - EXPL_RADIUS + 1; a <= x + EXPL_RADIUS - 1; a++) {
		for (int b = y - EXPL_RADIUS + 1; b <= y + EXPL_RADIUS - 1; b++) {
			for (int c = z - EXPL_RADIUS + 1; c <= z + EXPL_RADIUS - 1; c++) {
				if (a == x && b == y && c == z) continue;
				if (rand() % 4 != 0)
					explodePos(a, b, c, y);
			}
		}
	}
}

bool TNTManager::alreadyStored(TNTEntity *tnt) {
	std::list<TNTEntity>::iterator it;
	for (it = trigtnts.begin(); it != trigtnts.end(); ++it) {
		if ((*it) == (*tnt)) return true;
	}
	return false;
}

TNTManager::TNTManager(LandscapeRenderer *_renderer, Terrain *_terrain)
		: renderer(_renderer), terrain(_terrain), lastExplTime(0) {}

void TNTManager::update() {
	std::list<TNTEntity>::iterator it = trigtnts.begin();
	while (it != trigtnts.end()) {
		TNTEntity tnt = (*it);
		if (getTicks() - lastExplTime > TIME_BETWEEN_EXPL
				&& getTicks() - tnt.timeTriggered > TRIGGER_TO_FIRE_DELAY) {
			int tx = tnt.x, ty = tnt.y, tz = tnt.z;
			terrain->lazySet(tx, ty, tz, 0);
			renderer->addExplAt(tx, ty, tz, TNT_TEX_INDEX, ty - EXPL_RADIUS);
			terrain->removeEntityAt(tx, ty, tz, (CubeFace)23);
			playSound(SND_DIG);
			lastExplTime = getTicks();
			trigtnts.erase(it++);
			explodeAt(tx, ty, tz);
		} else
			++it;
	}

	terrain->lazySetFlush();
}

void TNTManager::fireAt(int x, int y, int z) {
	for (int a = x - 1; a <= x + 1; a++) {
		for (int b = y - 1; b <= y + 1; b++) {
			for (int c = z - 1; c <= z + 1; c++) {
				if (terrain->getValid(a, b, c) == TNT_TEX_INDEX + 1) {
					TNTEntity tnt(a, b, c);
					tnt.timeTriggered = getTicks();
					if (!alreadyStored(&tnt))
						trigtnts.push_back(tnt);
				}
			}
		}
	}
}

}
