#include "objects/drone.h"
#include "objects/hud.h"
int hudScore;

#include "world/world.h"

#include "audio/soundmanager.h"

#include "particles/particlelist.h"

#include "glm/glm.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtx/rotate_vector.hpp"
using namespace glm;

#include <iostream>
using namespace std;

const float Drone::DEFAULT_HEALTH = 30.0;

const float Drone::HOVER_REF_HEAR_DIST = 5.0;
const float Drone::HOVER_MAX_HEAR_DIST = FLT_MAX;

const float Drone::MOVE_SPEED = 3.0;				// in meters per second

Drone::Drone() { }

Drone::Drone(World *world, vec3 pos, ALuint hoverBuffer, ALuint warningBuffer, ALuint explodeBuffer)
{
	this -> world = world;
	setPos(pos);

	// set up our audio
	soundManager = SoundManager::getInstance();
	hoverSource = 0;
	this -> hoverBuffer = hoverBuffer;
	this -> warningBuffer = warningBuffer;
	this -> explodeBuffer = explodeBuffer;

	health = DEFAULT_HEALTH;
	alive = true;

	playedWarning = false;
}

Drone::~Drone()
{

}

void Drone::update(float dt)
{
	moveTowardsPlayer(dt);
	controlImpactMotion(dt);
	controlGroundIntersection();

	computeModelMat();

	controlWarningSound();
	controlHoverSound();
	controlDeath();
}

void Drone::moveTowardsPlayer(float dt)
{
	const float TILT_ANGLE = -M_PI / 16.0f;					// aggressive tilt towards player
	const float UPDATE_ORIENTATION_DISTANCE = 500.0;		// close enough for player to see drone

	const vec3 DRONE_TARGET_ADJUSTMENT(0.0f, -0.3f, 0.0f);

	vec3 pos = getPos();
	vec3 player;
	vec3 offset;

	vec3 forward, up, side;

	// we only need to target the player if they're still alive; otherwise we just move forwards
	if(world -> isPlayerAlive())
	{
		// drone position and player position
		player = world -> getPlayerPos() + DRONE_TARGET_ADJUSTMENT;

		// compute a vector towards the player
		offset = player - pos;
		distanceToPlayer = length(offset);
		velocity = (offset / distanceToPlayer) * MOVE_SPEED * dt;

		// only update the drone's orientation if the player is close enough to see this
		if(distanceToPlayer < UPDATE_ORIENTATION_DISTANCE)
		{
			// tilt the drone slightly in the direction of its movement (towards the player)
			forward = normalize(vec3(velocity.x, 0.0f, velocity.z));
			up = vec3(0.0, 1.0, 0.0);
			side = cross(forward, up);
			up = rotate(up, TILT_ANGLE, side);
			forward = rotate(forward, TILT_ANGLE, side);

			// re-orient towards the player
			setUp(up);
			setSide(side);
			setForward(forward);
		}
    }

    // always move forward
    setPos(pos + velocity);
}

void Drone::controlImpactMotion(float dt)
{
	const float MOTION_DAMPENING = 0.9;

	// scale down our impact push-back, and make it affect our position
	impactMotion = impactMotion * MOTION_DAMPENING;
	setPos(getPos() + impactMotion * dt);
}

void Drone::controlGroundIntersection()
{
	const float LOW_ENOUGH_TO_CHECK = 2.0;
	const float MIN_ROTOR_HEIGHT = 0.5;

	vec3 pos = getPos();
	float terrainHeight = world -> getTerrainHeight(pos);
	float terrainHeightDiffs[4];
	float lowestHeight;

	// if we're reasonably close to the ground, then check all four corners of the drone;
	// this can probably be optimized since the drone is somewhat square-shaped
	if((terrainHeight - pos.y) < LOW_ENOUGH_TO_CHECK)
	{
		// height of roughly where the four motors are
		terrainHeightDiffs[0] = world -> getTerrainHeight(vec3(pos.x - 1.0, 0.0, pos.z - 1.0));
		terrainHeightDiffs[1] = world -> getTerrainHeight(vec3(pos.x - 1.0, 0.0, pos.z + 1.0));
		terrainHeightDiffs[2] = world -> getTerrainHeight(vec3(pos.x + 1.0, 0.0, pos.z - 1.0));
		terrainHeightDiffs[3] = world -> getTerrainHeight(vec3(pos.x + 1.0, 0.0, pos.z + 1.0));

		// get the lowest one
		lowestHeight = glm::max(glm::max(terrainHeightDiffs[0], terrainHeightDiffs[1]), glm::max(terrainHeightDiffs[2], terrainHeightDiffs[3]));

		// if this is sufficiently low, then bring the drone up a bit
		if(pos.y < (lowestHeight + MIN_ROTOR_HEIGHT))
		{
			pos = vec3(pos.x, pos.y + ((lowestHeight + MIN_ROTOR_HEIGHT) - pos.y), pos.z);
			setPos(pos);
		}
	}
}

