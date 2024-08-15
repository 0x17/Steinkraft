// VertexStorage.cpp

#include "StdAfx.h"
#pragma hdrstop

#include "VertexStorage.hpp"

namespace as {

VertexStorage::VertexStorage(ComponentInfo _comps) 
:	comps(_comps),
	ndraw(0)
{
	compsPerVx = _comps.getCompsPerVx();
	sizePerVx = compsPerVx * (int)sizeof(float);
}

VertexStorage::~VertexStorage() {
}

} /* namespace as */
