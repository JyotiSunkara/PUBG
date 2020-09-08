#include "world.h"
#include "sky.h"
#include "terrain.h"
#include "grassmanager.h"

#include "soundmanager.h"

#include "player.h"
#include "hud.h"
#include "sign.h"
#include "tree.h"
#include "treemanager.h"
#include "drone.h"
#include "dronemanager.h"
#include "complexcollider.h"
#include "aabbcollider.h"
#include "cylindercollider.h"

#include "particlelist.h"
#include "particleconfig.h"
#include "particlemanager.h"

#include "math.h"
#include "image.h"
#include "planerenderer.h"
#include "profiling.h"

#include "lodepng.h"					// for world file loading

#include "glmmodel.h"

#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtx/rotate_vector.hpp"
using namespace glm;

#include <string>
#include <iostream>
#include <vector>
using namespace std;

const vec3 World::SUN_DIRECTION(normalize(vec3(0.288, 1.2, 2.2)));			// where the sun comes from

const float World::BULLET_RANGE = 500.0;									// how far a bullet should fire

const int World::SHADOW_MAP_SIZE = 2048;									// size of terrain shadow map, in pixels

World::World(GLFWwindow *window, vec2 windowSize, string worldFile)
{
	preparePerspectiveCamera(windowSize);
	prepareOrthoCamera(windowSize);

	createPlayer(window, windowSize);
	createWorld(worldFile);

	deathTimer = 0.0;
	gameDone = false;

	numGarbageItems = 0;
}

void World::preparePerspectiveCamera(vec2 windowSize)
{
	const float FOV = 45.0f;
	const float ASPECT_RATIO = (float)windowSize.x / (float)windowSize.y;
	const float PERSPECTIVE_NEAR_RANGE = 0.1f;
	const float PERSPECTIVE_FAR_RANGE = 8000.0f;

	// compute our perspective projection---this never changes so we can just do it once
	perspectiveProjection = perspective(FOV, ASPECT_RATIO, PERSPECTIVE_NEAR_RANGE, PERSPECTIVE_FAR_RANGE);
}

void World::prepareOrthoCamera(vec2 windowSize)
{
	const float ORTHO_NEAR_RANGE = -1.0;
	const float ORTHO_FAR_RANGE = 1.0;

	// compute our ortho projection and view---these never change so we can just do it once
	orthoProjection = ortho(0.0f, windowSize.x, 0.0f, windowSize.y, ORTHO_NEAR_RANGE, ORTHO_FAR_RANGE);
	orthoView = mat4(1.0);
}

