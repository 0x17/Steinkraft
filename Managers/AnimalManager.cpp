// AnimalManager.cpp

// TODO: Make sure pigs/animals can't fall from map borders (<0,>MAP_X, ..., <0,>MAP_Z)!



#include <cstdlib>
#include <ctime>
#include <cmath>

#include "AnimalManager.hpp"
#include "../Rendering/Voxelrenderer/ChunkMeshRenderer.hpp"
#include "../Rendering/Meshes/CubeVertices.hpp"

namespace as {
//===========================================================================
// Constants / Macros
//===========================================================================
const ticks_t ROT_CHG_FREQ = 10000;

const float STOP_DIST = 1.0f;
const float ANIMAL_SPEED = 0.00025f;
const float ROT_ADAPTION_SPEED = 2.0f;
const float ANIMAL_FALL_ACCEL = 0.000981f;

const float MAX_FSPEED_PER_TICK = -0.9999f;

const float DEATH_ROT_SPEED = 0.125f;

// Animal names ===============================================================
const int NUM_DOG_NAMES = 6;
static const char *dogNames[NUM_DOG_NAMES] = {
	"Lucky",
	"Leon",
	"Keanu",
	"Kentucky",
	"Brian",
	"Laika",
};

const int NUM_PIG_NAMES = 5;
static const char *pigNames[NUM_PIG_NAMES] = {
	"Porkchop",
	"Hamlet",
	"Piglet",
	"Piggy",
	"Porky"
};

//===========================================================================
// Globals/Statics
//===========================================================================
Terrain *Animal::t;
float Animal::triBase[TRIS_SIZE];
Mesh *AnimalManager::animalMeshes[Animal::NUM_ANIMALS];

//===========================================================================
// Methods/Types
//===========================================================================
AnimalOverlay::AnimalOverlay( Vec2 _pos, const char * _text ) : pos(_pos)
{
	strcpy(text, _text);
}

inline bool Animal::canFall() {
	return t->isEmptyPos(pos.x, pos.y - 1.0f, pos.z);
}

Animal::Animal(float _x, float _y, float _z, AnimalType _atype)
:	pos(_x,_y,_z),
	rot((float)(rand() % 360)),
	destRot(rot),
	fallVel(0.0f),
	lastDestRotChange(0),
	falling(false),
	atype(_atype),
	health(100),
	visible(false),
	dying(false),
	rotZ(0.0f)
{
	strcpy(name, _atype == Animal::AT_PIG ? pigNames[rand() % NUM_PIG_NAMES] : dogNames[rand() % NUM_DOG_NAMES]);
}

Animal::Animal(Animal const *o)
:	pos(o->pos),
	rot(o->rot),
	destRot(o->rot),
	fallVel(o->fallVel),
	lastDestRotChange(o->lastDestRotChange),
	falling(false),
	atype(o->atype),
	health(100),
	visible(false),
	dying(false),
	rotZ(0.0f) {
	strcpy(name, o->name);
}

bool Animal::inChk(int cmx, int cmz) const {
	int chunkMinX = cmx * ChunkMeshRenderer::CHUNK_X_SIZE;
	int chunkMaxX = (cmx + 1) * ChunkMeshRenderer::CHUNK_X_SIZE;
	int chunkMinZ = cmz * ChunkMeshRenderer::CHUNK_Z_SIZE;
	int chunkMaxZ = (cmz + 1) * ChunkMeshRenderer::CHUNK_Z_SIZE;

	return (pos[0] >= chunkMinX && pos[2] >= chunkMinZ && pos[0] < chunkMaxX && pos[2] < chunkMaxZ);
}

void Animal::update(ticks_t delta) {
	float psd = ANIMAL_SPEED * delta;

	if (canFall()) {
		falling = true;
		pos.y += fallVel < MAX_FSPEED_PER_TICK ? MAX_FSPEED_PER_TICK : fallVel;
		fallVel -= ANIMAL_FALL_ACCEL * delta;
	} else if (falling) {
		pos.y = (int)pos.y + 0.5f;
		falling = false;
	}

	if (getTicks() - lastDestRotChange > ROT_CHG_FREQ) {
		destRot = (float)(rand() % 365);
		lastDestRotChange = getTicks();
	}

	if (destRot - rot > 1.0f) {
		rot += ROT_ADAPTION_SPEED;
	} else if (rot - destRot > 1.0f) {
		rot -= ROT_ADAPTION_SPEED;
	}

	Vec3 oldPos = pos;

	float radRot = deg2rad(rot);
	pos.x += sinf(radRot) * psd;
	pos.z += cosf(radRot) * psd;

	if (!t->isEmptyPos(pos) || canFall()) {
		pos = oldPos;
		destRot = rot + 180.0f;
		lastDestRotChange = getTicks();
	}
}

void Animal::initTriCoords( float *coords, int numCoords )
{
	const int NUM_COMPS = (NUM_POS_COORD_COMPS + NUM_TX_COORD_COMPS + NUM_COL_COORD_COMPS);
	assert(numCoords == FACES_PER_BOX * VERTICES_PER_QUAD * NUM_COMPS);
	
	const float TRI_SCL_FACTOR = 1.5f;

	for(int i=0; i<TRIS_SIZE / 3; i++) {
		triBase[3*i+0] = coords[NUM_COMPS*i+0] * TRI_SCL_FACTOR;
		triBase[3*i+1] = coords[NUM_COMPS*i+1] * TRI_SCL_FACTOR;
		triBase[3*i+2] = coords[NUM_COMPS*i+2] * TRI_SCL_FACTOR;
	}
}

BoundingBox Animal::getBoundingBox()
{
	Vec3 offsetToOrigin(ANIMAL_SCALE/2, ANIMAL_SCALE/2, ANIMAL_SCALE/2);
	return BoundingBox(pos - offsetToOrigin, pos + offsetToOrigin);
}

std::vector<float> *Animal::genTriangles()
{
	std::vector<float> *tris = new std::vector<float>(TRIS_SIZE);
	for(int i=0; i<TRIS_SIZE / 3; i++) {
		(*tris)[3*i+0] = triBase[3*i+0] + pos.x;
		(*tris)[3*i+1] = triBase[3*i+1] + pos.y - ANIMAL_SCALEH;
		(*tris)[3*i+2] = triBase[3*i+2] + pos.z;
	}

	return tris;
}

void Animal::hit()
{
	health -= HIT_DAMAGE;

	if(health <= 0) {
		dying = true;
	}
}

bool Animal::updateDeathAnim( ticks_t delta )
{
	if(rotZ > 90.0f) {
		return true;
	}

	rotZ += delta * DEATH_ROT_SPEED;

	return false;
}

//====================================================================
// Animal manager
//====================================================================
AnimalManager::AnimalManager(Camera *_cam, Terrain *_t)
:	cam(_cam),
	camPos(_cam->getPosPtr()),
	terrain(_t)
{
	Animal::t = _t;

	memset(animalMeshes, 0, sizeof(Mesh *) * Animal::NUM_ANIMALS);
	
	setupAnimalMeshes();
}

void AnimalManager::spawnAnimals() {
	if(noAnimals) return;
	
	for(int i=0; i<NUM_INIT_ANIMALS; i++) {
		Vec3 pos;
		
		if(i < NUM_INIT_ANIMALS / 2) { // put half of them close to cam
			pos.x = camPos->x + (float)(rand() % (Terrain::CHUNK_SIZE * 2) - Terrain::CHUNK_SIZE);
			pos.y = (float)(Terrain::MAX_Y);
			pos.z = camPos->z + (float)(rand() % (Terrain::CHUNK_SIZE * 2) - Terrain::CHUNK_SIZE);
		} else
			pos = Vec3((float)(rand() % Terrain::MAX_X), (float)Terrain::MAX_Y, (float)(rand() % Terrain::MAX_Z));
		
		pos.y = (float)terrain->getYOfBlockBelow((int)pos.x, (int)pos.y, (int)pos.z) + 2;
		if(pos.y >= Terrain::MAX_Y) { i = i > 0 ? i-1 : i; continue; }
		
		animals.push_back(new Animal(pos.x, pos.y, pos.z, (Animal::AnimalType)(rand() % Animal::NUM_ANIMALS)));
	}
}

AnimalManager::~AnimalManager() {
	for(int i=0; i<Animal::NUM_ANIMALS; i++)
		SAFE_DELETE(animalMeshes[i]);
		
	releaseAnimals();
}

void AnimalManager::releaseAnimals() {
	for(std::list<Animal *>::iterator it = animals.begin(); it != animals.end(); ++it) {
		SAFE_DELETE((*it));
	}
	animals.clear();
}

void AnimalManager::setupAnimalMeshes(float brightness) {
	for(int i=0; i<Animal::NUM_ANIMALS; i++)
		SAFE_DELETE(animalMeshes[i]);
	
	const int numCoords = VERTICES_PER_QUAD * FACES_PER_BOX * (3 + 2 + 4);
	float *coords = new float[numCoords];
	TexCell *texCells[FACES_PER_BOX];

	// Init pig mesh =====================================================
	// front: face
	texCells[0] = new TexCell(7, 7);
	// back: tail
	texCells[1] = new TexCell(1, 9);
	// left/right/top/bottom: sides
	texCells[2] = texCells[3] = texCells[4] = texCells[5] = new TexCell(0, 9);
	genPosTxCoordCubeVertices(texCells, FACES_PER_BOX, coords, ANIMAL_SCALE, true, brightness);
	animalMeshes[Animal::AT_PIG] = new Mesh();
	animalMeshes[Animal::AT_PIG]->setVertices(coords, numCoords);
	SAFE_DELETE(texCells[0]);
	SAFE_DELETE(texCells[1]);
	SAFE_DELETE(texCells[2]);

	// Init dog mesh =====================================================
	// front: face
	texCells[0] = new TexCell(1, 11);
	// back: tail
	texCells[1] = new TexCell(3, 11);
	// left/right/top/bottom: sides
	texCells[2] = texCells[3] = texCells[4] = texCells[5] = new TexCell(2, 11);
	genPosTxCoordCubeVertices(texCells, FACES_PER_BOX, coords, ANIMAL_SCALE, true, brightness);
	animalMeshes[Animal::AT_DOG] = new Mesh();
	animalMeshes[Animal::AT_DOG]->setVertices(coords, numCoords);
	SAFE_DELETE(texCells[0]);
	SAFE_DELETE(texCells[1]);
	SAFE_DELETE(texCells[2]);

	Animal::initTriCoords(coords, numCoords);

	SAFE_DELETE_ARRAY(coords);
}

void AnimalManager::update(ticks_t delta) {
	std::list<Animal *>::iterator it, it2;
	Animal *p1, *p2;
	for (it = animals.begin(); it != animals.end();) {
		p1 = (*it);

		if(p1->dying) {
			if(p1->updateDeathAnim(delta)) {
				SAFE_DELETE((*it));
				animals.erase(it++);
				continue;
			}
		}

		p1->update(delta);		

		// check for collisions
		for (it2 = animals.begin(); it2 != animals.end(); ++it2) {
			p2 = (*it2);
			if (p1 == p2 || p2->dying) continue; // skip same pig

			// pigs collide -> make them turn in different directions
			if ((p2->pos - p1->pos).length() < 2.0f) {
				p1->destRot = p1->rot + 90.0f;
				p2->destRot = p2->rot - 90.0f;
				p1->lastDestRotChange = p2->lastDestRotChange = getTicks();
			}
		}

		++it;
	}
}

void AnimalManager::renderAnimalsInChk(int cmx, int cmz) {
	std::list<Animal *>::iterator it;
	Frustum *frustum = cam->getFrustumPtr();

	for (it = animals.begin(); it != animals.end(); ++it) {
		Animal *p = (*it);
		if (p->inChk(cmx, cmz)) {
			BoundingBox bbox = p->getBoundingBox();
			p->visible = frustum->boxInFrustum(&bbox);
			if(p->visible) {
				renderAnimal(p);
				p->visible = (p->pos - cam->getPos()).length() < MAX_HIT_DISTANCE;
			}
		}
	}
}

void AnimalManager::renderAnimal(Animal *p) {
	glPushMatrix();
	glTranslatef(p->pos.x, p->pos.y - ANIMAL_SCALEH, p->pos.z);
	glRotatef(p->rot, 0.0f, 1.0f, 0.0f);
	if(p->dying) {
		glRotatef(p->rotZ, 0.0f, 0.0f, 1.0f);
		float sclFac = (90.0f - p->rotZ)/90.0f;
		glScalef(sclFac, sclFac, sclFac);
	}
	animalMeshes[p->atype]->render();	
	glPopMatrix();


#if DEBUG_MODE
	std::vector<float> *tris = p->genTriangles();
	int ntris = tris->size();
	float *triArray = new float[ntris];
	for(int i=0; i<ntris; i++) {
		triArray[i] = (*tris)[i];
	}
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);
	for(int i=0; i<ntris; i+=3) {
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(triArray[i], triArray[i+1], triArray[i+2]);
	}
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	SAFE_DELETE_ARRAY(triArray);
#endif
}

