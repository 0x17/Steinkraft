// TranslatedStrs.hpp

#ifndef TRANSLATEDSTRS_HPP
#define TRANSLATEDSTRS_HPP

#include "States/MenuState.hpp"

namespace as {
//===========================================================================
// Constants
//===========================================================================
enum LabelId {
	LBL_NEW_WORLD = 0,
	LBL_LOAD_WORLD,
	LBL_SETTINGS,
	LBL_QUIT,

	LBL_TERRAIN,
	LBL_RANDOM,
	LBL_FLAT,
	LBL_SPHERE,
	LBL_PYRAMID,

	LBL_CREATIVE,
	LBL_SURVIVAL,

	LBL_VERY_LOW,
	LBL_LOW,
	LBL_MEDIUM,
	LBL_HIGH,
	LBL_VERY_HIGH,

	LBL_PREVIOUS,
	LBL_NEXT,
	LBL_LOAD_SEL,
	LBL_DELETE_SEL,
	LBL_BACK,

	LBL_DEF,
	LBL_OPP,
	LBL_ALT,
	LBL_PC,

	LBL_MAIN_MENU,
	LBL_VISUAL_DETAIL,
	LBL_CONN_TO_SV,
	LBL_GAME_MODE,

	LBL_INPUT,
	LBL_JOIN,
	LBL_CONNECT
};

//===========================================================================
// Prototypes
//===========================================================================
const char *getText(int labelId);

//===========================================================================
// Globals
//===========================================================================
static const char *labelsEng[] = {
	"New World",
	"Load World",
	"Settings",
	"Quit",

	"Terrain",
	"Random",
	"Flat",
	"Sphere",
	"Pyramid",

	"Creative",
	"Survival",

	"Tiny",
	"Near",
	"Normal",
	"Far",
	"Very Far",

	"Previous",
	"Next",
	"Load sel",
	"Delete sel",
	"Back",

	"Def",
	"Opp",
	"Alt",
	"PC",

	"Main Menu",
	"View distance",
	"Connect to Server",
	"Game mode",

	"Input",
	"Join",
	"Connect",
};

static const char *labelsGer[] = {
	"Neue Welt",
	"Lade Welt",
	"Optionen",
	"Beenden",

	"Gelaende",
	"Zufall",
	"Ebene",
	"Kugel",
	"Pyramide",

	"Kreativmodus",
	"Survival",

	"Minimal",
	"Gering",
	"Normal",
	"Weit",
	"Sehr Weit",

	"Vorherige",
	"Naechste",
	"Lade",
	"Loesche",
	"Menu",

	"Def",
	"Opp",
	"Alt",
	"PC",
	
	"Hauptmenu",
	"Sichtweite",
	"Verbindung zu Server",
	"Spielmodus",
	
	"Eingabe",
	"Join",
	"Verbinde"
};

}

#endif /* TRANSLATEDSTRS_HPP_ */
