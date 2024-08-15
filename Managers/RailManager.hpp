// RailManager.hpp

#ifndef RAILMANAGER_HPP
#define RAILMANAGER_HPP

#include <list>
#include <map>

#include "../Framework/Mesh.hpp"
#include "../Rendering/Meshes/CubeVertices.hpp"
#include "../Terrain.hpp"

namespace as {

struct Cart {
	Vec3 pos;
	float yaw;
	bool active;

	Cart() : pos(Vec3(0,0,0)), yaw(0.0f), active(false) {}
};

class RailManager {
public:
	RailManager(Terrain *t);
	virtual ~RailManager() {}

	void addRail(BlockPos pos);
	bool tryToRemoveRail(BlockPos pos);
	void determineRowColForPos(BlockPos pos, int *row, int *col) const;

	bool isRailAtPos(int x, int y, int z) const;
	bool isRailAtPos(BlockPos pos) const;

	void renderCart();
	void spawnCartAtPos(BlockPos pos);
	bool nearCart(Vec3 pos);
	Vec3 getCartPos() const;

	void moveCartForward(ticks_t delta);
	void moveCartBackward(ticks_t delta);
	Vec3 cartIntertia(ticks_t delta);
	
private:
	enum RailType {
		// Straight
		RT_XDIR,
		RT_ZDIR,
		// Turns
		RT_PX_TO_PZ,
		RT_PX_TO_NZ,
		RT_NX_TO_PZ,
		RT_NX_TO_NZ
	};

	RailManager::RailType determineRailTypeForPos(BlockPos pos) const;
	void initCartMesh();
	void moveCommon(float factor, float delta);

	std::map<BlockPos, RailType> rails;

	Mesh cartMesh;
	Cart cart;

	float xfactor,zfactor;
	float inertia;

	RailType prevRailType;
	RailType prevTurn;
	Vec3 lastValidPos;

	float targetXfactor, targetZfactor;
	Vec3 targetPos;
	bool targetPosSet;
};

}

#endif
