// PGL.h
// Portable inclusion

#ifndef PGL_H
#define PGL_H

#include "../Constants.h"

#if SDL
#include <SDL/SDL.h>
#include <GL/glew.h>
#if USE_SOUND
#include <SDL/SDL_mixer.h>
#endif
#elif IPHONE
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#elif ANDROID
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOG_TAG    "libsteinkraft"
#elif MAC
#if __OBJC__
#import <Cocoa/Cocoa.h>
#endif
#include <OpenGL/glu.h>
#endif

#include <cstring>
#include <cmath>

// since some envs don't know trigs for floats
#ifndef sinf
#define sinf sin
#endif
#ifndef cosf
#define cosf cos
#endif

#if !SDL
typedef unsigned int Uint32;
typedef unsigned int Uint8;
typedef int SDLMod;
#endif

extern void localAddrToStr(char *dest);

#if MOBILE
typedef unsigned long ticks_t;
#if ANDROID
#include <time.h>
inline ticks_t UX_GetTicks() {
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}
#define getTicks UX_GetTicks
#define quitGame Droid_QuitApp
extern void Droid_QuitApp();
extern void Droid_PlaySound(int sndId);
#define playSound Droid_PlaySound
extern void Droid_StopSound(int sndId);
#define stopSound Droid_StopSound
extern void Droid_BinaryWrite(const char *filename, const void *data, size_t size, bool append = false);
#define binaryWrite Droid_BinaryWrite
extern void Droid_BinaryRead(const char *filename, void *data, size_t size);
#define binaryRead Droid_BinaryRead
extern bool Droid_FileExists(const char *filename);
#define fileExists Droid_FileExists
extern void Droid_DeleteFile(const char *filename);
#define deleteFile Droid_DeleteFile
extern void Droid_ToggleTexture(int classic);
#define toggleTexture Droid_ToggleTexture
#elif IPHONE
extern ticks_t IOS_GetTicks();
#define getTicks IOS_GetTicks
extern void IOS_QuitApp();
#define quitGame IOS_QuitApp
extern void IOS_PlaySound(int sndId);
#define playSound IOS_PlaySound
extern void IOS_StopSound(int sndId);
#define stopSound IOS_StopSound
extern void IOS_BinaryWrite(const char *filename, const void *data, size_t size, bool append = false);
#define binaryWrite IOS_BinaryWrite
extern void IOS_BinaryRead(const char *filename, void *data, size_t size);
#define binaryRead IOS_BinaryRead
extern bool IOS_FileExists(const char *filename, bool trySuffix = true);
#define fileExists IOS_FileExists
extern void IOS_DeleteFile(const char *filename);
#define deleteFile IOS_DeleteFile
extern void IOS_ToggleTexture(int classic);
#define toggleTexture IOS_ToggleTexture
#endif
#elif SDL
typedef Uint32 ticks_t;

namespace as {
extern void SDL_QuitApp();
extern void SDL_PlaySound(int sndId);
extern void SDL_StopSound(int sndId);
extern void SDL_BinaryWrite(const char *filename, const void *data, size_t size, bool append = false);
extern void SDL_BinaryRead(const char *filename, void *data, size_t size);
extern bool SDL_FileExists(const char *filename);
extern void SDL_DeleteFile(const char *filename);
extern void SDL_ToggleTexture(int texMapIndex);
}

#define getTicks SDL_GetTicks
#define quitGame SDL_QuitApp
#define playSound SDL_PlaySound
#define stopSound SDL_StopSound
#define binaryWrite SDL_BinaryWrite
#define binaryRead SDL_BinaryRead
#define fileExists SDL_FileExists
#define deleteFile SDL_DeleteFile
#define toggleTexture SDL_ToggleTexture

#elif MAC
typedef unsigned long ticks_t;
extern long OSX_GetTicks();
#define getTicks OSX_GetTicks
extern void OSX_QuitApp();
#define quitGame OSX_QuitApp
extern void OSX_PlaySound(int sndId);
#define playSound OSX_PlaySound
extern void OSX_StopSound(int sndId);
#define stopSound OSX_StopSound
extern void OSX_BinaryWrite(const char *filename, const void *data, size_t size, bool append = false);
#define binaryWrite OSX_BinaryWrite
extern void OSX_BinaryRead(const char *filename, void *data, size_t size);
#define binaryRead OSX_BinaryRead
extern bool OSX_FileExists(const char *filename);
#define fileExists OSX_FileExists
extern void OSX_DeleteFile(const char *filename);
#define deleteFile OSX_DeleteFile
extern void OSX_GetMousePos(int *x, int *y);
extern void OSX_ToggleTexture(int classic);
#define toggleTexture OSX_ToggleTexture
#endif

namespace as {
extern bool useVertexArray;
}

#endif
