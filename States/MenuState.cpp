// MenuState.cpp



#include <cstdlib>
#include <cmath>
#include <ctime>

#include <map>

#include "../Framework/Utilities.hpp"
#include "../Framework/SpriteBatch.hpp"
#include "../Framework/Platforms/Desktop.hpp"

#include "../Terrain.hpp"
#include "../TranslatedStrs.hpp"
#include "../Managers/NetManager.hpp"

#include "MenuState.hpp"
#include "LandscapeScene.hpp"

#include "../RandomTexts.hpp"

namespace as {

bool buyIntent = false; // defined in Constants.h

//===========================================================================
// Global methods
//===========================================================================

/**
 Select the text string in the correct language for a given category
 with given index.
*/
const char *getText(int labelId) {
	return curLang == LANG_ENG ? labelsEng[labelId] : labelsGer[labelId];
}

//===========================================================================
// Constants
//===========================================================================

const char * const SETTINGS_FILENAME = "general.settings";
const char * const TOGGLES_FILENAME = "toggles.settings";
const char * const TEX_TOGGLE_FILENAME = "textoggle.settings";
const char * const TEXMAP_SELECTION_FILENAME = "texmapselection.settings";

//===========================================================================
// Methods
//===========================================================================

inline void MenuState::constructWorldFilename() {
	std::sprintf(worldFilename, "World%d.dump", selectedWorld);
}

MenuState::MenuState(StateManager *_g, bool forceShowLoadBtn)
:	g(_g),
	sb(new FontSpriteCache()),
	asb(new FontSpriteCache()),

	touchWasReleased(false),

	curScreen(MSCREEN_MAIN),
	selectedWorld(1),

	newWorldScheduled(0),
	loadWorldScheduled(0),
	connectScheduled(0),

	asServer(false),
	selectedDigit(-1),