void AnimalManager::renderPigAtPos(float x, float y, float z, float yaw) {
	glPushMatrix();
	glTranslatef(x, y - ANIMAL_SCALEH, z);
	glRotatef(rad2deg(yaw), 0.0f, 1.0f, 0.0f);
	animalMeshes[Animal::AT_PIG]->render();
	glPopMatrix();
}

bool AnimalManager::tryToHitAnimal( int tX, int tY, float distToNearestBlock )
{
	if(animals.empty()) return false;

	Ray pray = cam->getPickRay(tX, tY);
	Vec3 isection;
	std::vector<float> *tris;
	
	for(std::list<Animal *>::iterator it = animals.begin(); it != animals.end(); ++it) {
		Animal *animal = (*it);
		if(animal->visible) {
			tris = animal->genTriangles();
			bool wasHit = intersector::intersectRayTriangles(&pray, tris, &isection);
			SAFE_DELETE(tris);
			if(wasHit && (isection - cam->getPos()).length() < distToNearestBlock) {
				animal->hit();
				return true;
			}
		}
	}

	return false;
}

std::list<AnimalOverlay> *AnimalManager::genOverlays()
{
	std::list<AnimalOverlay> *overlays = new std::list<AnimalOverlay>();
	Vec3 scrPos;

	for(std::list<Animal *>::iterator it = animals.begin(); it != animals.end(); ++it) {
		Animal *animal = (*it);
		if(animal->visible && !animal->dying) {
			cam->project(Vec4(animal->pos), &scrPos);
			char buf[256];
			sprintf(buf, "%s %d", animal->name, animal->health);
			overlays->push_back(AnimalOverlay(Vec2(scrPos), buf));
		}
	}

	return overlays;
}

