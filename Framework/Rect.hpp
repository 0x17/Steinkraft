// Rect.hpp

#ifndef RECT_HPP
#define RECT_HPP

namespace as {

class Rect {
public:
	int x, y, w, h;

	Rect();
	Rect(float x, float y, float w, float h);
	Rect(int x, int y, int w, int h);

	bool contains(int px, int py) const;
};

inline bool Rect::contains(int px, int py) const {
	return px >= x && px <= x + w && py >= y && py <= y + h;
}

class Pos {
public:
	int x, y;
	
	Pos() { x=y=0; }
	Pos(int _x, int _y) : x(_x), y(_y) {}
};

}

#endif // RECT_HPP