	alpha(0.0f)
{
	/*if(fileExists(TEX_TOGGLE_FILENAME)) {
		binaryRead(TEX_TOGGLE_FILENAME, &classicTexture, sizeof(bool));
		toggleTexture(classicTexture ? 1 : 0);
	}*/
	
	glClearColor(0, 0, 0, 1);
	cam.apply();	
	glEnable(GL_BLEND);
	startMouseMode();

	if(fileExists(TEXMAP_SELECTION_FILENAME)) {
		binaryRead(TEXMAP_SELECTION_FILENAME, &activeTexMap, sizeof(int));
		toggleTexture(activeTexMap);
		classicTexture = (activeTexMap == TEXMAP_CLASSIC);
	} else {
		initTexMapSelectionScreen();
		return;
	}

	initMainScreen(forceShowLoadBtn);

	if(fileExists(SETTINGS_FILENAME)) {
		settings_t settings;
		binaryRead(SETTINGS_FILENAME, &settings, sizeof(int)*2);
		visualDetail = (DetailLevel)settings.visualDetail;
		activeInputMethod = (InputMethodType)settings.inputMethod;
	}
	
	if(fileExists(TOGGLES_FILENAME)) {
		toggles_t toggles;
		binaryRead(TOGGLES_FILENAME, &toggles, sizeof(bool)*3);
		noNight = toggles.noNight;
		noAnimals = toggles.noAnimals;
		noSound = toggles.noSound;
	}
}

MenuState::~MenuState() {
	SAFE_DELETE(asb);
	SAFE_DELETE(sb);
}

void MenuState::setupScreen(const char *title, int row, int col) {
	sb->clear();
	asb->clear();

	if(activeTexMap == TEXMAP_SPACE) {
		row = 7;
		col = 4;
	}

	for (int x = 0; x < SCR_W; x += 32) {
		for (int y = 0; y < SCR_H; y += 32) {
			sb->addSpr(new Sprite(x, y, 32, 32, (col*16.0f) / 256.0f, ((col + 1)*16.0f) / 256.0f,
				(row*16.0f) / 256.0f, ((row + 1)*16.0f) / 256.0f));
		}
	}
	
	/*sb->addSpr(new Sprite(0, 0, SCR_W, SCR_H, (col*16.0f) / 256.0f, ((col + 1)*16.0f) / 256.0f,
				(row*16.0f) / 256.0f, ((row + 1)*16.0f) / 256.0f));*/

	((FontSpriteCache *)sb)->addText(SCR_W / 2 - 6 * (int)strlen(title), SCR_H - 20, title);
}

void MenuState::initMainScreen(bool forceShowLoadBtn) {
	setupScreen(getText(LBL_MAIN_MENU), 0, 1);

	sb->addText(0, 60, "Copyright");
	sb->addText(0, 40, "2017");
	sb->addText(0, 20, "Andre");
	sb->addText(0, 0, "Schnabel");

#ifdef LITE
	sb->addText(SCR_W - 55, 20, "LITE");
#endif

	sb->addText(SCR_W - 40, 0, "V" VERSION_STR);

	bool noWorldsAvailable = (determineNumWorlds() == 0);

	static const char * const actions[] = { "newWorld", "loadWorld", "settings", "quit" };

	for (int k = 0; k < 4; k++) {
#if IPHONE
		if (k == 3) break; // don't show quit button on IOS devices
#endif

		// hide load btn if no world to load exists
		if (noWorldsAvailable && k == 1 && !forceShowLoadBtn) continue;

		if (k == 0) { // new world btn has server button on right
			addButton(
				(int)((SCR_W - BUTTON_WIDTH) / 2.0f + BUTTON_WIDTH - 40.0f * SCL_FAC),
				(int)(SCR_H - BUTTON_HEIGHT * 2.0f - k * BUTTON_HEIGHT),
				(scrH <= 320) ? "SV" : "Serv",
				"newWorldServer", (int)(40.0f * SCL_FAC));

			addButton(
				(int)((SCR_W - BUTTON_WIDTH) / 2.0f),
				(int)(SCR_H - BUTTON_HEIGHT*2 - k*BUTTON_HEIGHT),
				getText(k),
				actions[k], (int)(BUTTON_WIDTH - 40.0f*SCL_FAC));
		} else {
			addButton((int)((SCR_W - BUTTON_WIDTH) / 2.0f),
					  (int)(SCR_H - BUTTON_HEIGHT * 2.0f - k * BUTTON_HEIGHT),
					  getText(k), actions[k]);
		}
	}

	srand((uint)std::time(NULL));
	asb->addText(0, 0, randTxts[rand() % NUM_RAND_TXTS]);

	addButton(10, 96, getText(LBL_CONNECT), "connect", (int)(70.0f * SCL_FAC));

	curScreen = MSCREEN_MAIN;
}

void MenuState::initNewWorldScreen() {
	setupScreen(getText(LBL_NEW_WORLD), 6, 4);

	static const char * const actions[] = {
		"perlinNoise", "random", "flat", "sphere", "pyramid"
	};

	for (int k = 0; k < 5; k++) {
		addButton(
			(int)((SCR_W - BUTTON_WIDTH) / 2.0f),
			(int)(SCR_H - BUTTON_HEIGHT*2 - k*BUTTON_HEIGHT),
			getText(LBL_TERRAIN + k), actions[k]);
	}
	
	// back from new world button
	addButton((int)(SCR_W - 80*SCL_FAC),
			  (int)(SCR_H - BUTTON_HEIGHT),
			  getText(LBL_BACK),
			  "back", (int)(BUTTON_WIDTH / 2.0f));

	curScreen = MSCREEN_NEW_WORLD;
}

void MenuState::initSettingsScreen() {
	setupScreen(getText(LBL_SETTINGS), 0, 2);

	char strBuf[BUF_LEN];

	static const char * const actions[] = {
		"veryLow", "low", "medium", "high", "veryHigh"
	};

	sb->addText((int)((SCR_W - BUTTON_WIDTH) / 2.0f), SCR_H - 32, getText(LBL_VISUAL_DETAIL));

	// visual detail settings buttons
	for (int k = 0; k < ((SCR_H >= 320) ? 5 : 4); k++) {
		int x = (int)((SCR_W - BUTTON_WIDTH) / 2.0f);
		int y = (int)(SCR_H - BUTTON_HEIGHT * 2 - k * BUTTON_HEIGHT);

		strcpy(strBuf, getText(LBL_VERY_LOW + k));

		if ((visualDetail == DETAIL_VERY_LOW && k == 0)
				|| (visualDetail == DETAIL_LOW && k == 1)
				|| (visualDetail == DETAIL_MEDIUM && k == 2)
				|| (visualDetail == DETAIL_HIGH && k == 3)
				|| (visualDetail == DETAIL_VERY_HIGH && k == 4)) {
			sb->addSpr(new Sprite(x - 10, y, (int)BUTTON_WIDTH + 20, (int)BUTTON_HEIGHT,
								  0.0f / ACT_TEX_SIZE, 64.0f / ACT_TEX_SIZE,
								  160.0f / ACT_TEX_SIZE, 176.0f / ACT_TEX_SIZE));
			strcat(strBuf, " (X)");
		}

		addButton(x, y, strBuf, actions[k]);
	}

#if MOBILE || MOBILE_TEST
	sb->addText(32, SCR_H - 32, getText(LBL_INPUT));

	static const char * const imActions[NUM_INPUT_METHODS] = {
		"defaultIM", "opposedIM", "ripoffIM", "pcIM"
	};

	// input method buttons
	for (int k = 0; k < NUM_INPUT_METHODS; k++) {
		int x = (int)(10 * SCL_FAC);
		int y = (int)(SCR_H - BUTTON_HEIGHT * (k + 2));

		strcpy(strBuf, getText(LBL_DEF + k));

		if ((activeInputMethod == IM_DEFAULT && k == 0)
				|| (activeInputMethod == IM_OPPOSED && k == 1)
				|| (activeInputMethod == IM_RIPOFF && k == 2)
				|| (activeInputMethod == IM_RIPOFF_DPAD && k == 2)
				|| (activeInputMethod == IM_PC && k == 3)) {
			sb->addSpr(new Sprite(x - 10, y, (int)(BUTTON_WIDTH / 2.0f + 20), (int)BUTTON_HEIGHT,
								  0.0f / 256.0f, 64.0f / 256.0f, 160.0f / 256.0f, 176.0f / 256.0f));
			strcat(strBuf, " x");
			if (activeInputMethod == IM_RIPOFF_DPAD)
				strcat(strBuf, "PAD");
		}

		addButton(x, y, strBuf, imActions[k], (int)(BUTTON_WIDTH / 2.0f));
	}
#endif

	// back from settings button
	addButton((int)(SCR_W - 80*SCL_FAC),
			  (int)(SCR_H - BUTTON_HEIGHT),
			  getText(LBL_BACK),
			  "back", (int)(BUTTON_WIDTH / 2.0f));

	curScreen = MSCREEN_SETTINGS;
	
	// TOGGLES		
	// night button toggle
	strcpy(strBuf, "night");
	if(!noNight) strcat(strBuf, " x");
	addButton((int)(SCR_W - 80*SCL_FAC),
			  (int)(SCR_H - BUTTON_HEIGHT*3),
			  strBuf, "night", (int)(BUTTON_WIDTH / 2.0f));
	// animals button toggle
	strcpy(strBuf, "animals");
	if(!noAnimals) strcat(strBuf, " x");
	addButton((int)(SCR_W - 80*SCL_FAC),
			  (int)(SCR_H - BUTTON_HEIGHT*4),
			  strBuf, "animals", (int)(BUTTON_WIDTH / 2.0f));
	// sound button toggle
	strcpy(strBuf, "sound");
	if(!noSound) strcat(strBuf, " x");
	addButton((int)(SCR_W - 80*SCL_FAC),
			  (int)(SCR_H - BUTTON_HEIGHT*5),
			  strBuf, "sound", (int)(BUTTON_WIDTH / 2.0f));
	// classic texture button toggle
	strcpy(strBuf, "Texture");
	//if(classicTexture) strcat(strBuf, " x");
	addButton((int)(SCR_W - 80*SCL_FAC),
			  (int)(SCR_H - BUTTON_HEIGHT*6),
			  strBuf, "texmap", (int)(BUTTON_WIDTH / 2.0f));
}

int determineNumWorlds() {
	int i = 0, numWorlds = 0;
	char strBuf[BUF_LEN];

	while (i < MAX_NUM_WORLDS) {
		std::sprintf(strBuf, "World%d.dump", i + 1);
		if (fileExists(strBuf)) numWorlds++;
		i++;
	}

	return numWorlds;
}

int determineNextFreeSlot() {
	char strBuf[BUF_LEN];
	for (int i = 0; i < MAX_NUM_WORLDS; i++) {
		std::sprintf(strBuf, "World%d.dump", i + 1);
		if (!fileExists(strBuf)) return i + 1;
	}
	return MAX_NUM_WORLDS;
}

void MenuState::initConnectScreen() {
	char drawnIPStr[BUF_LEN];

	setupScreen(getText(LBL_CONNECT), 2, 2);

	if (selectedDigit == -1) {
		std::sprintf(selIPStr, "192168000001");
		selectedDigit = 0;
	}

	int k = 0, i = 0, l = 0;
	for (; i < selectedDigit; i++) {
		if (k % 3 == 0)
			drawnIPStr[l++] = '.';
		drawnIPStr[l++] = selIPStr[k++];
	}
	drawnIPStr[l++] = 'I';
	drawnIPStr[l++] = selIPStr[k++];
	drawnIPStr[l++] = 'I';

	while (selIPStr[k-1]) {
		if (k % 3 == 0)
			drawnIPStr[l++] = '.';
		drawnIPStr[l++] = selIPStr[k++];
	}

	addButton((int)((SCR_W - BUTTON_WIDTH) / 2.0f), 10, getText(LBL_CONN_TO_SV), "doConnect");
	sb->addText(SCR_W / 2, SCR_H / 2, drawnIPStr, 2.0f, true);

	sb->addSpr(new Sprite((int)((SCR_W - 128.0f * SCL_FAC) / 2.0f), // x
						  (int)(BUTTON_HEIGHT + 10.0f * SCL_FAC), // y
						  (int)(128.0f * SCL_FAC), // w
						  (int)(32.0f * SCL_FAC), // h
						  0, 64.0f / 256.0f, // u(min/max)
						  176.0f / 256.0f, (176.0f + 16.0f) / 256.0f)); // v(min/max)

	addButton(10, 10, (curLang == LANG_ENG) ? "Back" : "Menu", "backFromConnect", (int)(60.0f * SCL_FAC));

	curScreen = MSCREEN_CONNECT;
}

void MenuState::updateSelectedWorldDisp() {
	char worldStr[BUF_LEN];
	if (determineNumWorlds() == 0)
		strcpy(worldStr, "Empty");
	else
		std::sprintf(worldStr, "World%d", selectedWorld);

	asb->clear();
	asb->addText((int)(SCR_W / 2.0f - 80), (int)(SCR_H - 38), worldStr);
}

void MenuState::selectPrevWorld() {
	int lastWorld = selectedWorld;
	selectedWorld--;
	constructWorldFilename();
	while (!fileExists(worldFilename)) {
		std::sprintf(worldFilename, "World%d.dump", --selectedWorld);
		if (selectedWorld < 1) {
			selectedWorld = lastWorld;
			return;
		}
	}
}

void MenuState::selectNextWorld() {
	int lastWorld = selectedWorld;
	selectedWorld++;
	constructWorldFilename();
	while (!fileExists(worldFilename)) {
		std::sprintf(worldFilename, "World%d.dump", ++selectedWorld);
		if (selectedWorld > MAX_NUM_WORLDS) {
			selectedWorld = lastWorld;
			return;
		}
	}
}

void MenuState::initLoadWorldScreen() {
	setupScreen(getText(LBL_LOAD_WORLD), 3, 1);

	static const char * const actions[] = {
		"previousWorld", "nextWorld",
		"loadSelWorld", "delSelWorld",
		"backToMain"
	};

	for (int k = 0; k < 5; k++) {
		int x = (int)((SCR_W - BUTTON_WIDTH) / 2.0f);
		int y = (int)(SCR_H - BUTTON_HEIGHT - 40 - k * BUTTON_HEIGHT);

		// only show previous button if there's a previous world
		if (k == 0) {
			int oldSelWorld = selectedWorld;
			selectPrevWorld();
			if (selectedWorld != oldSelWorld) {
				selectNextWorld();
			} else continue;
		}
		// only show next button if there's a next world
		else if (k == 1) {
			int oldSelWorld = selectedWorld;
			selectNextWorld();
			if (selectedWorld != oldSelWorld) {
				selectPrevWorld();
			} else continue;
		}

		if (k == 2) { // load sel world has also server button
			// FIXME: See NetManager.cpp
			addButton((int)(x + BUTTON_WIDTH - (40.0f * SCL_FAC)), y,
					  (scrH <= 320) ? "SV" : "Serv",
					  "loadSelWorldServer", (int)(40.0f * SCL_FAC));

			addButton(x, y,
					  getText(LBL_PREVIOUS + k),
					  actions[k], (int)(BUTTON_WIDTH - 40.0f * SCL_FAC));
		} else {
			addButton(x, y, getText(LBL_PREVIOUS + k), actions[k]);
		}
	}

	updateSelectedWorldDisp();

	curScreen = MSCREEN_LOAD_WORLD;
}

void MenuState::processKeyboardInput(bool *keys, SDLMod mod, ticks_t delta) {
#if SDL
	if (keys[KEY_SPACE]) {
		glDisable(GL_BLEND);
		g->setState(new LandscapeScene((int)std::time(NULL), g, Terrain::TS_PERLIN));
	}
#endif
}

void  MenuState::processMouseInput(int dX, int dY, int wheel,
								   MouseButtons *mb, ticks_t delta)  {
#if SDL || MAC
	if (mb->lmb) {
		int mouseX;
		int mouseY;
#if SDL
		SDL_GetMouseState(&mouseX, &mouseY);
#else
		OSX_GetMousePos(&mouseX, &mouseY);
#endif
		processTouch(mouseX, mouseY, delta);
	} else {
		touchWasReleased = true;
	}
#endif
}

void MenuState::processTouch(int tX, int tY, ticks_t delta) {
	if (!touchWasReleased || tX == -1 || tY == -1) return;

	touchWasReleased = false;

	std::string action = sb->nameOfPointed(tX, tY);

	if(action != "")
		playSound(SND_FALL);

	if (curScreen == MSCREEN_MAIN) {
		processMainScreen(action);
	} else if (curScreen == MSCREEN_NEW_WORLD) {
		processNewWorldScreen(action);
	} else if (curScreen == MSCREEN_SETTINGS) {
		processSettingsScreen(action);
	} else if (curScreen == MSCREEN_CONNECT) {
		processConnectScreen(tX, tY, action);
	} else if (curScreen == MSCREEN_LOAD_WORLD) {
		processLoadWorldScreen(action);
	} else if(curScreen == MSCREEN_MEMORY_SETTINGS) {
		processMemorySettingsScreen(action);
	} else if(curScreen == MSCREEN_BUY) {
		processBuyScreen(action);
	} else if(curScreen == MSCREEN_TEXMAP_SELECTION) {
		processTexMapSelectionScreen(action);
	}
}

void MenuState::processNoTouch() {
	touchWasReleased = true;
}

inline void MenuState::addConnectToSvMsg(const char *norStr) {
	char ipStr[BUF_LEN], localIPStr[BUF_LEN];
#if !NO_NET
	NetManager::strToLocalIP(localIPStr);
#endif
	std::sprintf(ipStr, "Connect client to %s", localIPStr);
	sb->addText(0, 0, asServer ? ipStr : norStr, 1, true);
}

void MenuState::draw(ticks_t delta)  {
	if (newWorldScheduled == 1) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		sb->clear();
		addConnectToSvMsg("Generating new world");
		sb->render();
		newWorldScheduled = 2;
		return;
	} else if (newWorldScheduled == 2) {
		stopMouseMode();
		glDisable(GL_BLEND);
		g->setState(new LandscapeScene((int)std::time(NULL), g, tsource, asServer, asServer));

		return;
	}