void World::createWorld(string worldFile)
{
	const float MAX_TERRAIN_HEIGHT = 800.0;							// how high should the highest point on the terrain be?
	const float TERRAIN_TILE_SIZE = 10.0;							// each terrain tile is 10mx10m

	const int MAX_BLADES_OF_GRASS = 850000;							// how many blades of grass we want to render...lots!
	const float GRASS_AREA_RADIUS = 80.0;							// size of circular grass around we wrap around the player

	const float DRONE_MIN_DIST = 50.0;								// minimum drone starting 2D distance from player

	const int MAX_PARTICLES = 5000;									// maximum number of particles we want in the world

	const int NUM_DRONES = 180;										// how many drones to insert into the world

	// instructional sign position
	const vec3 SIGN_PLAYER_OFFSET(-3.5, 0.0, -6.5);
	const vec3 SIGN_POS = player -> getPos() + SIGN_PLAYER_OFFSET;
	const vec3 SIGN_AABB_SIZE(6.5, 5.0, 2.0);
	const float SIGN_ANGLE = 0.0;
	vec3 signPos;

	unsigned int width, length;												// number of tiles wide and long the world is
    unsigned char *data;											// data read from world file
    unsigned char *dataPtr;
    unsigned char pixelData[4];

    float *heights;
    float *heightPtr;
    unsigned int i, j;

    vector<vec3> treeList;
    vector<vec3>::iterator v;

	// construct our terrain object from file
	lodepng_decode32_file(&data, &width, &length, worldFile.c_str());

	dataPtr = data;
	heights = new float[width * length];
	heightPtr = heights;

	// record the size of the world for later on
	worldSize = vec2(width * TERRAIN_TILE_SIZE, length * TERRAIN_TILE_SIZE);

	// gather up the heights for our terrain module
	for(i = 0; i < length; i ++)
	{
		for(j = 0; j < width; j ++)
		{
			// read the current pixel colour
			memcpy(pixelData, dataPtr, sizeof(unsigned char) * 4);
			dataPtr += 4;

			// red pixel tells us the height of the terrain
			*heightPtr++ = ((float)pixelData[0] / 255.0) * MAX_TERRAIN_HEIGHT;

			// presence of a green pixel tells us a tree should be there
			if(pixelData[1] == 255)
			{
				// pure green is a tree
				treeList.push_back(vec3(j * TERRAIN_TILE_SIZE, 0.0, -(int)i * TERRAIN_TILE_SIZE));
			}
		}
	}

	// create our base shadow image for trees and signage
	shadows = new Image(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

	// add sign shadow
	shadows -> setRectangle(getShadowMapX(SIGN_POS.x) - 1, getShadowMapY(SIGN_POS.z), 2, 2, 170);

	// construct our terrain object now that we know the heights of every single vertex
	terrain = new Terrain(this, width, length, TERRAIN_TILE_SIZE, heights);

	// now insert the trees we recorded---there's probably a more graceful way of doing this without iterating through
	// each tree twice, and I'll probably optimize later (this is not the bottleneck when loading, however, so it's not
	// terribly important right now)
	trees = new TreeManager(this, treeList.size());
	for(v = treeList.begin(); v != treeList.end(); v ++)
	{
		v -> y = getTerrainHeight(*v);
		addTree(*v);
	}
	trees -> finalizeTreePlacement();

	// now that our objects are added, we can assign the shadow texture to the terrain
	terrain -> setShadowTexture(shadows -> makeGLTexture());

	// by the way, add some grass, too, while we're at it
	grass = new GrassManager(this, player, MAX_BLADES_OF_GRASS, GRASS_AREA_RADIUS);

	// create the skydome
	sky = new Sky();

	// start up the particle manager
	particles = new ParticleManager(MAX_PARTICLES);

	// create the no drone sign
	signPos = vec3(SIGN_POS.x, getTerrainHeight(SIGN_POS), SIGN_POS.z);
	sign = new Sign(this, signPos, SIGN_ANGLE);
	addAABB(signPos + vec3(0.0, SIGN_AABB_SIZE.y / 2.0f, 0.0), SIGN_AABB_SIZE);
	rayCollidables.push_back(sign);

	// create the drones themselves
	drones = new DroneManager(this, NUM_DRONES);
	for(i = 0; i < NUM_DRONES; i ++)
	{
		float angle = linearRand(-M_PI, M_PI);
		float dist = linearRand(DRONE_MIN_DIST, worldSize.x / 2.0f);
		addDrone(player -> getPos() + vec3(sin(angle) * dist, 0.0, cos(angle) * dist));
	}

	// create the list of particle configurations we'll be using
	initParticleList();

	// start the ambient meadow sound effect
	ambience = SoundManager::getInstance() -> loadWAV("../wav/ambience.wav");
	SoundManager::getInstance() -> loopSound(ambience);

	// the grass manager can now start up the thread that wraps the grass around the player as they move
	grass -> beginWrapThread();

	// we can now free up the memory we used for our initialization
	delete heights;
	delete[] data;
}

void World::createPlayer(GLFWwindow *window, vec2 windowSize)
{
	const vec3 STARTING_POS(2560.0, 0.0, -2560.0);
	player = new Player(window, this, STARTING_POS);
	hud = new HUD(player, orthoProjection, orthoView, windowSize);
}

void World::controlCamera()
{
	perspectiveView = lookAt(player -> getPos(), player -> getCameraLook(), vec3(0.0, 1.0, 0.0));
}

World::~World()
{
	vector<AABBCollider*>::iterator i;
	vector<CylinderCollider*>::iterator j;

	// remove axis-aligned bounding boxes from memory
	i = aabbs.begin();
	while(i != aabbs.end())
	{
		delete *i;
		i = aabbs.erase(i);
	}

	// remove axis-aligned cylinders from memory
	j = cylinders.begin();
	while(j != cylinders.end())
	{
		delete *j;
		j = cylinders.erase(j);
	}

	// complex colliders are removed by the objects that own them, when the owning parents' destructors are called

	// remove particle types from memory
	deinitParticleList();

	// delete master objects
	delete sign;
	delete particles;
	delete sky;
	delete grass;
	delete terrain;
	delete trees;
	delete hud;
	delete player;
	delete drones;

	// shut down singleton instances
	delete SoundManager::getInstance();
	delete PlaneRenderer::getInstance();
}

void World::update(float dt)
{
	// remove anything that needs removal
	flushGarbage();

	// update the objects in the world
	player -> update(dt);
	grass -> update(dt);
	drones -> update(dt);

	// has an update caused the player to die?---if so, deal with that
	controlPlayerDeath(dt);

	// remove any expired particles, and update any existing ones
	particles -> recycle();
	particles -> update(dt);

	// if the player collided with anything, see that it's dealt with
	handlePlayerCollisionWithCylinders();
	handlePlayerCollisionWithAABBS();

	// we can update the camera and gun positions now that the player is fully up-to-date
	player -> computeCameraOrientation();
	player -> computeGunPosition();

	// re-build our perspective view (our projection stays the same, of course)
	controlCamera();
}

void World::render()
{
	mat4 modelMat = mat4(1.0);
	vec3 cameraSide = player -> getCameraSide();
	vec3 cameraUp = player -> getCameraUp();
	vec3 playerPos = player -> getPos();

	// a whole lotta rendering going on here
	sky -> render(perspectiveProjection, perspectiveView, playerPos);
	terrain -> render(perspectiveProjection, perspectiveView, modelMat);
	trees -> render(perspectiveProjection, perspectiveView, modelMat);
	grass -> render(perspectiveProjection, perspectiveView, modelMat);
	player -> render(perspectiveProjection, perspectiveView);
	sign -> render(perspectiveProjection, perspectiveView);
	drones -> render(perspectiveProjection, perspectiveView);
	particles -> render(perspectiveProjection, perspectiveView, cameraSide, cameraUp);
	hud -> render();
}

void World::addGarbageItem()
{
	numGarbageItems ++;
}

void World::flushGarbage()
{
	vector<Object*>::iterator i = rayCollidables.begin();
	int j = 0;

	while(j < numGarbageItems && i != rayCollidables.end())
	{
		if((*i) -> flaggedAsGarbage())
		{
			i = rayCollidables.erase(i);
			j ++;
		}
		else
		{
			i ++;
		}
	}

	numGarbageItems = 0;
}

void World::controlPlayerDeath(float dt)
{
    const float DRONE_DEATH_DIST = 1.0;

    const float FADE_OUT_START = 0.5;
    const float FADE_OUT_TIME = 2.0;

    // one collision against a drone is enough to kill the player
    if(drones -> isDroneCloseTo(player -> getPos(), DRONE_DEATH_DIST) && player -> isAlive())
    {
		player -> die();
		hud -> enableBlood(true);
    }

    // if the player is dead, fade out and then quit
    if(!player -> isAlive())
    {
        deathTimer += dt;
        if(deathTimer > FADE_OUT_START)
        {
			hud -> setFade((deathTimer - FADE_OUT_START) / FADE_OUT_TIME);
			if(deathTimer > FADE_OUT_START + FADE_OUT_TIME)
			{
				gameDone = true;
			}
        }
    }
}

void World::handlePlayerCollisionWithCylinders()
{
	vec3 playerPos = player -> getPos();
	vec3 newPlayerPos;
	float oldPlayerY = playerPos.y;
	float dist;

	if(player -> getIsMoving() && getCylinderCollision(playerPos, &newPlayerPos, &dist))
	{
		newPlayerPos.y = oldPlayerY;
		player -> setPos(newPlayerPos);
	}
}

void World::handlePlayerCollisionWithAABBS()
{
	vec3 playerPos = player -> getPos();
	vec3 newPlayerPos;
	float oldPlayerY = playerPos.y;

	if(getAABBCollision(playerPos, &newPlayerPos))
	{
		newPlayerPos.y = oldPlayerY;
		player -> setPos(newPlayerPos);
	}
}

void World::addParticle(ParticleConfig *config, vec3 pos, float lifeFactor)
{
	particles -> add(config, pos, lifeFactor);
}

void World::updateRayCollidablesInCylinder(vec3 &p1, vec3 &p2, float lengthSquared, float radiusSquared)
{
	vector<Object*>::iterator i;
	Object *curr;
	vec3 objectPos;

	for(i = rayCollidables.begin(); i != rayCollidables.end(); i ++)
	{
		curr = *i;
		objectPos = curr -> getPos();
		if(isPointInCylinder(p1, p2, lengthSquared, radiusSquared, objectPos))
		{
			curr -> updateComplexCollider();
		}
	}
}

void World::fireBullet(vec3 bulletStart, vec3 bulletDir)
{
	const float BULLET_RANGE_SQUARED = BULLET_RANGE * BULLET_RANGE;

	const float BULLET_CYLINDER = 25.0;
	const float BULLET_CYLINDER_SQUARED = BULLET_CYLINDER * BULLET_CYLINDER;

	vec3 bulletEnd = bulletStart + bulletDir * BULLET_RANGE;

	// update the colliders for any objects that might be hit according to a quick cylinder test
	updateRayCollidablesInCylinder(bulletStart, bulletEnd, BULLET_RANGE_SQUARED, BULLET_CYLINDER_SQUARED);

	vec3 terrainIntersect;
	float terrainDistance;

	vec3 objectIntersect;
	float objectDistance;

	vec3 impactPoint;

	bool terrainCollision = getTerrainCollision(bulletStart, bulletDir, terrainIntersect, terrainDistance);
	Object *objectCollision = getObjectsCollision(bulletStart, bulletDir, objectIntersect, objectDistance);

	// if both types of collisions occurred, we deal with whichever is closest to the player
	if(terrainCollision && objectCollision)
	{
		if(terrainDistance < objectDistance)
		{
			objectCollision = NULL;
			impactPoint = terrainIntersect;
		}
		else
		{
			terrainCollision = false;
			impactPoint = objectIntersect;
		}
	}
	else if(terrainCollision)
	{
		impactPoint = terrainIntersect;
	}
	else if(objectCollision)
	{
		impactPoint = objectIntersect;
	}

	// if either occurred, we need to add an impact effect
	if(terrainCollision || objectCollision)
	{
		// add a quick, bright flash of impact
		addParticle(impactFlare, impactPoint);

		// and then add some smoke and dirt, too
		for(int i = 0; i < 10; i ++)
		{
			if(terrainCollision)
			{
				addParticle(dirtSpray, impactPoint);
			}
			addParticle(smoke, impactPoint);
		}

		// if we hit an object, make it handle the collision
		if(objectCollision)
		{
			objectCollision -> handleRayCollision(normalize(objectCollision -> getPos() - bulletStart), impactPoint);
		}
	}
}

Object *World::getObjectsCollision(vec3 start, vec3 dir, vec3 &intersect, float &distance)
{
	vector<Object*>::iterator i;
	vec3 diff;

	Object *curr;
	Object *closestCollider = NULL;
	float currentDist;
	float closestDist = FLT_MAX;

	// iterate through all of our objects that have colliders attached and see if we hit
	// any of them with our ray
	for(i = rayCollidables.begin(); i != rayCollidables.end(); i ++)
	{
		curr = *i;
		if(curr -> collidesWithRay(start, dir, BULLET_RANGE, intersect))
		{
			diff = intersect - start;
			currentDist = (diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z);
			if(currentDist < closestDist)
			{
				closestDist = currentDist;
				closestCollider = curr;
			}
		}
	}

	// record our distance to the collider, if we have one
	if(closestCollider)
	{
		distance = sqrt(closestDist);
	}

	return closestCollider;
}

bool World::getTerrainCollision(vec3 start, vec3 dir, vec3 &intersect, float &distance)
{
	bool result = terrain -> raycast(start, start + (dir * BULLET_RANGE), intersect);

	// if there is a collision, we need to record the distance
	if(result)
	{
		distance = length(start - intersect);
	}

	return result;
}

bool World::getAABBCollision(vec3 point, vec3 *newPoint)
{
	vector<AABBCollider*>::iterator i = aabbs.begin();
	bool result = false;

	while(!result && i != aabbs.end())
	{
		result = (*i++) -> testSlidingCollision(point, newPoint);
	}

	return result;
}

bool World::getCylinderCollision(vec3 point, vec3 *newPoint, float *closestDist)
{
	vector<CylinderCollider*>::iterator i = cylinders.begin();
	bool result = false;
	float dist = FLT_MAX;

	*closestDist = dist;
	while(!result && i != cylinders.end())
	{
		result = (*i++) -> testSlidingCollision(point, newPoint, &dist);
		if(dist < *closestDist)
		{
			*closestDist = dist;
		}
	}

	return result;
}

void World::addDrone(vec3 pos)
{
	Drone *drone;

	// align the drone with the ground
	pos.y = getTerrainHeight(pos) + 10.0;

	// add the drone to the system and the collision handler
	drone = drones -> addDrone(pos);
	rayCollidables.push_back(drone);
}

void World::addTree(vec3 pos)
{
	const float TREE_CYLINDER_HEIGHT = 20.0;
	const float TREE_CYLINDER_RADIUS = 1.0;

	Tree *tree;

	// align the tree with the ground
	pos.y = getTerrainHeight(pos);

	// add the tree to the system and the collision handler
	tree = trees -> addTree(pos);
	rayCollidables.push_back(tree);
	addCylinder(pos + vec3(0.0, TREE_CYLINDER_HEIGHT / 2.0f, 0.0), TREE_CYLINDER_RADIUS, TREE_CYLINDER_HEIGHT);

	// add a shadow to the terrain shadow map where this tree is
	shadows -> setCircle(getShadowMapX(pos.x), getShadowMapY(pos.z), 3, 170);
}

void World::addAABB(vec3 pos, vec3 size)
{
	aabbs.push_back(new AABBCollider(pos, size));
}

void World::addCylinder(vec3 pos, float radius, float height)
{
	cylinders.push_back(new CylinderCollider(pos, radius, height));
}

float World::getTerrainHeight(vec3 pos)
{
	return terrain -> getHeight(pos);
}

vec3 World::getPlayerPos()
{
	return player -> getPos();
}

bool World::isPlayerAlive()
{
	return player -> isAlive();
}

unsigned int World::getShadowMapX(float x)
{
	return (x / worldSize.x) * SHADOW_MAP_SIZE;
}

unsigned int World::getShadowMapY(float z)
{
	return (-z / worldSize.y) * SHADOW_MAP_SIZE;
}

uint8_t World::getShadowValue(glm::vec3 pos)
{
	unsigned int x = getShadowMapX(pos.x);
	unsigned int y = getShadowMapY(pos.z);
	float value = 0.0;

	value += shadows -> getPixel(x, y);
	value += shadows -> getPixel(x + 1, y + 1);
	value += shadows -> getPixel(x + 1, y - 1);
	value += shadows -> getPixel(x - 1, y + 1);
	value += shadows -> getPixel(x - 1, y - 1);

	return value / 5.0;
}

bool World::isGameDone()
{
	return gameDone;
}
