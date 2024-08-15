// Main.cpp



#include "Framework/Platforms/Desktop.hpp"
#include "Framework/Texture.hpp"

#include "States/LandscapeScene.hpp"
#include "States/SplashState.hpp"

#include "Managers/NetManager.hpp"

using namespace as;

int main(int argc, char **argv) {
	sprintf(remoteIPStr, "127.0.0.1");

	if (argc >= 2) {
		for (int i = 1; i < argc; i++) {
			if (!strcmp(argv[i], "scrW") && i < argc - 1)
				scrW = atoi(argv[i+1]);
			else if (!strcmp(argv[i], "scrH") && i < argc - 1)
				scrH = atoi(argv[i+1]);
			else if (!strcmp(argv[i], "ip") && i < argc - 1)
				strcpy(remoteIPStr, argv[i+1]);
			else if (!strcmp(argv[i], "fullscreen"))
				fullscreen = true;
		}
	}

	LibSdl lsdl;
	initGL();

	StateManager *g = StateManager::getInstance();

	State *s = new SplashState(g);
	g->setState(s);

	lsdl.mainLoop();

	SAFE_DELETE(g);

	return 0;
}