	if (loadWorldScheduled == 1) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		sb->clear();
		addConnectToSvMsg("Loading world");
		sb->render();
		loadWorldScheduled = 2;
		return;
	} else if (loadWorldScheduled == 2) {
		glDisable(GL_BLEND);
		stopMouseMode();
		{
			constructWorldFilename();
			g->setState(new LandscapeScene(worldFilename, g, asServer, asServer));
		}
		return;
	}

	if (connectScheduled == 1) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		sb->clear();
		sb->addText(0, 0, "Connecting to server", 1, true);
		sb->render();
		connectScheduled = 2;
		return;
	} else if (connectScheduled == 2) {
		glDisable(GL_BLEND);
		stopMouseMode();
		g->setState(new LandscapeScene(0, g, /* See NetManager.cpp */ Terrain::TS_EMPTY, true, false));
		return;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	sb->render();

	if (curScreen == MSCREEN_MAIN) {
		glPushMatrix();
		glTranslatef(10.0f, SCR_H - 70.0f, 0.0f);
		glRotatef(15.0f, 0.0f, 0.0f, 1.0f);
		alpha += PI / 40;
		float sc = (sinf(alpha) * 0.125f + 0.75f);
		glScalef(sc, sc, sc);
		asb->render();
		glPopMatrix();
	} else if (curScreen == MSCREEN_LOAD_WORLD) {
		asb->render();
	}

	//orthoEnd();
}

