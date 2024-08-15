// VisibleFaces.hpp

#ifndef VISIBLE_FACES_HPP
#define VISIBLE_FACES_HPP

namespace as {

enum CubeFace {
	CF_FRONT = 0,
	CF_BACK,
	CF_LEFT,
	CF_RIGHT,
	CF_BOTTOM,
	CF_TOP
};

class VisibleFaces {
public:
	bool front, back, bottom, top, left, right;

	VisibleFaces(bool _front, bool _back, bool _bottom, bool _top, bool _left, bool _right);
	VisibleFaces();

	bool allInvisible() const;
	int numVisibleFaces() const;
};

inline VisibleFaces::VisibleFaces(bool _front, bool _back, bool _bottom, bool _top, bool _left, bool _right)
: front(_front), back(_back), bottom(_bottom), top(_top), left(_left), right(_right) {}

inline VisibleFaces::VisibleFaces()
: front(false), back(false), bottom(false), top(false), left(false), right(false) {}

inline bool VisibleFaces::allInvisible() const {
	return !(left || right || bottom || top || front || back);
}

inline int VisibleFaces::numVisibleFaces() const {
	int nvisFaces = 0;
	if (left) nvisFaces++;
	if (right) nvisFaces++;
	if (bottom) nvisFaces++;
	if (top) nvisFaces++;
	if (front) nvisFaces++;
	if (back) nvisFaces++;
	return nvisFaces;
}

}

#endif // VISIBLE_FACES_HPP
