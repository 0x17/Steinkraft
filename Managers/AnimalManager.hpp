// AnimalManager.hpp

#ifndef ANIMAL_MANAGER_HPP
#define ANIMAL_MANAGER_HPP

#include <cstdlib>
#include <list>

#include "../Framework/Math/Vector.hpp"

#include "../Framework/Camera.hpp"
#include "../Framework/Math/Intersector.hpp"
#include "../Framework/Math/Frustum.hpp"

#include "../Rendering/Meshes/CubeVertices.hpp"

#include "../Framework/Mesh.hpp"

#include "../Terrain.hpp"


namespace as {
//===========================================================================
// Constants
//===========================================================================
const float ANIMAL_SCALE = 0.5f;
const float ANIMAL_SCALEH = (ANIMAL_SCALE / 2.0f);

//===========================================================================
// Types
//===========================================================================
class Animal {
public:
	enum AnimalType {
		AT_PIG,
		AT_DOG,
		NUM_ANIMALS
	};
	
	Animal(float x = 0.0f, float y = 0.0f, float z = 0.0f, AnimalType atype = AT_DOG);
	Animal(Animal const *o);
	bool inChk(int cmx, int cmz) const;
	void update(ticks_t delta);
	bool canFall();

	BoundingBox getBoundingBox();
	std::vector<float> *genTriangles();

	void hit();

	static void initTriCoords(float *coords, int numCoords);

	bool updateDeathAnim(ticks_t delta);

	enum Consts {
		TRIS_SIZE = FACES_PER_BOX * VERTICES_PER_QUAD * NUM_POS_COORD_COMPS,
		DEATH_ANIM_TICKS = 2000,
		PIG_TEX_INDEX = 63,
		HIT_DAMAGE = 25
	};

	static Terrain *t;
	static float triBase[TRIS_SIZE];

	Vec3 pos;
	float rot, destRot, fallVel;
	ticks_t lastDestRotChange;
	bool falling;

	AnimalType atype;

	short health;

	char name[32];

	bool visible;

	bool dying;
	float rotZ;
};

struct AnimalOverlay {
	Vec2 pos;
	char text[64];

	AnimalOverlay(Vec2 _pos, const char * _text);
};

class AnimalManager {
public:
	AnimalManager(Camera *cam, Terrain *t);
	~AnimalManager();
	
	void spawnAnimals();

	void update(ticks_t delta);
	void addAnimalAtPos(float x, float y, float z);
	void renderAnimalsInChk(int cmx, int cmz);

	void renderPigAtPos(float x, float y, float z, float yaw);

	bool tryToHitAnimal(int tX, int tY, float distToNearestAnimal);

	std::list<AnimalOverlay> *genOverlays();
	
	void loadFromFile(const char *filename);
	void saveToFile(const char *filename) const;
	
	static void setupAnimalMeshes(float brightness = 1.0f);
	
private:
	
	void renderAnimal(Animal *p);
	
	void releaseAnimals();
	
	std::list<Animal *> animals;
	static Mesh *animalMeshes[Animal::NUM_ANIMALS];
	Camera *cam;
	Vec3 *camPos;	
	Terrain *terrain;
	
	enum Consts {
		MAX_HIT_DISTANCE = 10,
		MAX_ANIMALS = 64,
		NUM_INIT_ANIMALS = 16
	};	
};

inline void AnimalManager::addAnimalAtPos(float x, float y, float z) {
	if (animals.size() < MAX_ANIMALS)
		animals.push_back(new Animal(x, y, z, /*atype*/ (Animal::AnimalType)(rand() % Animal::NUM_ANIMALS)));
}

}

#endif // PIG_MANAGER_HPP