void MenuState::addButton(int x, int y, const char *text, const char *action, int w, int h) {
	sb->addSpr(new Sprite(x, y, w, h, TXCRDS(0.0f, 64.0f), TXCRDS(160.0f, 176.0f)), action);
	sb->addText(x + 10 * (int)SCL_FAC, y + 10 * (int)SCL_FAC, text);
}

void MenuState::processMainScreen(std::string &action) {
	if (action == "connect") {
		//initConnectScreen();
		//connectScheduled = 1;
		prevScreen = MSCREEN_MAIN;
		initMemorySettingsScreen();
	}
	else if (action == "newWorldServer") { asServer = true; initNewWorldScreen(); }
	else if (action == "newWorld") { initNewWorldScreen(); }
	else if (action == "loadWorld") {
		#if IPHONE && LITE
		initBuyScreen();
		#else
		initLoadWorldScreen();
		#endif
	} else if (action == "settings") { initSettingsScreen(); }
	else if (action == "quit") { quitGame(); }
}

void MenuState::processNewWorldScreen(std::string &action) {
	if (action == "perlinNoise") tsource = Terrain::TS_PERLIN;
	else if (action == "random") tsource = Terrain::TS_RANDOM;
	else if (action == "flat") tsource = Terrain::TS_FLAT;
	else if (action == "sphere") tsource = Terrain::TS_SPHERE;
	else if (action == "pyramid") tsource = Terrain::TS_PYRAMID;
	else if (action == "back") {
		initMainScreen();
		return;
	}
	else return;

	// survival not supported in MP yet!
	/*if (asServer || tsource != Terrain::TS_PERLIN || NO_SURVIVAL) {
		newWorldScheduled = 1;
		survival = false;
	} else
		initNewWorldModeScreen();*/

	prevScreen = MSCREEN_NEW_WORLD;
	initMemorySettingsScreen();
}

