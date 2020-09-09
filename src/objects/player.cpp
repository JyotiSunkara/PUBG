#include "objects/player.h"

#include "world/world.h"

#include "particles/particleconfig.h"
#include "particles/particlelist.h"

#include "util/loadtexture.h"
#include "util/shader.h"

#include "audio/soundmanager.h"

#include "glmmodel/glmmodel.h"

#include "GL/glew.h"

#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
using namespace glm;

#include <iostream>
#include <string>
using namespace std;

const float Player::PLAYER_HEIGHT = 1.7;				// how high the camera is off the ground

const int Player::MAX_ROUNDS_PER_CLIP = 12;				// how many bullets can we fire before reloading?

const float Player::GUN_RECOIL_ANIM_TIME = 0.25;		// length of the gun recoil animation in seconds
const float Player::DEATH_IMPACT_ANIM_TIME = 0.5;		// length of the player impact animation in seconds

const float Player::MAX_LOOK_PITCH = M_PI_2 - 0.2;		// min and max pitch angle for the camera in radians

Player::Player(GLFWwindow *window, World* world, vec3 pos)
{
	this -> window = window;
	this -> world = world;
	this -> pos = pos;

	// movement and jumping defaults---initially, the player is on the ground
    jumpTimer = 0.0;
	gravity = 0.0;
	touchingGround = false;
	isMoving = false;

	// default viewing parameters
	targetLookAngleX = 0.0;
	targetLookAngleY = 0.0;
	lookAngleX = 0.0;
	lookAngleY = 0.0;

	// gun walking animation starts off at zero
	gunWalkBob = 0.0;
	gunWalkBobAmount = 0.0;

	// gun is currently not doing anything
	triggerPressed = false;
	gunRecoilFinished = false;

	// reset gun recoil animation
	gunRecoilTimer = GUN_RECOIL_ANIM_TIME;
	gunRecoilAmount = 0.0;

	// reset death impact animation
	deathImpactTimer = DEATH_IMPACT_ANIM_TIME;
	deathImpactAmount = 0.0;

	// we start with a full clip
	numShotsInClip = MAX_ROUNDS_PER_CLIP;

	// gun is fully loaded
	gunReloadState = STATE_LOADED;
	gunReloadTimer = 0.0;
	gunReloadOffsetAmount = 0.0;

	// player isn't dead...yet
	alive = true;

	// load our assets...this is mainly just the gun
    loadGun();
    loadTextures();
    loadShader();
    loadSounds();

    // assign some reasonable defaults to the player direction
	computeWalkingVectors();
    glfwGetCursorPos(window, &oldMouseX, &oldMouseY);
}

Player::~Player()
{
	glDeleteBuffers(3, vbos);
	glDeleteVertexArrays(1, &vao);
	delete shader;
}

void Player::loadGun()
{
	GLMmodel *geometry;

    // attempt to read the file; glmReadObj() will just quit if we can't
    geometry = glmReadOBJ((char*)"../mesh/gun.obj");
    if(geometry)
    {
		glmScale(geometry, 1.0);

		// build our buffer objects and then fill them with the geometry data we loaded
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(3, vbos);
		glmBuildVBO(geometry, &numGunVertices, &vao, vbos);
	}
}

void Player::loadTextures()
{
	gunDiffuseMap = loadPNG("../png/gun-diffuse-map.png");
	gunNormalMap = loadPNG("../png/gun-normal-map.png");
	gunSpecularMap = loadPNG("../png/gun-specular-map.png");
	gunEmissionMap = loadPNG("../png/gun-emission-map.png");
}

void Player::loadShader()
{
	shader = new Shader("../shaders/solid.vert", "../shaders/solid.frag");
	shader -> bindAttrib("a_Vertex", 0);
	shader -> bindAttrib("a_Normal", 1);
	shader -> bindAttrib("a_TexCoord", 2);
	shader -> link();
	shader -> bind();
	shader -> uniform1i("u_DiffuseMap", 0);
	shader -> uniform1i("u_NormalMap", 1);
	shader -> uniform1i("u_SpecularMap", 2);
	shader -> uniform1i("u_EmissionMap", 3);
	shader -> uniformVec3("u_Sun", World::SUN_DIRECTION);
	shader -> uniformVec3("u_MaterialDiffuse", vec3(1.0, 0.95, 0.85));
	shader -> uniformVec3("u_MaterialSpecular", vec3(1.0, 0.95, 0.85));
	shader -> uniform1f("u_SpecularIntensity", 8.0);
	shader -> uniform1f("u_SpecularHardness", 2.0);
	shader -> uniform1f("u_NormalMapStrength", 2.5);
	shader -> unbind();
}

