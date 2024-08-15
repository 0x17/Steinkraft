// BlockPos.cpp



#include "BlockPos.hpp"
#include "Terrain.hpp"

namespace as {

BlockPos::BlockPos() : x(0), y(0), z(0) {}
BlockPos::BlockPos( const Vec3 &v ) : x((int)v.x), y((int)v.y), z((int)v.z) {}
BlockPos::BlockPos( int _x, int _y, int _z ) : x(_x), y(_y), z(_z) {}
BlockPos::BlockPos( BlockPos *block ) : x(block->x), y(block->y), z(block->z) {}

int BlockPos::getDataIndex() const {
	return x*(Terrain::MAX_Y*Terrain::MAX_Z)+y*Terrain::MAX_Z+z;
}

std::string BlockPos::toString() const {
	char buf[256];
	sprintf(buf, "x=%d, y=%d, z=%d", x, y, z);
	return std::string(buf);
}

}
