/*
 * VertexArray.hpp
 *
 *  Created on: Jul 27, 2011
 *      Author: andreschnabel
 */

#ifndef VERTEXARRAY_HPP_
#define VERTEXARRAY_HPP_

#include "PGL.h"
#include "VertexStorage.hpp"

namespace as {

class VertexArray : public VertexStorage {
public:
	explicit VertexArray(ComponentInfo comps);
	virtual ~VertexArray();

	virtual void setData(const float *vxtxnxcl, int numVxTxNxCl);
	virtual void render(int offset, int count);
	virtual void render(int offset, int count, GLenum primitiveType);

private:
	float *vx;
};

} /* namespace as */
#endif /* VERTEXARRAY_HPP_ */
