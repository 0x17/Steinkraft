// SplashState.cpp



#include "../Framework/Utilities.hpp"
#include "../Framework/SpriteBatch.hpp"
#include "../Framework/Platforms/Desktop.hpp"

#include "SplashState.hpp"
#include "MenuState.hpp"

#include "../Framework/IndexedMesh.hpp"

namespace as {
//===========================================================================
// Methods
//===========================================================================

inline void SplashState::toMenu() {
	glDisable(GL_BLEND);
	playSound(SND_DIG);
	g->setState(new MenuState(g));
}

IndexedMesh *imesh;

SplashState::SplashState(StateManager *_g)
:	showingSpl(false),
	startTicks(0),
	g(_g),
	sb(new FontSpriteCache())
{
	cam.apply();
	
	((FontSpriteCache *)sb)->addText(0, 0, AUTHOR_NAME " proudly presents", 1.0f, true);

	glClearColor(0, 0, 0, 1);
	glEnable(GL_BLEND);

	playSound(SND_DIG);

	/*imesh = new IndexedMesh(ComponentInfo(true, true, false), true);
	const float TXFACTOR = TEX_COORD_FACTOR;
	float coords[] = {
		0.0f, 0.0f, 0.0f,	0.0f*TXFACTOR, 0.0f*TXFACTOR,
		100.0f, 0.0f, 0.0f,	1.0f*TXFACTOR, 0.0f*TXFACTOR,
		100.0f, 100.0f, 0.0f,	1.0f*TXFACTOR, 1.0f*TXFACTOR,
		0.0f, 100.0f, 0.0f,	0.0f*TXFACTOR, 1.0f*TXFACTOR,};
	ushort indices[] = {0,1,2,2,3,0};
	imesh->setVertices(coords, 4*5, indices, 6);*/
}

SplashState::~SplashState() {
	//SAFE_DELETE(imesh);
	SAFE_DELETE(sb);
}

void SplashState::processKeyboardInput(bool *keys, SDLMod mod, ticks_t delta)  {
#if SDL || MAC
	if (keys[KEY_RETURN]) {
		toMenu();
	}
#endif
}

void SplashState::processMouseInput(int dX, int dY, int wheel, MouseButtons *mb, ticks_t delta)  {
#if SDL || MAC
	if (mb->lmb) {
		toMenu();
	}
#endif
}

void SplashState::processTouch(int tX, int tY, ticks_t delta)  {
	if (tX != -1 && tY != -1) {
		toMenu();
	}
}

void SplashState::processNoTouch()  {
}

void SplashState::draw(ticks_t delta)  {
	if (!startTicks) startTicks = getTicks();

	if (getTicks() - startTicks > 2000 && !showingSpl) {
		sb->clear();

		for (int x = 0; x < SCR_W; x += 32) {
			for (int y = 0; y < SCR_H; y += 32) {
				sb->addSpr(new Sprite(x, y, 32, 32, 48.0f / 256.0f, 64.0f / 256.0f, 32.0f / 256.0f, 48.0f / 256.0f));
			}
		}

		((FontSpriteCache *)sb)->addText(0, 0, BASE_CAPTION, 4.0f, true);
		((FontSpriteCache *)sb)->addText(SCR_W / 2 - 80, SCR_H / 2 - 50,
										 (curLang == LANG_ENG) ? "Tap to start" : "Tippe zum Starten");

		showingSpl = true;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	sb->render();
	//imesh->render();
}

}
