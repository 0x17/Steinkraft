// SpriteBatch.hpp

#ifndef SPRITE_BATCH_HPP
#define SPRITE_BATCH_HPP

#include <list>
#include <map>
#include <string>

#include "Mesh.hpp"
#include "Rect.hpp"

namespace as {
//===========================================================================
// Sprite
//===========================================================================
class Sprite {
public:
	int x, y, w, h;
	float minU, maxU, minV, maxV;

	Sprite(int _x, int _y, int _w, int _h, float _minU, float _maxU, float _minV, float _maxV);
	Sprite(int _x, int _y, int _w, int _h, TexCoordRect &tcr);
	Sprite(Rect &rect, TexCoordRect &tcr);
	Sprite();

	bool pointInside(int px, int py) const;
};

inline Sprite::Sprite(int _x, int _y, int _w, int _h, float _minU, float _maxU, float _minV, float _maxV)
	: x(_x), y(_y), w(_w), h(_h), minU(_minU), maxU(_maxU), minV(_minV), maxV(_maxV)
{}

inline Sprite::Sprite(int _x, int _y, int _w, int _h, TexCoordRect &tcr)
	:	x(_x), y(_y), w(_w), h(_h),
	minU(tcr.minU / ACT_TEX_SIZE),
	maxU(tcr.maxU / ACT_TEX_SIZE),
	minV(tcr.minV / ACT_TEX_SIZE),
	maxV(tcr.maxV / ACT_TEX_SIZE)
{}

inline Sprite::Sprite()
	: x(0), y(0), w(0), h(0), minU(0), maxU(0), minV(0), maxV(0)
{}

inline bool Sprite::pointInside(int px, int py) const {
	py = SCR_H - py;
	return px >= x && px <= x + w && py >= y && py <= y + h;
}

//===========================================================================
// SpriteCache
//===========================================================================
class SpriteCache : public Mesh {
public:
	explicit SpriteCache(bool isStatic = true);
	virtual ~SpriteCache();

	void addSpr(Sprite *spr);
	void addSpr(Sprite *spr, std::string name);

	bool pointInSprite(int px, int py, std::string name) const;
	std::string nameOfPointed(int px, int py) const;

	void clear();
	virtual void render();
	
private:
	std::list<Sprite *> sprites;
	std::map<std::string, Sprite *> namedSprites;
	bool dirty;
	int curSize;
};

inline bool SpriteCache::pointInSprite(int px, int py, std::string name) const {
	return namedSprites.find(name)->second->pointInside(px, py);
}

//===========================================================================
// FontSpriteCache
//===========================================================================
class FontSpriteCache : public SpriteCache {
public:
	FontSpriteCache(bool isStatic = true);

	void addText(int x, int y, const char *str,
				 float scale = 1.0f, bool centered = false);
};

//===========================================================================
// SpriteBatch
//===========================================================================
class SpriteBatch : public SpriteCache {
public:
	SpriteBatch() : SpriteCache(false) {}
	virtual ~SpriteBatch() {}

	void draw(TexCoordRect &tcr, float x, float y);
	virtual void render();
};

}

#endif
