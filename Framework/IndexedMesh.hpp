// IndexedMesh.hpp

#ifndef INDEXED_MESH_HPP
#define INDEXED_MESH_HPP

#include "PGL.h"
#include "VertexStorage.hpp"

namespace as {

class IndexedMesh {
	uint vbo, ibo;
	bool isStatic;
	ComponentInfo comps;
	int compsPerVx;
	int ndraw;
	static bool clStates[3];
	int sizePerVx;

public:
	IndexedMesh(ComponentInfo _comps = ComponentInfo(), bool _isStatic = true);
	virtual ~IndexedMesh();

	void setVertices(const float *coords, int numCoords, const ushort *indices, int numIndices);

	virtual void render();
	void render(GLenum primitiveType) const;
	void render(int offset, int count) const;
	void render(int offset, int count, GLenum primitiveType) const;
};

inline void IndexedMesh::render( int offset, int count ) const
{
	render(offset, count, GL_TRIANGLES);
}

inline void IndexedMesh::render( GLenum primitiveType ) const
{
	render(0, ndraw, primitiveType);
}

inline void IndexedMesh::render()
{
	render(0, ndraw, GL_TRIANGLES);
}

}

#endif
