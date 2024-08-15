// QuadMesh.hpp

#ifndef QUADMESH_HPP_
#define QUADMESH_HPP_

#include "../../Framework/Mesh.hpp"

namespace as {

class Rect;

class QuadMesh : public Mesh {
public:
	QuadMesh(int texRow, int texCol, int x, int y, int w, int h);
	QuadMesh(const TexCoordRect tcr, int x, int y, int w, int h, float alpha = 1.0f);
	explicit QuadMesh(const TexCoordRect tcr, const Rect rect, float alpha = 1.0f);
	QuadMesh(float minU, float maxU, float minV, float maxV, int x, int y, int w, int h);
	QuadMesh(float minU, float maxU, float minV, float maxV, int x, int y, int w, int h, float alpha);
	virtual ~QuadMesh();

	void update(float minU, float maxU, float minV, float maxV, int x, int y, int w, int h, float alpha = 1.0f);
};

} /* namespace as */
#endif /* QUADMESH_HPP_ */