void MenuState::processSettingsScreen(std::string &action) {
	// View distance / Visual detail
	if (action == "veryLow") visualDetail = DETAIL_VERY_LOW;
	else if (action == "low") visualDetail = DETAIL_LOW;
	else if (action == "medium") visualDetail = DETAIL_MEDIUM;
	else if (action == "high") visualDetail = DETAIL_HIGH;
	else if (action == "veryHigh") visualDetail = DETAIL_VERY_HIGH;
	// Input methods
	else if (action == "defaultIM") activeInputMethod = IM_DEFAULT;
	else if (action == "opposedIM") activeInputMethod = IM_OPPOSED;
	else if (action == "ripoffIM") {
		// already selected? toggle dpad/vjoystick
		activeInputMethod = (activeInputMethod == IM_RIPOFF) ? IM_RIPOFF_DPAD : IM_RIPOFF;
	} else if (action == "pcIM") activeInputMethod = IM_PC;
	// Toggles
	else if (action == "night") noNight = !noNight;
	else if(action == "animals") noAnimals = !noAnimals;
	else if(action == "sound") noSound = !noSound;
	else if(action == "texmap") {
		initTexMapSelectionScreen();
		return;
	}
	// Back button
	else if (action == "back") {
		// Save current settings
		settings_t settings;
		settings.inputMethod = activeInputMethod;
		settings.visualDetail = visualDetail;
		binaryWrite(SETTINGS_FILENAME, &settings, sizeof(int)*2);
		
		// Save current toggles
		toggles_t toggles;
		toggles.noNight = noNight;
		toggles.noAnimals = noAnimals;
		toggles.noSound = noSound;
		binaryWrite(TOGGLES_FILENAME, &toggles, sizeof(bool)*3);
		
		//binaryWrite(TEX_TOGGLE_FILENAME, &classicTexture, sizeof(bool));
		
		initMainScreen();
		return;
	}

	initSettingsScreen();
}

