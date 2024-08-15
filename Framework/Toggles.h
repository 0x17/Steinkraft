// Toggles.h

#ifndef TOGGLES_H
#define TOGGLES_H

#ifdef _WIN32
#pragma warning( disable : 4996 ) // hide sprintf/strcat/strcpy deprecation
#endif

//==============================================================================
// Determine platform toggle
//==============================================================================
#define MOBILE 0
#define IPHONE 0

#ifdef ANDROID
#undef ANDROID
#define ANDROID 0
#endif // ANDROID

#define MAC 0
#define SDL 0

#ifdef FORCE_SDL
#undef SDL
#define SDL 1
#else // !FORCE_SDL
#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE
#undef MOBILE
#define MOBILE 1
#undef IPHONE
#define IPHONE 1
#elif TARGET_OS_MAC
#undef MAC
#define MAC 1
#define FORCESILENT
#else // _APPLE_ && !(TARGET_OS_IPHONE || TARGET_OS_MAC)
#undef SDL
#define SDL 1
#endif
#else // !__APPLE__
#ifdef ANDROID_NDK
#undef MOBILE
#define MOBILE 1
#undef ANDROID
#define ANDROID 1
#else // !ANDROID_NDK
#undef SDL
#define SDL 1
#endif // ANDROID_NDK
#endif // __APPLE__
#endif // FORCE_SDL

// otherwise we get include problems on win32
#if !MAC && WIN32
#undef MAC
#endif // !MAC && WIN32

//==============================================================================
// Game independent flags
//==============================================================================
#define LIMIT_FPS	1
#define SHOW_FPS	0
#define MOBILE_TEST	0	
#define GRAB_MOUSE	1
#define DEBUG_MODE	0
#define MOBILE_MODE (MOBILE_TEST || MOBILE)

#ifdef FORCESILENT
	#define USE_SOUND 0
#else
	#define USE_SOUND 1
#endif // FORCESILENT

//==============================================================================
// Unsigned types
//==============================================================================
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned char uchar;

//==============================================================================
// Screen dimension
//==============================================================================

namespace as {
	extern int scrW;
	extern int scrH;
	extern bool fullscreen;
}

#define SCR_W scrW
#define SCR_H scrH
#define RETINA 0

#if IPHONE && MOBILE
	namespace as {
		extern bool scrRetina;
	}
	#undef RETINA
	#define RETINA scrRetina
#endif // IPHONE

//==============================================================================
// Frustum related
//==============================================================================
#ifndef PI
	#define PI 3.1415926535897932f
#endif

const float FOV = 40.0f;
const float FOV_RAD = FOV / 180.0f * PI;
const float NEAR_PLANE = 0.01f;
const float FAR_PLANE = 1024.0f;
const float MIN_DIST = 0.1f;

#endif // TOGGLES_H
