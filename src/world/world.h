#pragma once

#include "AL/al.h"

#include "glm/glm.hpp"

#include "inttypes.h"
#include <string>
#include <vector>

class GLFWwindow;
class Sky;
class Terrain;
class TreeManager;
class GrassManager;
class ParticleManager;
class ParticleConfig;
class DroneManager;
class Sign;
class Player;
class HUD;

class Object;
class AABBCollider;
class CylinderCollider;

class Image;

class World {
private:
	static const int SHADOW_MAP_SIZE;						// how big we want the terrain static shadow map to be

	static const float BULLET_RANGE;						// how far should the player's bullet travel

	GLFWwindow *window;										// handle to OpenGL window (required for input and a few other things)
	Sky *sky;												// handle to skydome object
	Terrain *terrain;										// handle to ever-important terrain object
	Image *shadows;											// image we build to use as the terrain shadow map
	TreeManager *trees;										// this handles and renders all of our trees
	GrassManager *grass;									// this handles and renders all of the grass
	ParticleManager *particles;								// this handles and renders all of our particle effects
	DroneManager *drones;									// this handles and renders...you guessed it!---our drones!
	Sign *sign;												// single, one-off object that sits in the middle of nowhere

	Player *player;											// do I really have to explain these two?
	HUD *hud;

	glm::vec2 worldSize;									// basically, the width and length of the terrain in meters

	int numGarbageItems;									// how many items we need to remove from play (like dead drones)

	float deathTimer;										// controls delay before quit, after the player is killed
	bool gameDone;											// can we exit the main loop?

	std::vector<Object*> rayCollidables;					// list of all objects that can be shot by the player
	std::vector<AABBCollider*> aabbs;						// list of axis-aligned bounding boxes we can collide with
	std::vector<CylinderCollider*> cylinders;				// list of upwards-facing cylinders we can collide with

	ALuint ambience;										// OpenAL buffer object for meadow ambience

	glm::mat4 perspectiveProjection;						// 4x4 mat describing a perspective projection (for the player's view)
	glm::mat4 perspectiveView;								// 4x4 mat describing how the perspective camera is oriented

	glm::mat4 orthoProjection;								// 4x4 mat describing an orthographic projection (for the HUD)
	glm::mat4 orthoView;									// 4x4 mat describing how the orthographic camera is oriented

	// initialize the world based on the given file, and the player, too
	void createWorld(std::string worldFile);
	void createPlayer(GLFWwindow *window, glm::vec2 windowSize);

	// set up our camera matrices
	void preparePerspectiveCamera(glm::vec2 windowSize);
	void prepareOrthoCamera(glm::vec2 windowSize);

	// uses the player's look orientation to determine where the camera should face
	void controlCamera();

	// handle player interaction with some collidable objects
	void handlePlayerCollisionWithCylinders();
	void handlePlayerCollisionWithAABBS();
	void controlPlayerDeath(float dt);

	// we don't need to update all of the drones all the time for collision purposes, only the ones we're facing directly
	void updateRayCollidablesInCylinder(glm::vec3 &p1, glm::vec3 &p2, float lengthSquared, float radiusSquared);

	// bullet interaction with terrain and other objects
	bool getTerrainCollision(glm::vec3 bulletStart, glm::vec3 bulletDir, glm::vec3 &intersect, float &distance);
	Object *getObjectsCollision(glm::vec3 bulletStart, glm::vec3 bulletDir, glm::vec3 &intersect, float &distance);

	// add objects to the environment
	void addDrone(glm::vec3 pos);
	void addTree(glm::vec3 pos);

	// add collision objects to the environment
	void addRayCollidable(Object *obj);
	void addAABB(glm::vec3 pos, glm::vec3 size);
	void addCylinder(glm::vec3 pos, float radius, float height);

public:
	static const glm::vec3 SUN_DIRECTION;

	World(GLFWwindow *window, glm::vec2 windowSize, std::string worldFile);
	~World();

	// main updating and rendering
	void update(float dt);
	void render();

	void addGarbageItem();					// indicate we want something removed
	void flushGarbage();					// take out the trash! are there any objects we need to get rid of?

	// create a particle of the given type and add it to the particle system the world owns
	void addParticle(ParticleConfig *config, glm::vec3 pos, float lifeFactor = 1.0);

	// bullet control and interaction with the environment
	void fireBullet(glm::vec3 pos, glm::vec3 direction);
	bool raycast(glm::vec3 start, glm::vec3 end, glm::vec3 &intersect);

	// used to slide drones and players around other objects in the world
	bool getAABBCollision(glm::vec3 point, glm::vec3 *newPoint);
	bool getCylinderCollision(glm::vec3 point, glm::vec3 *newPoint, float *closestDist);

	// get the height of the terrain at the given point in 3D space
	float getTerrainHeight(glm::vec3);

	// get player status
	glm::vec3 getPlayerPos();
	bool isPlayerAlive();

	// conveniently used to locate the pixel coordinates of the shadow map given a 3D coordinate
	unsigned int getShadowMapX(float x);
	unsigned int getShadowMapY(float y);

	// used to cheaply shadow grass underneath trees or signage, by getting an average of nearby
	// pixels from the shadow map at the given 3D position
	uint8_t getShadowValue(glm::vec3 pos);

	// allows main loop to quit
	bool isGameDone();
};