void MenuState::processConnectScreen(int tX, int tY, std::string &action) {
	if (tX >= (SCR_W - 128*SCL_FAC) / 2
			&& tX <= (SCR_W + 128*SCL_FAC) / 2
			&& tY <= SCR_H - (BUTTON_HEIGHT + 10*SCL_FAC)
			&& tY >= SCR_H - (BUTTON_HEIGHT + 42*SCL_FAC))
	{
		int col = (int)((tX - (SCR_W - 128 * SCL_FAC) / 2) / (32 * SCL_FAC));

		switch (col) {
		case 0: // left arrow
			selectedDigit = (selectedDigit - 1 >= 0) ? selectedDigit - 1 : 11;
			break;
		case 1: // minus sign
			selIPStr[selectedDigit]--;
			if (selIPStr[selectedDigit] < '0')
				selIPStr[selectedDigit] = '9';
			break;
		case 2: // plus sign
			selIPStr[selectedDigit]++;
			if (selIPStr[selectedDigit] > '9')
				selIPStr[selectedDigit] = '0';
			break;
		case 3: // right arrow
			selectedDigit = (selectedDigit + 1 <= 11) ? selectedDigit + 1 : 0;
			break;
		}

		initConnectScreen();
	}

	if (action == "doConnect") {
		// convert sel ip string to right format (no leading zeroes, dots)
		char a[4], b[4], c[4], d[4];
		int e, f, _g, h;
		e = f = _g = h = 0;
		bool okA, okB, okC, okD;
		okA = okB = okC = okD = false;
		for (int i = 0; i < 3; i++) {
			if (selIPStr[i] != '0' || okA) {
				a[e++] = selIPStr[i];
				okA = true;
			}
			if (selIPStr[i+3] != '0' || okB) {
				b[f++] = selIPStr[i+3];
				okB = true;
			}
			if (selIPStr[i+6] != '0' || okC) {
				c[_g++] = selIPStr[i+6];
				okC = true;
			}
			if (selIPStr[i+9] != '0' || okD) {
				d[h++] = selIPStr[i+9];
				okD = true;
			}
		}
		a[e] = b[f] = c[_g] = d[h] = '\0';
		if (a[0] == '\0') { a[0] = '0'; a[1] = '\0'; }
		if (b[0] == '\0') { b[0] = '0'; b[1] = '\0'; }
		if (c[0] == '\0') { c[0] = '0'; c[1] = '\0'; }
		if (d[0] == '\0') { d[0] = '0'; d[1] = '\0'; }
		std::sprintf(remoteIPStr, "%s.%s.%s.%s", a, b, c, d);
		std::printf("Connecting to: %s\n", remoteIPStr);
		connectScheduled = 1;
	} else if (action == "backFromConnect") {
		initMainScreen();
	}
}

