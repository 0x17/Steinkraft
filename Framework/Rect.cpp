// Rect.cpp

#include "StdAfx.h"
#pragma hdrstop

#include "Rect.hpp"

namespace as {

Rect::Rect()
: x(0), y(0), w(0), h(0)
{}

Rect::Rect(int _x, int _y, int _w, int _h)
: x(_x), y(_y), w(_w), h(_h)
{}

Rect::Rect( float _x, float _y, float _w, float _h )
: x((int)_x), y((int)_y), w((int)_w), h((int)_h)
{}

}

