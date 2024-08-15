// Constants.h

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "Framework/Toggles.h"

//==============================================================================
// Game flags
//==============================================================================
#define HIRES_TEX	1
#define NO_NET		1

//==============================================================================

#define VERSION_STR "07"
#define COMPILE_INFO "BUILD: date(" __DATE__ ") time(" __TIME__ ")"

#define BASE_CAPTION "Steinkraft"
#define AUTHOR_NAME "Andre Schnabel"

namespace as {
const char * const CAPTION = BASE_CAPTION " V" VERSION_STR " # " COMPILE_INFO;

//==============================================================================
// Visual detail
//==============================================================================
enum DetailLevel {
    DETAIL_VERY_LOW,
    DETAIL_LOW,
    DETAIL_MEDIUM,
    DETAIL_HIGH,
    DETAIL_VERY_HIGH
};

extern DetailLevel visualDetail;

typedef struct settings_s {
    int visualDetail, inputMethod;
} settings_t;

//==============================================================================
// Input
//==============================================================================
enum InputMethodType {
    IM_DEFAULT, // both joysticks bottom
    IM_OPPOSED, // mvmt joystick left bottom, rotation joystick right top
    IM_RIPOFF, // tap to build, long tap to dig, dragging to rotate
    IM_PC, // both joysticks bottom and pc like digging/put
    NUM_INPUT_METHODS,
    IM_RIPOFF_DPAD // tap to build, long tap to dig, dragging to rotate (dpad mvmt)
};

extern InputMethodType activeInputMethod;

//==============================================================================
// Texturing constants
//==============================================================================
const int TEX_SIZE = 16;
const int TEXMAP_SIZE = 128;
const float ACT_TEX_SIZE = 256.0f;
const char * const TEX_FILENAME = "data/texmap.png";
const char * const TEX_CLASSIC_FILENAME = "data/texmapClassic.png";
const char * const TEX_HIDEF_FILENAME = "data/texmapHidef.png";

const float TEX_COORD_FACTOR = ((float)TEX_SIZE / ACT_TEX_SIZE);
const int NUM_TEXTURES = ((TEXMAP_SIZE / TEX_SIZE) * (TEXMAP_SIZE / TEX_SIZE));
const int NUM_TEX_PER_ROW = (TEXMAP_SIZE / TEX_SIZE);

#define UV_ROW_COL(row, col)	((col-1)*TEX_SIZE)/ACT_TEX_SIZE, \
								((col+0)*TEX_SIZE)/ACT_TEX_SIZE, \
								((row-1)*TEX_SIZE)/ACT_TEX_SIZE, \
								((row+0)*TEX_SIZE)/ACT_TEX_SIZE

#define UV_MIN_MAX(minU, maxU, minV, maxV)	(minU)/ACT_TEX_SIZE, \
											(maxU)/ACT_TEX_SIZE, \
											(minV)/ACT_TEX_SIZE, \
											(maxV)/ACT_TEX_SIZE

template <class T>
inline T TXCRD(T u) {
    return u / ACT_TEX_SIZE;
}

#define TXCRDS(u,v) u/ACT_TEX_SIZE, v/ACT_TEX_SIZE

//==============================================================================
// Sound constants
//==============================================================================
enum Sounds {
    SND_PUT = 0,
    SND_FALL,
    SND_JUMP,
    SND_DIG,
    SND_STEP1,
    SND_STEP2,
    SND_STEP3,
    SND_STEP4
};

const int NUM_SOUNDS = 8;

extern const char *soundFilenames[NUM_SOUNDS]; // see Movement.cpp

//==============================================================================
// Language constants
//==============================================================================
typedef enum lang_e {
    LANG_ENG,
    LANG_GER
} lang_t;

extern lang_t curLang;

//===========================================================================
// Game mode
//===========================================================================
extern bool keepMeshes;
// MISC
extern bool buyIntent;

// TOGGLES
extern bool noNight; // always day
extern bool noAnimals;
extern bool noSound;
extern bool classicTexture;

typedef enum texmap_e {
	TEXMAP_CLASSIC,
	TEXMAP_SPACE,
	TEXMAP_HIDEF,
	NUM_TEXMAPS
} texmap_t;
extern int activeTexMap;
	
typedef struct toggles_s {
	bool noNight, noAnimals, noSound;
} toggles_t;

} // namespace as

#endif /* CONSTANTS_H */