void Player::loadSounds()
{
    soundManager = SoundManager::getInstance();
	gunFireSound = soundManager -> loadWAV("../wav/gun-fire.wav");
	gunReloadSound = soundManager -> loadWAV("../wav/gun-reload.wav");
	deathSound = soundManager -> loadWAV("../wav/squishy-death.wav");
}

void Player::update(float dt)
{
	controlDeathImpact(dt);

	// player can only do things if they're alive
	if(isAlive())
	{
		controlMouseInput(dt);
		computeWalkingVectors();
		controlMovingAndFiring(dt);
		controlGunBobbing(dt);
		controlGunRecoil(dt);
		controlGunReloading(dt);
	}

	controlLooking(dt);
	controlListener();
}

void Player::render(mat4 &projection, mat4 &view)
{
	const vec3 GUN_SIZE(-0.225, 0.225, 0.225);
	const float GUN_RECOIL_ROTATE_STRENGTH = -4.0;
	const float GUN_RELOAD_ROTATE_AMOUNT = M_PI_2;

	mat4 gunMat;						// model matrix for gun when rendering
	mat4 viewLocalMat;					// view matrix with positional information removed (see comments below for why we do this)
	mat4 normalMat;						// inverse transpose of model matrix---used so that the gun lighting is computed correctly

	// orient the gun in the same direction as the camera (with the recoil and reloading offsets applied)
	gunMat[0] = vec4(cameraSide, 0.0);
	gunMat[1] = vec4(cameraUp, 0.0);
	gunMat[2] = vec4(cameraForward, 0.0);
	gunMat[3] = vec4(gunPos, 1.0);
	gunMat = scale(gunMat, GUN_SIZE);
	gunMat = rotate(gunMat, (gunRecoilAmount * GUN_RECOIL_ROTATE_STRENGTH) + (gunReloadOffsetAmount * GUN_RELOAD_ROTATE_AMOUNT), vec3(1.0, 0.0, 0.0));

	// the gun is pretty small relative to the world, so it's probably a good idea to render it relative to the player
	// rather than relative to the world; this is really only necessary in really large environments where precision
	// becomes a problem for objects close up
	viewLocalMat = view;
	viewLocalMat[3] = vec4(0.0, 0.0, 0.0, 1.0);

	// compute our normal matrix for lighting
	normalMat = inverseTranspose(mat3(gunMat));

	// send in the cavalry
	shader -> bind();
	shader -> uniformMatrix4fv("u_Projection", 1, value_ptr(projection));
	shader -> uniformMatrix4fv("u_View", 1, value_ptr(viewLocalMat));
	shader -> uniformMatrix4fv("u_Model", 1, value_ptr(gunMat));
	shader -> uniformMatrix3fv("u_Normal", 1, value_ptr(normalMat));

	// we use diffuse, normal, specular, and emission texture maps when rendering for a really nice effect
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gunDiffuseMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gunNormalMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gunSpecularMap);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gunEmissionMap);

	// we don't want this gun transparent
	glDisable(GL_BLEND);

	// finally, render it
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, numGunVertices);
}

void Player::controlDeathImpact(float dt)
{
	float impactFactor;

	deathImpactTimer += dt;

	// keep the timer from going over bounds
	if(deathImpactTimer >= DEATH_IMPACT_ANIM_TIME)
	{
		deathImpactTimer = DEATH_IMPACT_ANIM_TIME;
	}

	// we use an inverted, long-tailed quadratic equation to model the recoil animation of the gun
	impactFactor = deathImpactTimer / DEATH_IMPACT_ANIM_TIME;
	deathImpactAmount = -((impactFactor * impactFactor) - impactFactor) * pow(abs(impactFactor - 1.0), 10);
}

