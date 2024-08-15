// MenuState.hpp

#ifndef MENU_STATE_HPP
#define MENU_STATE_HPP

#include <list>

#include "../Constants.h"

#include "../Framework/State.hpp"
#include "../Framework/SpriteBatch.hpp"
#include "../Framework/Camera.hpp"

#include "../Terrain.hpp"

namespace as {
//===========================================================================
// Constants
//===========================================================================
#if LITE // prevent lite users from using up too much memory
const int MAX_NUM_WORLDS = 20;
#else
const int MAX_NUM_WORLDS = 64;
#endif

#define SCL_FAC			(SCR_H / 240.0f)
#define BUTTON_WIDTH	(150 * SCL_FAC)
#define BUTTON_HEIGHT	(38 * SCL_FAC)

//===========================================================================
// Types
//===========================================================================

class StateManager;

class MenuState : public State {
public:
	explicit MenuState(StateManager *g, bool forceShowLoadBtn = false);
	virtual ~MenuState();
	virtual void persist() {}

	virtual void processKeyboardInput(bool *keys, SDLMod mod, ticks_t delta);
	virtual void processMouseInput(int dX, int dY, int wheel, MouseButtons *mb, ticks_t delta);
	virtual void processTouch(int tX, int tY, ticks_t delta);

	virtual void processNoTouch();
	virtual void draw(ticks_t delta);

private:
	void addButton(int x, int y, const char *text, const char *action,
				   int w = (int)BUTTON_WIDTH, int h = (int)BUTTON_HEIGHT);

	void initMainScreen(bool forceShowLoadBtn = false);
	void initNewWorldScreen();
	void initSettingsScreen();
	void initLoadWorldScreen();
	void initConnectScreen();
	void initMemorySettingsScreen();
	void initBuyScreen();
	void initTexMapSelectionScreen();

	void processMainScreen(std::string &action);
	void processNewWorldScreen(std::string &action);
	void processSettingsScreen(std::string &action);
	void processConnectScreen(int tX, int tY, std::string &action);
	void processLoadWorldScreen(std::string &action);
	void processMemorySettingsScreen(std::string &action);
	void processBuyScreen(std::string &action);
	void processTexMapSelectionScreen(std::string &action);

	void setupScreen(const char *title, int row, int col);
	void updateSelectedWorldDisp();

	void selectPrevWorld();
	void selectNextWorld();

	void addConnectToSvMsg(const char *norStr);

	void constructWorldFilename();
	
	typedef enum menu_screen_enum {
		MSCREEN_MAIN = 0,
		MSCREEN_NEW_WORLD,
		MSCREEN_LOAD_WORLD,
		MSCREEN_SETTINGS,
		MSCREEN_CONNECT,
		MSCREEN_MEMORY_SETTINGS,
		MSCREEN_BUY,
		MSCREEN_TEXMAP_SELECTION
	} menu_screen_t;
	
	StateManager *g;
	FontSpriteCache *sb, *asb;

	bool touchWasReleased;

	menu_screen_t curScreen;

	int selectedWorld;
	int newWorldScheduled, loadWorldScheduled, connectScheduled;
	Terrain::TerrainSource tsource;

	bool asServer;
	char selIPStr[BUF_LEN];
	int selectedDigit;

	float alpha;

	char worldFilename[BUF_LEN];

	menu_screen_t prevScreen;
	
	OrthoCamera cam;
};

extern int determineNumWorlds();
extern int determineNextFreeSlot();

}

#endif
