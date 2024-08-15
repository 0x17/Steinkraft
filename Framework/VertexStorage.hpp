// VertexStorage.hpp

#ifndef VERTEXSTORAGE_HPP
#define VERTEXSTORAGE_HPP

#include "PGL.h"

namespace as {

class ComponentInfo {
public:
	bool usePos, useTexCoord, useColor;

	ComponentInfo(bool _usePos, bool _useTexCoord, bool _useColor)
	: usePos(_usePos), useTexCoord(_useTexCoord), useColor(_useColor)
	{}

	ComponentInfo()
	: usePos(true), useTexCoord(true), useColor(true)
	{}

	int getCompsPerVx() {
		int result = 0;
		result += (usePos) ? 3 : 0;
		result += (useTexCoord) ? 2 : 0;
		result += (useColor) ? 4 : 0;
		return result;
	}
};

class TexCoordRect {
public:
	float minU, maxU, minV, maxV;

	TexCoordRect(float _minU, float _maxU, float _minV, float _maxV)
	: minU(_minU), maxU(_maxU), minV(_minV), maxV(_maxV)
	{}
};

// Base class for VertexBuffer and VertexArray
class VertexStorage {
public:
	explicit VertexStorage(ComponentInfo comps);
	virtual ~VertexStorage();

	virtual void setData(const float *vxtxnxcl, int numVxTxNxCl) = 0;
	virtual void render(int offset, int count) = 0;
	virtual void render(int offset, int count, GLenum primitiveType) = 0;

	void render();
	void render(GLenum primitiveType);

protected:
	ComponentInfo comps;

	int ndraw;
	int compsPerVx;
	int sizePerVx;
};

inline void VertexStorage::render() {
	render(0, ndraw);
}

inline void VertexStorage::render(GLenum primitiveType) {
	render(0, ndraw, primitiveType);
}

} /* namespace as */

#endif /* VERTEXSTORAGE_HPP */