void AnimalManager::loadFromFile(const char *filename) {
	if(noAnimals) return;
	
	char dataFilename[BUF_LEN], descrFilename[BUF_LEN];
	int l;

	strcpy(dataFilename, filename);
	strcpy(descrFilename, filename);

	strcat(dataFilename, ".animals");
	strcat(descrFilename, ".adescr");

	if (!fileExists(dataFilename) || !fileExists(descrFilename)) return;

	binaryRead(descrFilename, &l, sizeof(int));

	Animal *animalArray = new Animal[l];
	binaryRead(dataFilename, animalArray, sizeof(Animal) * l);

	releaseAnimals();

	for (int i = 0; i < l; i++) {
		animals.push_back(new Animal(&animalArray[i]));
	}

	SAFE_DELETE_ARRAY(animalArray);
}

// TODO: Write better persistency methods (using append? single file for more info? serialization?)

// TODO: Fix memory leak!
void AnimalManager::saveToFile(const char *filename) const {
	if(noAnimals) return;
	
	char dataFilename[BUF_LEN], descrFilename[BUF_LEN];
    int l = static_cast<int>(animals.size());

	strcpy(dataFilename, filename);
	strcpy(descrFilename, filename);

	strcat(dataFilename, ".animals");
	strcat(descrFilename, ".adescr");

	binaryWrite(descrFilename, &l, sizeof(int));

	Animal *animalArray = new Animal[l];

	std::list<Animal *>::const_iterator it;
	int i = 0;
	for (it = animals.begin(); it != animals.end(); ++it) {
		Animal *aptr = (*it);
		//memcpy(&animalArray[i++], aptr, sizeof(Animal));
		animalArray[i++] = Animal(aptr);
	}

	binaryWrite(dataFilename, animalArray, sizeof(Animal) * l);

	SAFE_DELETE_ARRAY(animalArray);
}

}
