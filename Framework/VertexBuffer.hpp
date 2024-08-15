// VertexBuffer.hpp

#ifndef VERTEXBUFFER_HPP_
#define VERTEXBUFFER_HPP_

#include "PGL.h"
#include "VertexStorage.hpp"

namespace as {

class VertexBuffer : public VertexStorage {
public:
	explicit VertexBuffer(ComponentInfo comps, bool isStatic = true);
	virtual ~VertexBuffer();

	virtual void setData(const float *vxtxnxcl, int numVxTxNxCl);
	virtual void render(int offset, int count);
	virtual void render(int offset, int count, GLenum primitiveType);

private:
	bool isStatic;
	GLuint vbo;
	static bool clStates[3];
};

} /* namespace as */
#endif /* VERTEXBUFFER_HPP_ */
