// gl_code.cpp
/*
 @file
 @author André Schnabel <aschnabel64@gmail.com>
 @section DESCRIPTION
 Contains Android NDK specific glue code.
*/

#include "StdAfx.h"
#pragma hdrstop

#if ANDROID

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/InflatingStream.h>
#include <Poco/DeflatingStream.h>

#include "Framework/PGL.h"

#include <jni.h>
#include <android/log.h>

#include <GLES/gl.h>

#include <fstream>
#include <list>

#include <sys/stat.h>

#include "Constants.h"
#include "Framework/State.hpp"
#include "States/LandscapeScene.hpp"
#include "States/SplashState.hpp"
#include "TranslatedStrs.hpp"

//===========================================================================
// Constants/Macros
//===========================================================================
#ifndef LOG_TAG
#define  LOG_TAG    "libsteinkraft"
#endif
#ifndef LOGI
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#endif
#ifndef LOGE
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif

//==============================================================================
// Globals
//==============================================================================
as::StateManager *g = NULL;
int x[4], y[4];
ticks_t lf_time = 0;
int as::scrW = 800, as::scrH = 480;
as::DetailLevel as::visualDetail = as::DETAIL_LOW;
std::list<int> scheduledSounds;
char extPath[512];

bool textureToggleReq = false;
int activeTexMap = 0;

namespace as {
lang_t curLang;
}

//==============================================================================
// Global methods
//==============================================================================
void Droid_QuitApp() {
	SAFE_DELETE(g);
	exit(0);
}

void Droid_PlaySound(int sndId) {
	if(as::noSound) return;
	// investigate why increment is necessary?
	scheduledSounds.push_back(++sndId);
}

void Droid_StopSound(int sndId) {
	if(as::noSound) return;
	scheduledSounds.clear();
}

int pollSound() {
	if (scheduledSounds.empty()) return -1;
	else {
		int res = scheduledSounds.front();
		scheduledSounds.pop_front();
		return res;
	}
}

void createDirIfNeeded() {
	struct stat st;
	LOGI("looking for steincraft dir!");
	if (stat(extPath, &st) != 0) {
		LOGI("dir not found... creating!");
		mkdir(extPath, S_IRWXU);
		if (stat(extPath, &st) == 0)
			LOGI("mkdir success!");
		else
			LOGI("mkdir failed!");
	}
}

void Droid_BinaryWrite(const char *filename, const void *data, size_t size, bool append) {
	static char fnbuf[1024];
	createDirIfNeeded();
	strcpy(fnbuf, extPath);
	strcat(fnbuf, filename);
	LOGI("writing file: %s", fnbuf);
	/*std::fstream f(fnbuf, std::ios::out | std::ios::binary);
	if(f.is_open()) {
		f.write((char *)data, size);
		f.flush();
		f.close();
		LOGI("file written!");
	} else {
		LOGE("Error writing file: %s!", fnbuf);
	}*/
	
	// SAFE METHOD
	
	/*FILE *fp = fopen(fnbuf, "wb");
	if (fp) {
		fwrite(data, size, 1, fp);
		fclose(fp);
		LOGI("file written!");
	} else {
		LOGE("Error writing file: %s!", fnbuf);
	}*/
	
	// USE COMPRESSION
	Poco::FileOutputStream outStream(std::string(fnbuf), std::ios::binary);
	Poco::DeflatingOutputStream compressStream(outStream, Poco::DeflatingStreamBuf::STREAM_GZIP);
	compressStream.write((const char *)data, size);
	compressStream.close();
	outStream.close();
}

const char *Droid_GetDataPath() {
	return extPath;
}

const char *Droid_GetAssetPath() {
	return NULL;
}

void Droid_BinaryRead(const char *filename, void *data, size_t size) {
	static char fnbuf[1024];
	strcpy(fnbuf, extPath);
	strcat(fnbuf, filename);
	LOGI("reading file: %s", fnbuf);
	/*std::fstream f(fnbuf, std::ios::in | std::ios::binary);
	if(f.is_open()) {
		f.read((char *)data, size);
		f.close();
		LOGI("File read!");
	} else {
		LOGE("Error reading file: %s!", fnbuf);
	}*/
	
	// SAFE METHOD
	/*FILE *fp = fopen(fnbuf, "rb");
	if (fp) {
		fread(data, size, 1, fp);
		fclose(fp);
		LOGI("File read!");
	} else {
		LOGE("Error reading file: %s!", fnbuf);
	}*/
	
	// USE COMPRESSION
	Poco::FileInputStream inStream(std::string(fnbuf), std::ios::binary);
	Poco::InflatingInputStream decompressedStream(inStream, Poco::InflatingStreamBuf::STREAM_GZIP);
	decompressedStream.read((char *)data, size);
	inStream.close();
}

