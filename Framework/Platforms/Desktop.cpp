// Desktop.cpp

#include "StdAfx.h"
#pragma hdrstop

#include "Desktop.hpp"

/*#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/InflatingStream.h>
#include <Poco/DeflatingStream.h>*/

#include <filesystem>

#include <cstring>
#include <fstream>

#include "../State.hpp"
#include "../Utilities.hpp"

#include "../Texture.hpp"

//! SDL backend code
#if SDL

namespace as {

int scrW = 800, scrH = 480;
lang_t curLang;

const int MAX_FPS = 80;

#if USE_SOUND
Mix_Chunk *sounds[NUM_SOUNDS];
#endif

DetailLevel visualDetail = DETAIL_VERY_HIGH;

bool done = false;
Uint8 *keys = NULL;
Texture *texMap;

void LoadSounds();
void FreeSounds();

LibSdl::LibSdl() {
	int result;
	ticks_t vidflags = SDL_OPENGL, sdlflags = SDL_INIT_VIDEO;

	curLang = LANG_ENG;

	if (fullscreen) {
		vidflags |= SDL_FULLSCREEN;
	}

#if USE_SOUND
	sdlflags |= SDL_INIT_AUDIO;
#endif

	result = SDL_Init(sdlflags);
	if (result == -1)
		error("Unable to initialize SDL!");

	atexit(SDL_Quit);

	SDL_WM_SetCaption(CAPTION, NULL);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);

	if (!SDL_SetVideoMode(SCR_W, SCR_H, 32, vidflags))
		error("Unable to set video mode!");

	SDL_ShowCursor(MOBILE_TEST);

#if GRAB_MOUSE
	SDL_WM_GrabInput(SDL_GRAB_ON);
#endif

	if (glewInit() != GLEW_OK)
		error("GLEW initialization failed!");

	if (!GLEW_VERSION_1_3)
		error("No OpenGL 1.3 support. Abort.");

	useVertexArray = (!GLEW_VERSION_2_0);
	std::printf("Using %s for geometry.\n", useVertexArray ?
		"vertex arrays" : "vertex buffer objects");

#if USE_SOUND
	result = Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024);
	if (result == -1)
		error("Unable to open audio!");

	LoadSounds();
#endif
}

LibSdl::~LibSdl() {
	FreeSounds();
	SDL_Quit();

#if USE_SOUND
	Mix_CloseAudio();
#endif
}

bool convKeys[NUM_KEYS];
#define CONV_KCODES(src, dest) if(keys[src]){convKeys[dest]=true;}

void LibSdl::processInput( ticks_t delta )
{
	static SDL_Event event;
	static int dX, dY, b;
	static MouseButtons mb;

	b = 0;

	while (SDL_PollEvent(&event)) {
		done = (event.type == SDL_QUIT);

		if (event.type == SDL_MOUSEBUTTONDOWN && b == 0) {
			if (event.button.button == SDL_BUTTON_WHEELUP)
				b = 1;
			else if (event.button.button == SDL_BUTTON_WHEELDOWN)
				b = -1;
		}
	}

	SDL_PumpEvents();
	keys = SDL_GetKeyState(NULL);

	done = keys[SDLK_ESCAPE] > 0 || done;

	if (keys[SDLK_g])
		SDL_WM_GrabInput(SDL_GRAB_OFF);
	if (keys[SDLK_h])
		SDL_WM_GrabInput(SDL_GRAB_ON);

	memset(convKeys, 0, sizeof(bool) * NUM_KEYS);
	CONV_KCODES(SDLK_LEFT, KEY_LEFT);
	CONV_KCODES(SDLK_RIGHT, KEY_RIGHT);
	CONV_KCODES(SDLK_UP, KEY_UP);
	CONV_KCODES(SDLK_DOWN, KEY_DOWN);
	CONV_KCODES(SDLK_w, KEY_W);
	CONV_KCODES(SDLK_a, KEY_A);
	CONV_KCODES(SDLK_s, KEY_S);
	CONV_KCODES(SDLK_d, KEY_D);
	CONV_KCODES(SDLK_p, KEY_P);
	CONV_KCODES(SDLK_o, KEY_O);
	CONV_KCODES(SDLK_f, KEY_F);
	CONV_KCODES(SDLK_BACKSPACE, KEY_BACKSPACE);
	CONV_KCODES(SDLK_RETURN, KEY_RETURN);
	CONV_KCODES(SDLK_SPACE, KEY_SPACE);

	StateManager::getInstance()->getState()->processKeyboardInput(convKeys, SDL_GetModState(), delta);

	Uint8 btns = SDL_GetRelativeMouseState(&dX, &dY);

	mb.lmb = ((btns & SDL_BUTTON_LMASK) != 0);
	mb.mmb = ((btns & SDL_BUTTON_MMASK) != 0);
	mb.rmb = ((btns & SDL_BUTTON_RMASK) != 0);

	StateManager::getInstance()->getState()->processMouseInput(dX, dY, b, &mb, delta);

#if MOBILE_TEST
	int x, y;
	Uint8 res = SDL_GetMouseState(&x, &y);
	if (res & SDL_BUTTON(SDL_BUTTON_LEFT))
		StateManager::getInstance()->getState()->processTouch(x, y, delta);
	else
		StateManager::getInstance()->getState()->processNoTouch();
#endif
}

