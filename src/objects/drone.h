#pragma once

#include "objects/object.h"

#include "audio/soundmanager.h"

#include "glm/glm.hpp"

class World;

class Drone : public Object
{
private:
	static const float DEFAULT_HEALTH;					// hit points a drone starts out with

	static const float HOVER_REF_HEAR_DIST;				// OpenAL param: "reference distance" used in distance model
	static const float HOVER_MAX_HEAR_DIST;				// OpenAL param: "maximum distance" used in distance model

	World *world;										// handle to world object for player access
	SoundManager *soundManager;							// convenient handle to singleton instance of audio handler

	ALuint hoverBuffer;									// OpenAL buffer for the hover sound
	ALuint hoverSource;									// specific OpenAL hover source used by this drone
	ALuint warningBuffer;								// OpenAL buffer for the warning buzz
	ALuint explodeBuffer;								// OpenAL buffer for the explosion sound

	glm::vec3 velocity;									// velocity of the drone in m/s
	glm::vec3 impactMotion;								// separate velocity used to control the impact effect of getting shot by the player

	float distanceToPlayer;								// computed every frame (we should use temporal partitioning here since we don't need this constantly)
	bool playedWarning;									// have we played the warning buzz yet? used to ensure we don't continually play it

	float health;										// current hit points of drone
	bool alive;											// is the drone at >0 hit points?

	void moveTowardsPlayer(float dt);					// simple motion towards player
	void controlImpactMotion(float dt);					// simple impact effect after getting shot
	void controlGroundIntersection();					// prevents intersection with terrain (temporal partitioning would be useful here, too)
    void controlWarningSound();							// plays warning sound when player is dangerously close
    void controlHoverSound();							// loops hover sound when player is close enough to hear it
    void controlDeath();								// controls death detection

    void explode();										// boom

public:
	static const float MOVE_SPEED;						// speed of drone is constant

	Drone();
	Drone(World *world, glm::vec3 pos, ALuint hoverBuffer, ALuint warningBuffer, ALuint explodeBuffer);
	~Drone();

	// updating is done individually, but drones are rendered in batches via DroneManager::render();
	void update(float dt);

	// true iff health > 0
	bool getAlive();

	// called if a collision with a bullet is detected
	void handleRayCollision(glm::vec3 dir, glm::vec3 point);
};
