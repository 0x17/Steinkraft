// Utilities.cpp

#include "StdAfx.h"
#pragma hdrstop

#include <cmath>
#include <cassert>
#include <cstdlib>

#include "PGL.h"

#include "Math/Vector.hpp"
#include "Math/Mat4x4.hpp"

#include "Utilities.hpp"

namespace as {
	
bool fullscreen;

void error(const char *msg) {
	std::printf("Error: %s\n", msg);
	std::fflush(stdout);
	exit(1);
}

void initGL() {
#if MOBILE
	useVertexArray = false;
	glClearDepthf(1.0f);
#else
	glClearDepth(1.0f);
#endif
	
	glEnable(GL_CULL_FACE);
	
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	glAlphaFunc(GL_GREATER, 0.5f);
	glDisable(GL_ALPHA_TEST);
	
	glDisable(GL_STENCIL_TEST);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
	
#if !MOBILE // not part of GLES spec
	glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
#endif
	
	glHint(GL_GENERATE_MIPMAP_HINT, GL_FASTEST);
	
	glViewport(0, 0, SCR_W, SCR_H);
	
	glEnable(GL_TEXTURE_2D);
	
	glDisable(GL_LIGHTING);
	
	collectGlError();		
}

}