std::string Droid_ReadText(const char *filename) {
	static char fnbuf[1024];
	strcpy(fnbuf, extPath);
	strcat(fnbuf, filename);
	FILE *fp = fopen(fnbuf, "rb");
	
	std::string outStr;
	
	char c;
	do {
		c = fgetc(fp);
		outStr.append(1, c);
	} while(c != EOF);
	
	fclose(fp);
	
	return outStr;
}

bool Droid_FileExists(const char *filename) {
	static char fnbuf[1024];
	strcpy(fnbuf, extPath);
	strcat(fnbuf, filename);
	struct stat st;
	return (stat(fnbuf, &st) == 0);
}

void Droid_DeleteFile(const char *filename) {
	static char fnbuf[1024];
	strcpy(fnbuf, extPath);
	strcat(fnbuf, filename);
	remove(fnbuf);
}

void Droid_ToggleTexture(int _activeTexMap) {
	textureToggleReq = true;
	activeTexMap = _activeTexMap;
}

int pollTextureToggle() {
	if(!textureToggleReq) return -1;
	else {
		textureToggleReq = false;
		return activeTexMap;
	}
}

bool setupGraphics(int w, int h) {
	for (int i = 0; i < 4; i++) {
		x[i] = y[i] = -1;
	}
	as::scrW = w;
	as::scrH = h;
	g = as::StateManager::getInstance();
	g->setState(new as::SplashState(g));
	lf_time = getTicks();
	return true;
}

ticks_t delta;

void renderFrame() {
	if(!g) return;
	
	delta = getTicks() - lf_time;
	delta = MIN((ticks_t)2000, delta);
	lf_time = getTicks();

	g->getState()->processKeyboardInput(NULL, 0, delta);

	bool wasTouched = false;

	for (int i = 0; i < 4; i++) {
		if (x[i] != -1 && y[i] != -1) {
			g->getState()->processTouch(x[i], y[i], delta/*, i*/);
			wasTouched = true;
		} /*else {
			g->getState()->processTouch(-1, -1, delta, i);
		}*/
	}

	if (!wasTouched) {
		g->getState()->processNoTouch();
	}

	g->getState()->draw(delta);
}

void processTouch(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) {
	x[0] = x1;
	x[1] = x2;
	x[2] = x3;
	x[3] = x4;
	y[0] = y1;
	y[1] = y2;
	y[2] = y3;
	y[3] = y4;
}

void destroyStuff(int onlyPersist) {
	if(onlyPersist == 1){
		if(g) {
			g->getState()->persist();
		}
	} else {
		SAFE_DELETE(g);
	}
}

//==============================================================================
// JNI related
//==============================================================================