void LibSdl::display( ticks_t delta )
{
	StateManager::getInstance()->getState()->draw(delta);
	SDL_GL_SwapBuffers();
}

void LoadSounds() {
#if USE_SOUND
	char sndfname[BUF_LEN];

	for (int i = 0; i < NUM_SOUNDS; i++) {
		std::sprintf(sndfname, "data/sounds/%s", soundFilenames[i]);
		sounds[i] = Mix_LoadWAV_RW(SDL_RWFromFile(sndfname, "rb"), 1);
		if (!sounds[i]) {
			std::printf("Loading sound failed!");
			exit(1);
		}
	}
#endif
}

void FreeSounds() {
#if USE_SOUND
	for (int i = 0; i < NUM_SOUNDS; i++) {
		Mix_FreeChunk(sounds[i]);
	}
#endif
}

void SDL_QuitApp() {
	done = true;
}

void SDL_PlaySound(int sndId) {
#if USE_SOUND
	if(noSound) return;
	Mix_PlayChannelTimed(-1, sounds[sndId], 0, -1);
#endif
}

void SDL_StopSound(int sndId) {
	if(noSound) return;
	//Mix_HaltChannel(-1);
}

void SDL_BinaryWrite(const char *filename, const void *data, size_t size, bool append) {
	/*char filenameWithSuffix[1024];
	sprintf(filenameWithSuffix, "%s.gz", filename);
	Poco::FileOutputStream outStream(std::string(filenameWithSuffix), std::ios::binary);
	Poco::DeflatingOutputStream compressStream(outStream, Poco::DeflatingStreamBuf::STREAM_GZIP);
	compressStream.write((const char *)data, size);
	compressStream.close();
	outStream.close();*/
	auto flags {std::ios::binary};
	if(append) flags |= std::ios::app;
	std::ofstream ofs{filename, flags};
	ofs.write((const char *)data, size);
}

void SDL_BinaryRead(const char *filename, void *data, size_t size) {
	/*char filenameWithSuffix[1024];
	sprintf(filenameWithSuffix, "%s.gz", filename);
	Poco::FileInputStream inStream(std::string(filenameWithSuffix), std::ios::binary);
	Poco::InflatingInputStream decompressedStream(inStream, Poco::InflatingStreamBuf::STREAM_GZIP);
	decompressedStream.read((char *)data, size);
	inStream.close();*/
	std::ifstream ifs{filename, std::ios::binary};
	ifs.read((char *)data, size);
}

bool SDL_FileExists(const char *filename) {
	//return Poco::File(std::string(filename) + ".gz").exists();
	return std::filesystem::exists(filename);
}

void SDL_DeleteFile(const char *filename) {
	/*Poco::File f(std::string(filename) + ".gz");
	f.remove();*/
	std::filesystem::remove(filename);
}
	
void SDL_ToggleTexture(int texMapIndex) {
	SAFE_DELETE(texMap);

	const char *texFilename;
	switch(texMapIndex) {
	case TEXMAP_CLASSIC:
		texFilename = TEX_CLASSIC_FILENAME;
		break;
	case TEXMAP_SPACE:
		texFilename = TEX_FILENAME;
		break;
	case TEXMAP_HIDEF:
		texFilename = TEX_HIDEF_FILENAME;
		break;
	}
	texMap = new Texture(texFilename, true);

	texMap->bind();
}

void LibSdl::mainLoop()
{
	toggleTexture(activeTexMap);
	
	ticks_t sTime = getTicks(), delta, lfpsPrint = 0;

	while (!done) {
		delta = getTicks() - sTime;
		delta = MIN((ticks_t)2000, delta);
		sTime = getTicks();

		processInput(delta);
		display(delta);

		// limit to MAX_FPS iff. LIMIT_FPS is true
		if (LIMIT_FPS && getTicks() - sTime < 1000 / MAX_FPS)
			SDL_Delay(1000 / MAX_FPS - (getTicks() - sTime));

		// print fps every 3 seconds iff. SHOW_FPS is true
		if (SHOW_FPS && getTicks() - lfpsPrint > 3000) {
			lfpsPrint = getTicks();
			if (delta != 0) std::printf("FPS=%.2f\n", 1000.0f / (float)delta);
		}
	}
	
	SAFE_DELETE(texMap);
}

} // namespace as

#endif // SDL