void Player::controlMouseInput(float dt)
{
	double mouseX, mouseY;

	// always update our mouse position
	glfwGetCursorPos(window, &mouseX, &mouseY);

	// compute a smooth look direction based on the mouse motion
	targetLookAngleX -= (mouseY - oldMouseY) * 0.002;
	targetLookAngleY -= (mouseX - oldMouseX) * 0.002;
	if(targetLookAngleX > MAX_LOOK_PITCH) targetLookAngleX = MAX_LOOK_PITCH;
	if(targetLookAngleX < -MAX_LOOK_PITCH) targetLookAngleX = -MAX_LOOK_PITCH;

	// track our old mouse position for mouse movement calculations next frame
	oldMouseX = mouseX;
	oldMouseY = mouseY;
}

void Player::controlMovingAndFiring(float dt)
{
	const float JUMP_ACCEL_TIME = 0.10;				// player jump acceleration control
	const float JUMP_STRENGTH = 60.0;				// maximum jump acceleration experienced by player
	const float GRAVITY_STRENGTH = 9.81;			// lighter than regular gravity, for a smoother jump
	const float GUN_INACCURACY = 0.0025;			// how much to vary the direction of the bullet when firing

	const float MOVE_SPEED = 6.7;					// fast sprint is 15mph, or 6.7 m/s

	vec3 targetVelocity;							// how fast we want to go
	vec3 bulletDir;									// the direction we fire bullets in

	// inform everyone else that we're not doing anything yet
	isMoving = false;

	// W or the right mouse button cause us to move forwards
	if(glfwGetKey(window, 'W') == GLFW_PRESS || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT))
	{
		targetVelocity += vec3(0.0, 0.0, MOVE_SPEED);
		isMoving = true;
	}

	// A will move us to the left
	if(glfwGetKey(window, 'A') == GLFW_PRESS)
	{
		targetVelocity += vec3(-MOVE_SPEED, 0.0, 0.0);
		isMoving = true;
	}

	// D will move us to the right
	if(glfwGetKey(window, 'D') == GLFW_PRESS)
	{
		targetVelocity += vec3(MOVE_SPEED, 0.0, 0.0);
		isMoving = true;
	}

	// S will move us backwards
	if(glfwGetKey(window, 'S') == GLFW_PRESS)
	{
		targetVelocity += vec3(0.0, 0.0, -MOVE_SPEED);
		isMoving = true;
	}

	// space bar will start the player's jump acceleration
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && touchingGround)
	{
		jumpTimer = JUMP_ACCEL_TIME;
	}

	// left mouse button will fire the gun
	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && gunReloadState == STATE_LOADED)
	{
		// we can only fire again if we've already released the trigger and the recoil animation is finished
		if(!triggerPressed && gunRecoilFinished)
		{
			// if we have shots left in our clip, we can fire
			if(numShotsInClip > 0)
			{
				gunRecoilTimer = 0.0;
				triggerPressed = true;
				numShotsInClip --;

				// play the gun sound
				soundManager -> playSound(gunFireSound);

				// compute the bullet direction and fire
				bulletDir = normalize(cameraForward + linearRand(vec3(-GUN_INACCURACY), vec3(GUN_INACCURACY)));
				world -> fireBullet(pos, bulletDir);
				world -> addParticle(muzzleFlash, gunPos + pos + (cameraForward * 0.15f) + (cameraUp * 0.065f));
			}
			else
			{
				// otherwise, we reload automatically
				gunReloadState = STATE_START_MOVING_DOWN;
			}
		}
	}
	else
	{
		triggerPressed = false;
	}

	// V or R will reload our gun, but it will also happen automatically if needed
	if(((glfwGetKey(window, 'V') == GLFW_PRESS || glfwGetKey(window, 'R') == GLFW_PRESS) && gunReloadState == STATE_LOADED && numShotsInClip < MAX_ROUNDS_PER_CLIP) ||
	   (numShotsInClip == 0 && gunRecoilFinished == true && gunReloadState == STATE_LOADED))
	{
		gunReloadState = STATE_START_MOVING_DOWN;
	}

	// if we're in the middle of a jump, then keep accelerating upwards
	if(jumpTimer > 0.0)
	{
		gravity += (JUMP_STRENGTH * (jumpTimer / JUMP_ACCEL_TIME)) * dt;
	}
	jumpTimer -= dt;

	// add gravity
	gravity -= GRAVITY_STRENGTH * dt;
	pos.y += gravity * dt;

	// move forward, too
	pos = pos + (forward * targetVelocity.z * dt) + (side * targetVelocity.x * dt);

	// detect collision with ground
	if(pos.y < world -> getTerrainHeight(pos) + PLAYER_HEIGHT)
	{
		// restore normal ground-level height and set gravity speed to zero
		pos.y = world -> getTerrainHeight(pos) + PLAYER_HEIGHT;
		gravity = 0.0;

		// we can jump
		touchingGround = true;
	}
	else
	{
		touchingGround = false;
	}
}

