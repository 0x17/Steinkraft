// SpriteBatch.cpp

#include <iostream>



#include "SpriteBatch.hpp"
#include "Utilities.hpp"

namespace as {
//===========================================================================
// Sprite
//===========================================================================
Sprite::Sprite(Rect &rect, TexCoordRect &tcr)
:	x(rect.x),
	y(rect.y),
	w(rect.w),
	h(rect.h),
	minU(tcr.minU),
	maxU(tcr.maxU),
	minV(tcr.minV),
	maxV(tcr.maxV)
{}

//===========================================================================
// SpriteCache
//===========================================================================
SpriteCache::SpriteCache(bool isStatic)
:	Mesh(ComponentInfo(true, true, false), isStatic),
	dirty(false),
	curSize(0)
{
	assert(sprites.size() == 0);
}

SpriteCache::~SpriteCache() {
	clear();
}

void SpriteCache::addSpr(Sprite *spr) {
	sprites.push_back(spr);
	curSize++;
	dirty = true;
}

void SpriteCache::addSpr(Sprite *spr, std::string name) {
    //std::cout << "Inserting sprite \"" << name << "\" at (" << std::to_string(spr->x) << "," << std::to_string(spr->y) << "," << std::to_string(spr->w) << ";" << std::to_string(spr->h) << ")" << std::endl;
	namedSprites.insert(std::pair<std::string, Sprite *>(name, spr));
	addSpr(spr);
}

std::string SpriteCache::nameOfPointed(int px, int py) const {
	std::map<std::string, Sprite *>::const_iterator it;
	for (it = namedSprites.begin(); it != namedSprites.end(); ++it) {
		if ((*it).second->pointInside(px, py)) {
			return (*it).first;
		}
	}
	return std::string("");
}

void SpriteCache::clear() {
	std::list<Sprite *>::iterator it;
	for (it = sprites.begin(); it != sprites.end(); ++it) {
		SAFE_DELETE((*it));
	}

	sprites.clear();
	namedSprites.clear();
	curSize = 0;
	dirty = true;
}

void SpriteCache::render() {
	// setup mesh vertices from sprites
	if (dirty) {
		int numCoords = curSize * 6 * (3 + 2);
		float *coords = new float[numCoords];

		int k = 0;
		float xs[6], ys[6], us[6], vs[6];
		Sprite *spr;

		std::list<Sprite *>::iterator it;
		for (it = sprites.begin(); it != sprites.end(); ++it) {
			spr = (*it);

			xs[0] = xs[1] = xs[5] = (float)spr->x;
			us[0] = us[1] = us[5] = spr->minU;

			xs[2] = xs[3] = xs[4] = (float)(spr->x + spr->w);
			us[2] = us[3] = us[4] = spr->maxU;

			ys[0] = ys[4] = ys[5] = (float)(spr->y + spr->h);
			vs[0] = vs[4] = vs[5] = spr->minV;

			ys[1] = ys[2] = ys[3] = (float)spr->y;
			vs[1] = vs[2] = vs[3] = spr->maxV;

			for (int i = 0; i < 6; i++) {
				coords[k++] = xs[i]; // x
				coords[k++] = ys[i]; // y
				coords[k++] = 0; // z
				coords[k++] = us[i]; // u
				coords[k++] = vs[i]; // v
			}
		}

		assert(k == numCoords);

		setVertices(coords, numCoords);
		SAFE_DELETE_ARRAY(coords);
		dirty = false;
	}

	Mesh::render();
}

//===========================================================================
// FontSpriteCache
//===========================================================================
FontSpriteCache::FontSpriteCache(bool isStatic) : SpriteCache(isStatic) {}

void FontSpriteCache::addText(int x, int y, const char *str, float scale, bool centered) {
	ulong l = static_cast<ulong>(strlen(str));
	char c;
	int xnum, ynum;
	float u, v;
	int xOff = (centered) ? (int)((SCR_W - scale * 11 * l) / 2.0f) : x;

	// for each character add a apt glyph sprite
	for (ulong i = 0; i < l; i++) {
		c = str[i];

		if (c >= 'a' && c <= 'm') {
			ynum = 0;
			xnum = c - 'a';
		} else if (c >= 'A' && c <= 'M') {
			ynum = 0;
			xnum = c - 'A';
		} else if (c >= 'n' && c <= 'z') {
			ynum = 1;
			xnum = c - 'n';
		} else if (c >= 'N' && c <= 'Z') {
			ynum = 1;
			xnum = c - 'N';
		} else if (c >= '0' && c <= '9') {
			ynum = 2;
			xnum = c - '0';
		} else {
			xOff += 11 * (int)scale;
			continue;
		}

		u = (191.0f + (xnum * 5.0f)) / ACT_TEX_SIZE;
		v = (ynum * 5.0f) / 256.0f;

		Sprite *spr = new Sprite(xOff, (centered) ? (int)((SCR_H - scale * 10) / 2.0f) : y,
								 10 * (int)scale, 10 * (int)scale,
								 u, u + 5.0f / ACT_TEX_SIZE,
								 v, v + 5.0f / ACT_TEX_SIZE);

		addSpr(spr);

		xOff += 11 * (int)scale;
	}
}

//===========================================================================
// SpriteBatch
//===========================================================================
void SpriteBatch::draw( TexCoordRect &tcr, float x, float y )
{
	int w = (int)((tcr.maxU - tcr.minU) / (float)TEXMAP_SIZE);
	int h = (int)((tcr.maxV - tcr.minV) / (float)TEXMAP_SIZE);
	addSpr(new Sprite((int)x, (int)y, (int)w, (int)h, tcr));
}

void SpriteBatch::render()
{
	SpriteCache::render();
	clear();
}

}
