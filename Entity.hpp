// Entity.hpp

#ifndef ENTITY_HPP
#define ENTITY_HPP

namespace as {

class Entity {
public:
	enum EntityTexIndices {
		LADDER_TEX_INDEX = 64,
		TORCH_TEX_INDEX,
		GLASS_TEX_INDEX,
		FLOWER_TEX_INDEX,
		MUSHROOM_TEX_INDEX,
		RAIL_TEX_INDEX,
		DOOR_TEX_INDEX	
	};
	
	enum EntityType {
		LADDER = 0,
		TORCH,
		GLASS,
		FLOWER,
		MUSHROOM,
		RAIL,

		// doors can be either along x- or z-axis.
		DOOR_X,
		DOOR_X_OPEN,
		DOOR_Z,
		DOOR_Z_OPEN
	};
	
	enum Consts {
		NUM_ENTITIES = 7
	};

	BlockPos pos;
	EntityType type;
	CubeFace cface;

	Entity() {}

	Entity(int _x, int _y, int _z, EntityType _type, CubeFace _cface)
	: pos(_x, _y, _z), type(_type), cface(_cface)
	{}

	Entity(const Entity& other)
	: pos(other.pos), type(other.type), cface(other.cface)
	{}

	bool operator==(const Entity& other) const {
		return pos == other.pos && cface == other.cface;
	}
	
	static bool isDoorIndex(int x) {
		return (x == DOOR_X || x == DOOR_Z || x == DOOR_X_OPEN || x == DOOR_Z_OPEN);
	}
};

}

#endif