void Player::controlGunBobbing(float dt)
{
	const float GUN_BOB_SPEED = 9.0 * dt;			// we use a sinusoidal curve to handle the gun bobbing effect

	if(isMoving)
	{
		gunWalkBob += GUN_BOB_SPEED;
		gunWalkBobAmount = sin(gunWalkBob);
	}
}

void Player::controlGunRecoil(float dt)
{
	const float ALLOW_TRIGGER_PRESS_TIME = GUN_RECOIL_ANIM_TIME * 0.75;

	float recoilFactor;

	gunRecoilTimer += dt;
	gunRecoilFinished = false;

	// keep the timer from going over bounds
	if(gunRecoilTimer >= GUN_RECOIL_ANIM_TIME)
	{
		gunRecoilTimer = GUN_RECOIL_ANIM_TIME;
	}

	// we don't have to wait for the entire recoil sequence to finish before firing the next shot
	if(gunRecoilTimer >= ALLOW_TRIGGER_PRESS_TIME)
	{
		gunRecoilTimer = GUN_RECOIL_ANIM_TIME;
		gunRecoilFinished = true;
	}

	// we use an inverted, long-tailed quadratic equation to model the recoil animation of the gun
	recoilFactor = gunRecoilTimer / GUN_RECOIL_ANIM_TIME;
	gunRecoilAmount = -((recoilFactor * recoilFactor) - recoilFactor) * pow(abs(recoilFactor - 1.0), 3);
}

void Player::controlGunReloading(float dt)
{
	const float GUN_RELOAD_MOVE_DOWN_TIME = 0.4;				// how long it takes for the gun to lower for reloading
	const float GUN_RELOAD_RELOADING_TIME = 0.4;				// how long it takes for the gun to be reloaded when out of sight
	const float GUN_RELOAD_MOVE_UP_TIME = 0.3;					// how long it takes for the gun to raise into view after reloading

	if(gunReloadState == STATE_START_MOVING_DOWN)
	{
		// the gun begins tilting downwards for a reload after ejecting a clip; play the reload sound
		gunReloadState = STATE_MOVING_DOWN;
		gunReloadTimer = GUN_RELOAD_MOVE_DOWN_TIME;

		soundManager -> playSound(gunReloadSound);
	}
	else if(gunReloadState == STATE_MOVING_DOWN)
	{
		// the gun is down, so start the reload timer to simulate the time it takes to load a clip
		gunReloadOffsetAmount = 1.0 - (gunReloadTimer / GUN_RELOAD_MOVE_DOWN_TIME);
		gunReloadTimer -= dt;
		if(gunReloadTimer <= 0.0)
		{
			gunReloadState = STATE_RELOADING;
			gunReloadTimer = GUN_RELOAD_RELOADING_TIME;
			gunReloadOffsetAmount = 1.0;
		}
	}
	else if(gunReloadState == STATE_RELOADING)
	{
		// we wait for the clip to get loaded
		gunReloadTimer -= dt;
		if(gunReloadTimer <= 0.0)
		{
			gunReloadState = STATE_MOVING_UP;
			gunReloadTimer = GUN_RELOAD_MOVE_UP_TIME;
			numShotsInClip = MAX_ROUNDS_PER_CLIP;
		}
	}
	else if(gunReloadState == STATE_MOVING_UP)
	{
		// clip is loaded, bring the gun back up
		gunReloadOffsetAmount = gunReloadTimer / GUN_RELOAD_MOVE_UP_TIME;
		gunReloadOffsetAmount *= gunReloadOffsetAmount;
		gunReloadTimer -= dt;
		if(gunReloadTimer <= 0.0)
		{
			gunReloadState = STATE_LOADED;
			gunReloadOffsetAmount = 0.0;
		}
	}
}