void MenuState::processLoadWorldScreen(std::string &action) {
	if (action == "previousWorld") {
		selectPrevWorld();
		initLoadWorldScreen();
	} else if (action == "nextWorld") {
		selectNextWorld();
		initLoadWorldScreen();
	} else if (action == "loadSelWorldServer") {
		if(determineNumWorlds() == 0) return;
		
		#ifdef LITE
			prevScreen = MSCREEN_LOAD_WORLD;
			initBuyScreen();
			return;
		#endif

		asServer = true;
		prevScreen = MSCREEN_LOAD_WORLD;
		initMemorySettingsScreen();		
		//loadWorldScheduled = 1;
	} else if (action == "loadSelWorld") {
		if (determineNumWorlds() == 0) return;
		
		#ifdef LITE
			prevScreen = MSCREEN_LOAD_WORLD;
			initBuyScreen();
			return;
		#endif

		prevScreen = MSCREEN_LOAD_WORLD;
		initMemorySettingsScreen();
		//loadWorldScheduled = 1;
	} else if (action == "delSelWorld") {
		if(determineNumWorlds() == 0) return;

		constructWorldFilename();
		deleteFile(worldFilename);
		char otherFn[BUF_LEN];
		std::sprintf(otherFn, "%s.spawnpos", worldFilename);
		if (fileExists(otherFn))
			deleteFile(otherFn);

		std::sprintf(otherFn, "%s.entities", worldFilename);
		if (fileExists(otherFn))
			deleteFile(otherFn);
		std::sprintf(otherFn, "%s.edescr", worldFilename);
		if (fileExists(otherFn))
			deleteFile(otherFn);
			
		std::sprintf(otherFn, "%s.animals", worldFilename);
		if (fileExists(otherFn))
			deleteFile(otherFn);
		std::sprintf(otherFn, "%s.adescr", worldFilename);
		if (fileExists(otherFn))
			deleteFile(otherFn);

		if (determineNumWorlds() == 0) {
			initMainScreen();
			return;
		}

		// scan to the left
		do {
			std::sprintf(worldFilename, "World%d.dump", --selectedWorld);
			if (fileExists(worldFilename)) break;
		} while (selectedWorld > 0);

		// nothing found... now scan to the right (since detNumWorlds>0)
		if (selectedWorld == 0) {
			do {
				std::sprintf(worldFilename, "World%d.dump", ++selectedWorld);
				if (fileExists(worldFilename)) break;
			} while (selectedWorld <= MAX_NUM_WORLDS);
		}
		initLoadWorldScreen();
	} else if (action == "backToMain") {
		initMainScreen();
	}
}

