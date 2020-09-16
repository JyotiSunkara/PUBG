#include "objects/dronemanager.h"
#include "objects/drone.h"
#include "objects/complexcollider.h"
#include "objects/player.h"
extern bool fogFlag;
#include "objects/hud.h"

#include "util/shader.h"
#include "util/loadtexture.h"
#include "util/math.h"

#include "world/world.h"

#include "audio/soundmanager.h"

#include "glmmodel/glmmodel.h"

#include "GL/glew.h"

#include "glm/glm.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
using namespace glm;

#include <cstdlib>
#include <iostream>
using namespace std;

int hudDrones = 150;
int hudScore;

DroneManager::DroneManager(World *world, int maxDrones) {
	this -> world = world;
	this -> maxDrones = maxDrones;
	numDrones = 0;
	numDronesAlive = 0;

	drones = new Drone[maxDrones];
	modelMats = new mat4[maxDrones];

	initTemporalPartitioning();
	loadModels();
	loadTextures();
	loadShader();
	loadSounds();
}

DroneManager::~DroneManager() {
	glmDelete(droneColliderModel);
	glmDelete(droneBladeModel);
	glmDelete(droneBodyModel);

	glDeleteBuffers(4, bodyVBOs);
	glDeleteVertexArrays(1, &bodyVAO);
	delete bodyShader;

	glDeleteBuffers(4, bladesVBOs);
	glDeleteVertexArrays(1, &bladesVAO);
	delete bladesShader;

	delete[] drones;
	delete[] modelMats;
}

void DroneManager::initTemporalPartitioning() {
	int i;
	cylinderTestTimers = new float[maxDrones];
	for(i = 0; i < maxDrones; i ++)
	{
		cylinderTestTimers[i] = 0.0;
	}
}

void DroneManager::loadModels() {
	// attempt to read the body file; glmReadObj() will just quit if we can't
    droneBodyModel = glmReadOBJ((char*)"../mesh/drone-body.obj");
    if(droneBodyModel)
    {
		glmScale(droneBodyModel, 1.0);

		// build our buffer objects and then fill them with the geometry data we loaded
		glGenVertexArrays(1, &bodyVAO);
		glBindVertexArray(bodyVAO);
		glGenBuffers(4, bodyVBOs);
		glmBuildVBO(droneBodyModel, &numBodyVertices, &bodyVAO, bodyVBOs);

		// set aside some memory for our drone model matrices (i.e., prepare for instanced rendering)
		glBindBuffer(GL_ARRAY_BUFFER, bodyVBOs[3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * maxDrones, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)0);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)sizeof(vec4));
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)(sizeof(vec4) * 2));
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)(sizeof(vec4) * 3));
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
	}

	// attempt to read the blades file; glmReadObj() will just quit if we can;t
	droneBladeModel = glmReadOBJ((char*)"../mesh/drone-blades.obj");
	if(droneBladeModel)
	{
		glmScale(droneBladeModel, 1.0);

		// build out buffer objects and then fill them with the geometry data we loaded
		glGenVertexArrays(1, &bladesVAO);
		glBindVertexArray(bladesVAO);
		glGenBuffers(4, bladesVBOs);
		glmBuildVBO(droneBladeModel, &numBladesVertices, &bladesVAO, bladesVBOs);

		// set aside some memory for our blade model matrices (i.e., prepare for instanced rendering)
		glBindBuffer(GL_ARRAY_BUFFER, bladesVBOs[3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * maxDrones, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)0);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)sizeof(vec4));
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)(sizeof(vec4) * 2));
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)(sizeof(vec4) * 3));
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
	}

	// attempt to read the collision geometry; glmReadObj() will just quit if we can't
	droneColliderModel = glmReadOBJ((char*)"../mesh/drone-collider.obj");
	if(droneColliderModel)
	{
		glmScale(droneColliderModel, 1.0);
	}
}

void DroneManager::loadTextures()
{
    diffuseMap = loadPNG("../png/drone-diffuse-map.png");
    normalMap = loadPNG("../png/drone-normal-map.png");
    specularMap = loadPNG("../png/drone-specular-map.png");
    emissionMap = loadPNG("../png/drone-emission-map.png");
    bladesTexture = loadPNG("../png/drone-blades.png");
}