void Player::controlLooking(float dt)
{
	const float DEATH_IMPACT_STENGTH_FACTOR = 8.0;

	// compute our pitch angle, taking into account any impact animations
	lookAngleX = (lookAngleX + (deathImpactAmount * DEATH_IMPACT_STENGTH_FACTOR)) + (targetLookAngleX - lookAngleX) * 0.8;
	if(lookAngleX > MAX_LOOK_PITCH) lookAngleX = MAX_LOOK_PITCH;
	if(lookAngleX < -MAX_LOOK_PITCH) lookAngleX = -MAX_LOOK_PITCH;

	// y-look direction is simpler
	lookAngleY = lookAngleY + (targetLookAngleY - lookAngleY) * 0.8;
}

void Player::controlListener()
{
    soundManager -> setListenerPos(pos);
	//soundPlayer -> setListenerVelocity(glm::vec3);
	soundManager -> setListenerOrientation(cameraForward, cameraUp);
}

void Player::computeCameraOrientation()
{
	mat4 camera = mat4(1.0);
	camera = rotate(camera, lookAngleY, vec3(0.0, 1.0, 0.0));
	camera = rotate(camera, lookAngleX, vec3(1.0, 0.0, 0.0));

	// we extract the vectors because this is more useful than maintaining a matrix and then extracting them repeatedly later on
	cameraForward = -vec3(camera[2]);
	cameraSide = vec3(camera[0]);
	cameraUp = vec3(camera[1]);
	cameraLook = pos + cameraForward;
}

void Player::computeGunPosition()
{
	// position the gun to the lower right of the player
	const vec3 GUN_BASE_OFFSET(0.2, -0.125, 0.25);

	// controls the strength of the bobbing effect
	const float GUN_BOB_AMOUNT = 0.006;

	// controls the strength of the recoil effect
	const float GUN_RECOIL_MOVE_STRENGTH = 0.6;

	// controls the offset of the reloading animation
	const float GUN_RELOAD_MOVE_AMOUNT = -0.25;

	// compute the base position of the gun, and add the bobbing effect, the recoil, and the reloading offset
	gunPos = (cameraForward * GUN_BASE_OFFSET.z) + (cameraSide * GUN_BASE_OFFSET.x) + (cameraUp * GUN_BASE_OFFSET.y);
	gunPos += (cameraSide * gunWalkBobAmount * GUN_BOB_AMOUNT) - (cameraUp * abs(gunWalkBobAmount) * GUN_BOB_AMOUNT);
	gunPos += cameraUp * gunRecoilAmount * GUN_RECOIL_MOVE_STRENGTH;
	gunPos += cameraUp * gunReloadOffsetAmount * GUN_RELOAD_MOVE_AMOUNT;
}

void Player::computeWalkingVectors()
{
	up = vec3(0.0, 1.0, 0.0);
    forward = rotate(vec3(0.0, 0.0, -1.0), lookAngleY, up);
    side = normalize(cross(forward, up));
}

vec3 Player::getPos()
{
	return pos;
}

void Player::setPos(vec3 pos)
{
	this -> pos = pos;
}

vec3 Player::getCameraLook()
{
	return cameraLook;
}

vec3 Player::getCameraSide()
{
	return cameraSide;
}

vec3 Player::getCameraUp()
{
	return cameraUp;
}

bool Player::getIsMoving()
{
	return isMoving;
}

int Player::getNumShotsInClip()
{
	return numShotsInClip;
}

void Player::die()
{
	// are we currently alive?
	if(alive)
	{
		// record ourselves as dead and start the "oof" impact animation
		alive = false;
		deathImpactTimer = 0.0;

		// play death sound
		soundManager -> playSound(deathSound);
	}
}

bool Player::isAlive()
{
	return alive;
}
