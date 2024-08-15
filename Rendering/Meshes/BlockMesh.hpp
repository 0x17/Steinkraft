// BlockMesh.hpp

#ifndef BLOCKMESH_HPP_
#define BLOCKMESH_HPP_

#include "../../Framework/Mesh.hpp"
#include "CubeVertices.hpp"

namespace as {

class BlockMesh : public Mesh {
public:
	BlockMesh();
	explicit BlockMesh(TexCoordRect *tcr);
	BlockMesh(int texRow, int texCol, float bness = 1.0f);
	BlockMesh(int texRow, int texCol, float alpha, CubeFace face, float bness = 1.0f);
	explicit BlockMesh(float alpha);
	virtual ~BlockMesh();

private:
	void update(TexCoordRect *tcr, float alpha, CubeFace *face = NULL);
	void update(TexCoordRect *tcr);
	
	float bness;
};

} /* namespace as */
#endif /* BLOCKMESH_HPP_ */