void DroneManager::loadShader()
{
	// Dronemanager::render takes care of lighting
	bodyShader = new Shader("../shaders/solid-instanced.vert", "../shaders/solid.frag");
	bodyShader -> bindAttrib("a_Vertex", 0);
	bodyShader -> bindAttrib("a_Normal", 1);
	bodyShader -> bindAttrib("a_TexCoord", 2);
	bodyShader -> bindAttrib("a_Model", 3);
	bodyShader -> link();
	bodyShader -> bind();
	bodyShader -> uniform1i("u_DiffuseMap", 0);
	bodyShader -> uniform1i("u_NormalMap", 1);
	bodyShader -> uniform1i("u_SpecularMap", 2);
	bodyShader -> uniform1i("u_EmissionMap", 3);
	bodyShader -> uniformVec3("u_Sun", World::SUN_DIRECTION);
	bodyShader -> uniformVec3("u_MaterialDiffuse", vec3(1.0, 1.0, 1.0));
	bodyShader -> uniformVec3("u_MaterialSpecular", vec3(1.0, 1.0, 1.0));
	bodyShader -> uniform1f("u_SpecularIntensity", 6.0);
	bodyShader -> uniform1f("u_SpecularHardness", 16.0);
	bodyShader -> uniform1f("u_NormalMapStrength", 2.0);
	bodyShader -> uniformVec3("fogColor", vec3(0.65, 0.65, 0.65));
	bodyShader -> uniform1i("fogFlag", fogFlag);
	bodyShader -> unbind();

	// shader for the drone blades
	bladesShader = new Shader("../shaders/solid-instanced.vert", "../shaders/plane.frag");
	bladesShader -> bindAttrib("a_Vertex", 0);
	bladesShader -> bindAttrib("a_Normal", 1);
	bladesShader -> bindAttrib("a_TexCoord", 2);
	bladesShader -> bindAttrib("a_Model", 3);
	bladesShader -> link();
	bladesShader -> bind();
	bladesShader -> uniform1i("u_Texture", 0);
	bladesShader -> uniformVec4("u_Color", vec4(0.0, 0.0, 0.0, 1.0));
	bladesShader -> uniformVec3("fogColor", vec3(0.65, 0.65, 0.65));
	bladesShader -> uniform1i("fogFlag", fogFlag);
	bladesShader -> unbind();
}

void DroneManager::loadSounds()
{
	hoverSound = SoundManager::getInstance() -> loadWAV("../wav/drone-hover.wav");			// loops continually when player is close enough to hear it
	warningSound = SoundManager::getInstance() -> loadWAV("../wav/drone-warning.wav");		// drone makes a warning buzz when player is nearby
	explodeSound = SoundManager::getInstance() -> loadWAV("../wav/drone-explode.wav");		// drone explosion sound when player shoots and kills it
}

Drone *DroneManager::addDrone(vec3 pos) {
	Drone *result = NULL;

	if(numDrones < maxDrones) {
		drones[numDrones] = Drone(world, pos, hoverSound, warningSound, explodeSound);
		result = &drones[numDrones];

		result -> setComplexCollider(new ComplexCollider(droneColliderModel));

		numDrones ++;
	} else {
		cerr << "DroneManager::addDrone() cannot add a drone because the maximum of " << maxDrones << " has been reached" << endl;
		exit(1);
	}

	return result;
}