void MenuState::initMemorySettingsScreen()
{
	setupScreen("Memory Settings", 2, 5);
	curScreen = MSCREEN_MEMORY_SETTINGS;

	static const char * const actions[] = { "conservative", "performance" };
	static const char * const labels[] = { "Conservative", "Performance" };

	for (int k = 0; k < 2; k++) {
		addButton((int)((SCR_W - BUTTON_WIDTH) / 2.0f),
			(int)(SCR_H - BUTTON_HEIGHT*2 - k*BUTTON_HEIGHT), labels[k], actions[k]);
	}
	
	// back from memory settings button
	addButton((int)(SCR_W - 80*SCL_FAC),
			  (int)(SCR_H - BUTTON_HEIGHT),
			  getText(LBL_BACK),
			  "back", (int)(BUTTON_WIDTH / 2.0f));
}

void MenuState::processMemorySettingsScreen( std::string &action )
{
	if(action == "conservative") {
		keepMeshes = false;
	} else if(action == "performance") {
		keepMeshes = true;
	} else if (action == "back") {
		initMainScreen();
		return;
	} else {
		return;
	}

	switch(prevScreen) {
	case MSCREEN_NEW_WORLD:
		newWorldScheduled = 1;
		break;
	case MSCREEN_LOAD_WORLD:
		loadWorldScheduled = 1;
		break;
	case MSCREEN_MAIN:
		connectScheduled = 1;
		break;
	default:
		break;	
	}
}

void MenuState::initBuyScreen() {
#if IPHONE
	setupScreen("Buy full version to save and load worlds", 3, 1);
#else
	setupScreen("Buy full version to load this world", 3, 1);
#endif
	curScreen = MSCREEN_BUY;
	
	static const char * const actions[] = { "openStore", "back" };
	static const char * const labels[] = { "Open Store", "Back" };
	
	const float topY = SCR_H - BUTTON_HEIGHT*2;

	for (int k = 0; k < 2; k++) {
		addButton((int)((SCR_W - BUTTON_WIDTH) / 2.0f),
			(int)(topY - k*BUTTON_HEIGHT), labels[k], actions[k]);
	}
}

void MenuState::processBuyScreen(std::string &action) {
	if(action == "back") {
		#if IPHONE
		initMainScreen();
		#else
		initLoadWorldScreen();
		#endif
		return;
	} else if(action == "openStore") {
		// TODO set some flag for Android and iOS
		buyIntent = true;
	}
}

void MenuState::initTexMapSelectionScreen() {
	setupScreen("Select Texture Pack", 3, 1);
	curScreen = MSCREEN_TEXMAP_SELECTION;

	static const char * const actions[] = { "classic", "space", "hidef" };
	static const char * const labels[] = { "Classic", "Space", "High Definition" };

	const float topY = SCR_H - BUTTON_HEIGHT*2;

	for(int k=0; k<NUM_TEXMAPS; k++) {
		addButton((int)((SCR_W - BUTTON_WIDTH) / 2.0f),
			(int)(topY - k*BUTTON_HEIGHT), labels[k], actions[k]);
	}
}

void MenuState::processTexMapSelectionScreen(std::string &action) {
	static const char * const actions[] = { "classic", "space", "hidef" };
	for(int i=0; i<NUM_TEXMAPS; i++) {
		if(action == actions[i]) {
			activeTexMap = i;
			classicTexture = (activeTexMap == TEXMAP_CLASSIC);
			toggleTexture(activeTexMap);
			binaryWrite(TEXMAP_SELECTION_FILENAME, &activeTexMap, sizeof(int));
			initMainScreen();			
			return;
		}
	}
}

}
