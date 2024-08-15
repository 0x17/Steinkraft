#ifndef TNT_MANAGER_HPP
#define TNT_MANAGER_HPP

#include "../Rendering/LandscapeRenderer.hpp"
#include <list>

namespace as {

class TNTEntity : public BlockPos {
public:
	ticks_t timeTriggered;

	TNTEntity(int x, int y, int z)
			: BlockPos(x, y, z), timeTriggered(getTicks()) {}

	bool operator==(const TNTEntity &other) const;
};

class TNTManager {
public:
	TNTManager(LandscapeRenderer *_renderer, Terrain *_terrain);
	void update();
	void fireAt(int x, int y, int z);
	
	enum TextureIndices {
		FIRE_TEX_INDEX = 45,
		TNT_TEX_INDEX = 46
	};

private:
	void explodePos(int a, int b, int c, int y);
	void explodeAt(int x, int y, int z);
	bool alreadyStored(TNTEntity *tnt);
	
	LandscapeRenderer *renderer;
	Terrain *terrain;
	std::list<TNTEntity> trigtnts;
	ticks_t lastExplTime;
};

inline bool TNTEntity::operator==(const TNTEntity &other) const {
	return x == other.x && y == other.y && z == other.z;
}

}

#endif // TNT_MANAGER_HPP