#ifndef LITE // for full version
extern "C" {
	JNIEXPORT void JNICALL Java_com_andredotcom_steinkraft_SteinkraftLib_init(JNIEnv * env, jobject obj,  jint width, jint height, jstring path, jint lang);
	JNIEXPORT void JNICALL Java_com_andredotcom_steinkraft_SteinkraftLib_step(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_andredotcom_steinkraft_SteinkraftLib_shutdown(JNIEnv * env, jobject obj, jint onlyPersist);
	JNIEXPORT void JNICALL Java_com_andredotcom_steinkraft_SteinkraftLib_touch(JNIEnv * env, jobject obj, jint x1, jint y1, jint x2, jint y2, jint x3, jint y3, jint x4, jint y4);
	JNIEXPORT int JNICALL Java_com_andredotcom_steinkraft_SteinkraftLib_pollSound(JNIEnv * env, jobject obj);
	JNIEXPORT int JNICALL Java_com_andredotcom_steinkraft_SteinkraftLib_pollTextureToggle(JNIEnv * env, jobject obj);
};

JNIEXPORT void JNICALL Java_com_andredotcom_steinkraft_SteinkraftLib_init(JNIEnv * env, jobject obj,  jint width, jint height, jstring path, jint lang) {
	const char *str = env->GetStringUTFChars(path, 0);
	strcpy(extPath, str);
	strcat(extPath, "/" BASE_CAPTION "/");
	LOGI("extPath=%s", extPath);
	env->ReleaseStringUTFChars(path, str);

	as::curLang = (as::lang_t)lang;

	setupGraphics(width, height);
}

JNIEXPORT void JNICALL Java_com_andredotcom_steinkraft_SteinkraftLib_step(JNIEnv * env, jobject obj) {
	renderFrame();
}

JNIEXPORT void JNICALL Java_com_andredotcom_steinkraft_SteinkraftLib_shutdown(JNIEnv * env, jobject obj, jint onlyPersist) {
	destroyStuff(onlyPersist);
}

JNIEXPORT void JNICALL Java_com_andredotcom_steinkraft_SteinkraftLib_touch(JNIEnv * env, jobject obj, jint x1, jint y1, jint x2, jint y2, jint x3, jint y3, jint x4, jint y4) {
	processTouch(x1, y1, x2, y2, x3, y3, x4, y4);
}

JNIEXPORT int JNICALL Java_com_andredotcom_steinkraft_SteinkraftLib_pollSound(JNIEnv * env, jobject obj) {
	return pollSound();
}

JNIEXPORT int JNICALL Java_com_andredotcom_steinkraft_SteinkraftLib_pollTextureToggle(JNIEnv * env, jobject obj) {
	return pollTextureToggle();
}

#else // for LITE version

extern "C" {
	JNIEXPORT void JNICALL Java_com_andredotcom_steinkraftlite_SteinkraftLib_init(JNIEnv * env, jobject obj,  jint width, jint height, jstring path, jint lang);
	JNIEXPORT void JNICALL Java_com_andredotcom_steinkraftlite_SteinkraftLib_step(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_andredotcom_steinkraftlite_SteinkraftLib_shutdown(JNIEnv * env, jobject obj, jint onlyPersist);
	JNIEXPORT void JNICALL Java_com_andredotcom_steinkraftlite_SteinkraftLib_touch(JNIEnv * env, jobject obj, jint x1, jint y1, jint x2, jint y2, jint x3, jint y3, jint x4, jint y4);
	JNIEXPORT int  JNICALL Java_com_andredotcom_steinkraftlite_SteinkraftLib_pollSound(JNIEnv * env, jobject obj);
	JNIEXPORT int  JNICALL Java_com_andredotcom_steinkraftlite_SteinkraftLib_pollBuyIntent(JNIEnv * env, jobject obj);
	JNIEXPORT int  JNICALL Java_com_andredotcom_steinkraftlite_SteinkraftLib_pollTextureToggle(JNIEnv * env, jobject obj);
};

JNIEXPORT void JNICALL Java_com_andredotcom_steinkraftlite_SteinkraftLib_init(JNIEnv * env, jobject obj,  jint width, jint height, jstring path, jint lang) {
	const char *str = env->GetStringUTFChars(path, 0);
	strcpy(extPath, str);
	strcat(extPath, "/" BASE_CAPTION "/");
	LOGI("extPath=%s", extPath);
	env->ReleaseStringUTFChars(path, str);

	as::curLang = (as::lang_t)lang;

	setupGraphics(width, height);
}

JNIEXPORT void JNICALL Java_com_andredotcom_steinkraftlite_SteinkraftLib_step(JNIEnv * env, jobject obj) {
	renderFrame();
}

JNIEXPORT void JNICALL Java_com_andredotcom_steinkraftlite_SteinkraftLib_shutdown(JNIEnv * env, jobject obj, jint onlyPersist) {
	destroyStuff(onlyPersist);
}

JNIEXPORT void JNICALL Java_com_andredotcom_steinkraftlite_SteinkraftLib_touch(JNIEnv * env, jobject obj, jint x1, jint y1, jint x2, jint y2, jint x3, jint y3, jint x4, jint y4) {
	processTouch(x1, y1, x2, y2, x3, y3, x4, y4);
}

JNIEXPORT int JNICALL Java_com_andredotcom_steinkraftlite_SteinkraftLib_pollSound(JNIEnv * env, jobject obj) {
	return pollSound();
}

JNIEXPORT int JNICALL Java_com_andredotcom_steinkraftlite_SteinkraftLib_pollBuyIntent(JNIEnv * env, jobject obj) {
	return (as::buyIntent == true) ? 1 : 0;
}

JNIEXPORT int JNICALL Java_com_andredotcom_steinkraftlite_SteinkraftLib_pollTextureToggle(JNIEnv * env, jobject obj) {
	return pollTextureToggle();
}

#endif

#endif // ANDROID

