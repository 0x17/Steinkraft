// Utilities.hpp

#ifndef UTILITIES_HPP_
#define UTILITIES_HPP_

#include <cassert>

#include "PGL.h"

#ifndef PI
const float PI = 3.1415926535897932f;
#endif

#if DEBUG && 0
inline void collectGlError() {
	GLenum val;
	if((val = glGetError()) != 0) {
		printf("glError: %d :: ", val);

		switch(val) {
			case GL_INVALID_ENUM:
				printf("Table too large!");
				break;
			case GL_INVALID_VALUE:
				printf("Invalid value!");
				break;
			case GL_INVALID_OPERATION:
				printf("Invalid operation!");
				break;
			case GL_STACK_OVERFLOW:
				printf("Stack overflow!");
				break;
			case GL_STACK_UNDERFLOW:
				printf("Stack underflow!");
				break;
			case GL_OUT_OF_MEMORY:
				printf("Out of memory!");
				break;
			/*case GL_TABLE_TOO_LARGE:
				printf("Table too large!");
				break;*/
		}

		printf("\n");
		//printf("GLU: %s\n", gluErrorString());
	}
}
#else
#define collectGlError(); ;
#endif

inline float deg2rad(float alpha) {
	return alpha / 180.0f*PI;
}

inline float rad2deg(float rad) {
	return rad / PI*180.0f;
}

template <class T>
inline T myAbs(T x) { return (x > 0) ? x : -x; }

template <class T>
inline T myFabs(T x) { return (x > 0.0f) ? x : -x; }

template <class T>
inline T myMin(T x, T y) { return (x > y) ? y : x; }

template <class T>
inline T myMax(T x, T y) { return (x > y) ? x : y; }

#ifndef ABS
#define ABS myAbs
#endif

#ifndef FABS
#define FABS myFabs
#endif

#ifndef MAX
#define MAX myMax
#endif

#ifndef MIN
#define MIN myMin
#endif

template <class T>
inline T SIGN(T x) { return (x > 0) ? 1 : -1; }

#define SAFE_DELETE(p) if(p) { delete p; p=NULL; }
#define SAFE_DELETE_ARRAY(p) if(p) { delete [] p; p=NULL; }

const int BUF_LEN = 512;
const int LBUF_LEN = 1024;

#if SDL && !MOBILE_TEST
	inline void startMouseMode() {
		SDL_ShowCursor(SDL_TRUE);
		SDL_WM_GrabInput(SDL_GRAB_OFF);
	}

	inline void stopMouseMode() {
		SDL_ShowCursor(SDL_FALSE);
		SDL_WM_GrabInput(GRAB_MOUSE ? SDL_GRAB_ON : SDL_GRAB_OFF);
	}

	// draw modes
	inline void drawModeWireframe() { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
	inline void drawModeFilled() { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }
#elif MAC && !MOBILE_TEST
	extern void startMouseMode();
	extern void stopMouseMode();
#else
	inline void startMouseMode() { while (0) {} }
	inline void stopMouseMode() { while (0) {} }
#endif

namespace as {

void error(const char *msg);
void initGL();

}

#endif
