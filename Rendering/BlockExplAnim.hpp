// BlockExplAnim.hpp

#ifndef BLOCKEXPLANIM_HPP_
#define BLOCKEXPLANIM_HPP_

#include <list>

#include "../Framework/Mesh.hpp"
#include "Animation.hpp"

namespace as {
	
class Terrain;

class BlockExplAnim : public Animation {
public:
	BlockExplAnim(int x, int y, int z, int texId, int blockBelowY, ComponentInfo compInf, Terrain *t);
	virtual ~BlockExplAnim();

	bool render();
	
private:
	float yAnim;
	int x, y, z;
	Mesh mesh;
	int blockBelowY;
};

class BlockExplAnimManager {
public:
	BlockExplAnimManager(Terrain *t);
	virtual ~BlockExplAnimManager();

	void addBlockExplAnim(int x, int y, int z, int texId, int blockBelowY);
	void render();
	
private:
	Terrain *terrain;
	std::list<BlockExplAnim *> anims;
	ComponentInfo compInf;
};

} /* namespace as */
#endif /* BLOCKEXPLANIM_HPP_ */
