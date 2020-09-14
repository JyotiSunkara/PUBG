#pragma once

#include "AL/al.h"

#include "GL/glew.h"
#include "glm/glm.hpp"

class GLFWwindow;
class World;
class Shader;
class SoundManager;

class Player
{
private:
	typedef enum RELOADSTATE
	{
		STATE_LOADED,								// nothing happens here; gun has at least one round
		STATE_START_MOVING_DOWN,					// set reloadState to this state to begin the reloading sequence
		STATE_MOVING_DOWN,							// gun moves down and rotates out of sight
		STATE_RELOADING,							// pause while gun is out of sight
		STATE_MOVING_UP								// gun is brought back up into sight
	} ReloadState;

	static const float GUN_RECOIL_ANIM_TIME;		// length of the gun recoil animation in seconds
	static const float DEATH_IMPACT_ANIM_TIME;		// length of the player impact animation in seconds

	static const float MAX_LOOK_PITCH;				// min and max pitch angle for the camera in radians

	// handles to important world objects //

	GLFWwindow *window;						// required for mouse input
	World *world;							// required for sun position, and creating world elements (bullets, particles, etc.)
	SoundManager *soundManager;				// required for various sound effecys

	// motion and orientation control //

	glm::vec3 pos;							// global position of player
	glm::vec3 gunPos;						// position of gun relative to player
	glm::vec3 argonPos;						// position of Argon
	glm::vec3 forward;						// forwards vector of player
	glm::vec3 side;							// sideway vector of player (used for strafing)
	glm::vec3 up;							// up vector of player

	float jumpTimer;						// jumping uses a short timed burst of upwards motion; this timer controls it
	float gravity;							// current downward pull experienced by player---zero when touching ground
	bool touchingGround;					// is the player touching the ground currently?
	bool isMoving;							// is the player walking at all, in any direction?

	double oldMouseX;						// used to determine mouse motion
	double oldMouseY;						// used to determine mouse motion

	float targetLookAngleX;					// used to compute smooth camera motion
	float targetLookAngleY;					// used to compute smooth camera motion

	float lookAngleX;						// is computed using targetLookAngleX for smooth camera motion
	float lookAngleY;						// is computed using targetLookAngleY for smooth camera motion

	glm::vec3 cameraForward;				// forward look direction of camera
	glm::vec3 cameraSide;					// sideways vector of camera
	glm::vec3 cameraUp;						// cross of cameraForward and cameraSide
	glm::vec3 cameraLook;					// the global 3D position where the camera is focused (used for glm::lookAt())

	// gun and life properties //

	float gunWalkBob;						// used to time the gun bob effect
	float gunWalkBobAmount;					// abs(sin(gunWalkBobAmount)) is used to compute gun walking bob effect

	bool triggerPressed;					// double-action gun---is the left mouse button currently down?
	bool gunRecoilFinished;					// has the recoil finished enough for us to allow another shot?

	float gunRecoilTimer;					// timer in seconds, used to control the recoil animation
	float gunRecoilAmount;					// factor from 0 to 1 for how far we are into the recoil animation

	int numShotsInClip;						// current number of bullets we have left before a reload is needed

	ReloadState gunReloadState;				// current state of the gun reloading animation; see the ReloadState enum above
	float gunReloadTimer;					// timer in seconds, used to control the reloading animation
	float gunReloadOffsetAmount;			// ranges from 0 to 1: 0 is fully in view, and 1 is lowered out of sight

	bool alive;								// one hit, one kill---is the player alive?
	float deathImpactTimer;					// controls the impact animation of the player's head when hit by a drone
	float deathImpactAmount;				// strength of impact 'oof' when player is hit

	ALuint gunFireSound;					// OpenAL buffer object for the gun firing sound
	ALuint gunReloadSound;					// OpenAL buffer object for the gun reloading sound
	ALuint deathSound;						// OpenAL buffer object for really nasty, squishy-sounding death

	// gun rendering //

	Shader *shader;							// the "solid" shader program we use when rendering the gun

	GLuint vaoGun;							// vertex array object (i.e., GL state) for the gun
	GLuint vbosGun[3];						// vertex buffer objects (i.e., vertex, tex coords, normals) for the gun

	GLuint vaoArgon;						// vertex array object (i.e., GL state) for Argon
	GLuint vbosArgon[3];					// vertex buffer objects (i.e., vertex, tex coords, normals) for Argon
	int numArgonVertices;					// number of vertices, required for GL rendering call

	int numGunVertices;						// number of vertices, required for GL rendering call
	GLuint gunDiffuseMap;					// plain texture used on the gun
	GLuint gunNormalMap;					// normal mapping used for nice per-fragment lighting
	GLuint gunSpecularMap;					// a nice effect is to vary the amount of specular highlighting to give the appearance of dirt
	GLuint gunEmissionMap;					// controls lit components of the gun (LEDs, etc.)

	// resource loading //

	void loadGun();							// load gun geometry
	void loadTextures();					// load gun diffuse, normal, specular, and emission maps
	void loadShader();						// load up and compile the shader for the gun
	void loadSounds();						// load up any sound effects

	// update routines //

	void controlDeathImpact(float dt);		// handles death animation
	void controlMouseInput(float dt);		// turns mouse motion into camera angles
	void computeWalkingVectors();			// computes the vectors used for walking
	void controlLooking(float dt);			// places limits on the player's viewing angles
	void controlMovingAndFiring(float dt);	// controls player motion, firing, and reloading
	void controlGunBobbing(float dt);		// gun bobs as the player walks; this controls that effect
	void controlGunRecoil(float dt);		// gun recoils when the player fires; this controls that effect
	void controlGunReloading(float dt);		// gun dips down and becomes unusable when reloading; this controls that effect
	void controlListener();					// positions and orients the OpenAL 3D listener where the player is

public:
	static const float PLAYER_HEIGHT;		// how high the camera is off the ground

	static const int MAX_ROUNDS_PER_CLIP;	// how many bullets can we fire before reloading?

	Player(GLFWwindow *window, World* world, glm::vec3 pos);
	~Player();

	// update routines //

	void update(float dt);
	void renderGun(glm::mat4 &projection, glm::mat4 &view);
	void renderArgon(glm::mat4 &projection, glm::mat4 &view);

	// these are called externally by the game world, although they probably don't
	// need to be, strictly speaking...
	void computeCameraOrientation();
	void computeGunPosition();
	void computeArgonPosition();

	// some simple getters/setters //

	glm::vec3 getPos();						// used for grass positioning, among other things
	void setPos(glm::vec3 pos);

	glm::vec3 getCameraLook();				// used for a few shaders
	glm::vec3 getCameraSide();
	glm::vec3 getCameraUp();

	bool getIsMoving();						// for grass updating
	int getNumShotsInClip();				// for HUD bullet count rendering

	void die();								// kills the player instantly; caused by drone hit
	bool isAlive();							// pretty self-explanatory; true iff player has not been hit by drone
};