void Drone::controlWarningSound()
{
	const float DIST_TO_WARN = 18.0;
	const float DIST_TO_RESET = 20.0;

	const float REF_HEAR_DIST = 100.0;
	const float MAX_HEAR_DIST = 100.0;

	if(distanceToPlayer < DIST_TO_WARN)
	{
		if(!playedWarning)
		{
			playedWarning = true;
			soundManager -> playSound(warningBuffer, getPos(), REF_HEAR_DIST, MAX_HEAR_DIST);
		}
	}
	else if(distanceToPlayer > DIST_TO_RESET)
	{
		playedWarning = false;
	}
}

void Drone::controlHoverSound()
{
	const float LOOP_AUDIO_DIST = 300.0;			// distance beyond which we don't loop the hover sound

	vec3 pos;

	// position the hover source where the drone is
	if(distanceToPlayer < LOOP_AUDIO_DIST)
	{
		// only update the hover buzz source if we're alive
		if(alive)
		{
			// get the drone's global position so we can position the sound
			pos = getPos();

			// no hover source active, so start one
			if(hoverSource == 0)
			{
				hoverSource = soundManager -> loopSound(hoverBuffer, pos, HOVER_REF_HEAR_DIST, HOVER_MAX_HEAR_DIST);
			}

			// position the sound where the drone is
			alSource3f(hoverSource, AL_POSITION, pos.x, pos.y, pos.z);
			alSource3f(hoverSource, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
		}
    }
    else
    {
		// we're too far away, so release the sound if we have one
		if(hoverSource != 0)
		{
			soundManager -> stop(hoverSource);
			hoverSource = 0;
		}
    }
}

void Drone::controlDeath()
{
	const float EXPLODE_REF_DIST = 25.0;
	const float EXPLODE_MAX_DIST = FLT_MAX;

	// if we've just detected the destruction of the drone
	if(health <= 0.0 && alive)
	{
		// record it, and...
		alive = false;

		// ...blow it up!
		explode();
		// Extra five points for killing
		hudScore += 5;


		// play the explosion sound
		soundManager -> playSound(explodeBuffer, getPos(), EXPLODE_REF_DIST, EXPLODE_MAX_DIST);

		// also release our hold on the hover sound, if we have one
		if(hoverSource > 0)
		{
			soundManager -> stop(hoverSource);
		}

		// make sure the world knows to remove us from the collider list
		flagAsGarbage();
		world -> addGarbageItem();
	}
}

bool Drone::getAlive()
{
	return alive;
}

void Drone::handleRayCollision(vec3 dir, vec3 pos)
{
	const float RAY_COLLISION_DAMAGE = 11.0;

	impactMotion += dir * 15.0f;
	health -= RAY_COLLISION_DAMAGE;
	hudScore += 5;
}

void Drone::explode()
{
	const int NUM_FIREBALLS = 5;
	const int NUM_SPARKS = 100;

	vec3 pos = getPos();
	int i;

	// cool flames that leave trails
	for(i = 0; i < NUM_FIREBALLS; i ++)
	{
		world -> addParticle(explodeEmitter, pos, 1.0);
	}

	// small burst of sparks
	for(i = 0; i < NUM_SPARKS; i ++)
	{
		world -> addParticle(spark, pos, 1.0);
	}
}