void DroneManager::update(float dt)
{
	const float ALWAYS_TEST_CYLINDER_DISTANCE = 2.0;

	Drone *curr = drones;
	float *cylinderTimer = cylinderTestTimers;
	float closestDist;

	mat4 *modelMatPtr;
	vec3 pos;
	vec3 newPos;
	int i;

	// update the active drones and update how many of them we track for rendering
	modelMatPtr = modelMats;
	numDronesAlive = 0;
	for(i = 0; i < numDrones; i ++)
	{
		if(curr -> getAlive())
		{
			curr -> update(dt);
			pos = curr -> getPos();

			// handle collision with AABBs (super rare, but overhead is negligible compared to other tests)
			if(world -> getAABBCollision(pos, &newPos))
			{
				curr -> setPos(newPos);
				pos = newPos;
			}

			// cylinder collision is also very rare, so we test for cylinder collisions on a timer based on
			// how close the closest cylinder is when we last did a test
			(*cylinderTimer) -= dt;
			if(*cylinderTimer <= 0.0)
			{
				// handle collision with cylinders
				if(world -> getCylinderCollision(pos, &newPos, &closestDist))
				{
					curr -> setPos(newPos);
					pos = newPos;
					*cylinderTimer = 0.0;
				}
				else
				{
					// set the time to test for cylinder collision based on closest distance to nearest cylinder
					if(closestDist < ALWAYS_TEST_CYLINDER_DISTANCE)
					{
						*cylinderTimer = 0.0;
					}
					else
					{
						// pick a new random test delay that is still guaranteed to not miss a collision; we add
						// a random time to this so the drones will all be slightly offset in time when they do this
						*cylinderTimer = (closestDist / Drone::MOVE_SPEED) - linearRand(1.0, 3.0);
					}
				}
			}

			// save the drone's model matrix for rendering later
			curr -> getModelMat(modelMatPtr);
			modelMatPtr++;
			numDronesAlive ++;
		}
		curr ++;
		cylinderTimer ++;
	}

	// now send the data to the graphics card (the body geometry)
	glBindVertexArray(bodyVAO);
	glBindBuffer(GL_ARRAY_BUFFER, bodyVBOs[3]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(mat4) * numDrones, modelMats);

	// now send the data to the graphics card (the blades geometry)
	glBindVertexArray(bladesVAO);
	glBindBuffer(GL_ARRAY_BUFFER, bladesVBOs[3]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(mat4) * numDrones, modelMats);
	hudDrones = numDronesAlive;
	hudScore = (numDrones - numDronesAlive) * 10;
}

void DroneManager::render(mat4 &projection, mat4 &view) {	
	// compute our normal matrix for lighting
	mat4 modelMatrix(1.0);
	mat3 normal = inverseTranspose(mat3(modelMatrix));		// this does actually do anything, and should be passed in per
															// instance, not per rendering call

	// OpenGL rendering settings common to blades and body
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// drone blades and bodies are rendered separately
	renderBodies(projection, view, normal);
	renderBlades(projection, view, normal);
}

void DroneManager::renderBodies(mat4 &projection, mat4 &view, mat3 &normal)
{
	// send in the cavalry
	bodyShader -> bind();
	bodyShader -> uniformMatrix4fv("u_Projection", 1, value_ptr(projection));
	bodyShader -> uniformMatrix4fv("u_View", 1, value_ptr(view));
	bodyShader -> uniformMatrix3fv("u_Normal", 1, value_ptr(normal));

	// we use diffuse, normal, specular, and emission texture maps when rendering for a really nice effect
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, specularMap);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, emissionMap);

	// finally, render the drone bodies
	glBindVertexArray(bodyVAO);
	glDrawArraysInstanced(GL_TRIANGLES, 0, numBodyVertices, numDronesAlive);
}

void DroneManager::renderBlades(mat4 &projection, mat4 &view, mat3 &normal)
{
	// send in the cavalry
	bladesShader -> bind();
	bladesShader -> uniformMatrix4fv("u_Projection", 1, value_ptr(projection));
	bladesShader -> uniformMatrix4fv("u_View", 1, value_ptr(view));
	//bladesShader -> uniformMatrix3fv("u_Normal", 1, value_ptr(normal));

	// we use diffuse, normal, specular, and emission texture maps when rendering for a really nice effect
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bladesTexture);

	// we want these transparent and not writing to the depth buffer
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	// finally, render the drone bodies
	glBindVertexArray(bladesVAO);
	glDrawArraysInstanced(GL_TRIANGLES, 0, numBladesVertices, numDronesAlive);

	// restore opacity
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

bool DroneManager::isDroneCloseTo(vec3 pos, float distance)
{
	double DIST_SQUARED = distance * distance;

	Drone *curr = drones;
	vec3 dronePos;
	vec3 diff;
	double distSquared;
	bool result = false;
	int i = 0;

	while(!result && i < numDrones)
	{
		if(curr -> getAlive())
		{
			dronePos = curr -> getPos();
			distSquared = (dronePos.x - pos.x) * (dronePos.x - pos.x) +
						  (dronePos.y - pos.y) * (dronePos.y - pos.y) +
						  (dronePos.z - pos.z) * (dronePos.z - pos.z);
			result = distSquared < DIST_SQUARED;
		}
		curr ++;
		i ++;
	}

	return result;
}
