// BlockPos.hpp

#ifndef BLOCK_POS_HPP
#define BLOCK_POS_HPP

#include "Framework/Math/Vector.hpp"

#include <string>

namespace as {
/**
 Stores the position of a cubic unit block (cube) in 3D space.
 Coordinates are in integer precision.
*/
class BlockPos {
public:
	int x, y, z;

	BlockPos();
	BlockPos(const Vec3 &v);
	BlockPos(int _x, int _y, int _z);
	explicit BlockPos(BlockPos *block);

	bool operator==(const BlockPos& o) const;
	bool operator<(const BlockPos& o) const;
	BlockPos operator+(BlockPos const& o) const;

	std::string toString() const;

private:
	int getDataIndex() const;
};

inline bool BlockPos::operator==(const BlockPos& o) const {
	return x == o.x && y == o.y && z == o.z;
}

inline bool BlockPos::operator<(const BlockPos& o) const {
	return this->getDataIndex() < o.getDataIndex();
}

inline BlockPos BlockPos::operator+(BlockPos const& o) const {
	return BlockPos(x+o.x, y+o.y, z+o.z);
}

}

#endif
