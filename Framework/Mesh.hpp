// Mesh.hpp

#ifndef MESH_HPP_
#define MESH_HPP_

#include "PGL.h"
#include "VertexStorage.hpp"

namespace as {

class Mesh {
public:
	explicit Mesh(bool isStatic = true);
	explicit Mesh(ComponentInfo compInf, bool isStatic = true);
	virtual ~Mesh();

	void setVertices(const float *coords, int numCoords);

	virtual void render();
	void render(GLenum primitiveType) const;
	void render(int offset, int count) const;
	void render(int offset, int count, GLenum primitiveType) const;

private:
	void selectStorage(ComponentInfo compInf, bool isStatic);

	VertexStorage *vxStorage;
};

} /* namespace as */
#endif /* MESH_HPP_ */
