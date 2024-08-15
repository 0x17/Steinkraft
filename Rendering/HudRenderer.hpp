// HudRenderer.hpp

#ifndef HUDRENDERER_HPP_
#define HUDRENDERER_HPP_

#include "../Framework/PGL.h"

#include "../Framework/Camera.hpp"

namespace as {

#define DPAD_BTN_SIZE scl(40.0f)

class Camera;
class BlockMesh;
class QuadMesh;
class Mesh;
class Terrain;
class SpriteCache;
class FontSpriteCache;
class Animation;
class Player;
class AnimalManager;

class HudRenderer {
public:
	HudRenderer(Terrain *terrain, Camera *cam, AnimalManager *animalMgr);
	void initMeshes();
	virtual ~HudRenderer();
	void updateTexPreview(int selectedTexture);
	void render(bool digMode, bool drawTexSelOverlay);
	void startDigAnim();
	void startPutAnim();

	float getScaleFactor() const;

	void setDrawFeedbackCircles(bool drawFeedbackCircles, float fcx = 0.0f, float fcy = 0.0f, float fciz = 1.0f);

	void toggleVisibility();
	
	static HudRenderer *getInstance() {
		return instance;
	}

private:
	void drawHeadsUpDisplay();
	void drawBlockPreview();

	void drawControlsOverlay(bool digMode);
	void drawFeedbackCircles();

	void drawInfo();
	void drawAnimalOverlays();

	// auxiliary
	template <class T> T scl(T x) const;
	
	static HudRenderer *instance;
	
	Camera *cam;
	Terrain *terrain;
	AnimalManager *animalMgr;

	QuadMesh *crosshairMesh, *texPreviewMesh;

	QuadMesh *leftOuterCircleMesh, *rightOuterCircleMesh, *innerCircleMesh;
	QuadMesh *digBtnMesh, *putBtnMesh;
	QuadMesh *jumpBtnMesh, *menuBtnMesh;
	QuadMesh *innerFeedbackCircle, *outerFeedbackCircle;
	QuadMesh *dpadLeftMesh, *dpadRightMesh;
	QuadMesh *dpadUpMesh, *dpadDownMesh;
	QuadMesh *dpadJumpMesh;

	BlockMesh *blockPreviewMesh, *darkBlockPreviewMesh;

	Animation *digAnim, *putAnim;

	float scaleFactor;

	FontSpriteCache *fsb;

	Mesh *texOverlayMesh, *entityOverlayMesh, *minecartOverlayMesh, *fenceOverlayMesh;

	bool entitySelected;

	bool drawfcircles;
	float fcx, fcy, fciz;

	bool hidden;
	
	OrthoCamera orthoCam;
	
	int lastSelectedTexture;
};

inline void HudRenderer::toggleVisibility() {
	hidden = !hidden;
}

template <class T>
inline T HudRenderer::scl(T x) const {
	return (T)((float)x * scaleFactor);
}

inline float HudRenderer::getScaleFactor() const {
	return scaleFactor;
}

} /* namespace as */
#endif /* HUDRENDERER_HPP_ */
