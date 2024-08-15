// IVoxelRenderer.hpp

#ifndef IVOXELRENDERER_HPP
#define IVOXELRENDERER_HPP

#include "../../BlockPos.hpp"

namespace as {

class IVoxelRenderer {
public:
	virtual ~IVoxelRenderer() {}

	virtual void update(BlockPos *bposChanged) = 0;
	virtual void render() = 0;
};

}

#endif /* IVOXELRENDERER_HPP */
