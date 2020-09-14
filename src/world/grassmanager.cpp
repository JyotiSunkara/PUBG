#include "world/grassmanager.h"
#include "world/world.h"

#include "objects/player.h"

#include "util/shader.h"
#include "util/math.h"
#include "util/profiling.h"

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/noise.hpp"
using namespace glm;

#include <iostream>
using namespace std;

GrassManager::GrassManager(World *world, Player *player, int maxBlades, float grassAreaRadius)
{
	this -> world = world;
	this -> player = player;
	this -> maxBlades = maxBlades;
	this -> grassAreaRadius = grassAreaRadius;

	modelMats = new vec4[maxBlades * 4];
	modelMatUpdateChunk = &modelMats[maxBlades * 3];
	shadowValues = new float[maxBlades];
	shadowValuesUpdateChunk = shadowValues;
	updateChunkIndex = 0;

	waveValue = 0.0;
	shutdown = false;

	setupVBOs();
	loadShader();
	placeGrass();
}

GrassManager::~GrassManager()
{
	shutdown = true;
	pthread_join(updateThread, NULL);

	delete[] shadowValues;
	delete[] modelMats;

	glDeleteBuffers(5, vbos);
	glDeleteVertexArrays(1, &vao);
	delete shader;
}

void GrassManager::beginWrapThread()
{
	pthread_create(&updateThread, NULL, invokeWrapLoop, this);
}

void *GrassManager::invokeWrapLoop(void *arg)
{
	GrassManager *grass = (GrassManager*)arg;
	grass -> updateLoop();
	return NULL;
}

void GrassManager::setupVBOs()
{
	const int NUM_VERTICES = 3;
	const vec3 VERTICES[] = {vec3(-0.01, 0.0, 0.0),
							 vec3(0.01, 0.0, 0.0),
							 vec3(0.0, 1.0, 0.09)};
	const vec4 COLORS[] = {vec4(0.25, 0.37, 0.12, 1.33) * 1.125f,
						   vec4(0.15, 0.35, 0.09, 1.33) * 1.125f,
						   vec4(0.45, 0.65, 0.22, 1.33) * 1.375f};

	// set up our OpenGL render state and get ready to put some data in the GPU
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(5, vbos);

	// vertex positions are shared across all instances
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * NUM_VERTICES, VERTICES, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// same with colours---those never change
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * NUM_VERTICES, COLORS, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// the brightness values vary, though, but we only write them to the GPU once
	glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * maxBlades, NULL, GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glVertexAttribDivisor(2, 1);

	// the shadow intensities will vary through the life of the grass
	glBindBuffer(GL_ARRAY_BUFFER, vbos[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * maxBlades, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glVertexAttribDivisor(3, 1);


    glBindBuffer(GL_ARRAY_BUFFER, vbos[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * maxBlades * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(vec4) * maxBlades * 0));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(vec4) * maxBlades * 1));
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(vec4) * maxBlades * 2));
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(vec4) * maxBlades * 3));
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);
}

void GrassManager::loadShader()
{
	shader = new Shader("../shaders/grass.vert", "../shaders/grass.frag");
	shader -> bindAttrib("a_Vertex", 0);
	shader -> bindAttrib("a_Color", 1);
	shader -> bindAttrib("a_Brightness", 2);
	shader -> bindAttrib("a_ShadowValue", 3);
	shader -> bindAttrib("a_InstanceMatrix", 4);
	shader -> link();
	shader -> bind();
	shader -> uniform1f("u_GrassAreaRadius", grassAreaRadius);
	shader -> uniform1f("u_WaveStrength", 0.025);
	shader -> unbind();
}

void GrassManager::placeGrass()
{
	const float GLM_RAND_FIX = 0.6;		// see below for why we need this; it's a GLM precision bug of some kind...

	const float MIN_HEIGHT = 0.4;
	const float MAX_HEIGHT = 0.8;

	const float MIN_BRIGHTNESS = 0.5;
	const float MAX_BRIGHTNESS = 0.9;
	const float BRIGHTNESS_RANGE = MAX_BRIGHTNESS - MIN_BRIGHTNESS;

	float *brightnessValues = new float[maxBlades];
	float *brightnessValuePtr = brightnessValues;

	float *shadowValuePtr = shadowValues;

	float per;

	vec4 *sidePtr = &modelMats[maxBlades * 0];
	vec4 *upPtr = &modelMats[maxBlades * 1];
	vec4 *forwardPtr = &modelMats[maxBlades * 2];
	vec4 *posPtr = &modelMats[maxBlades * 3];

	vec3 randomPos;
	mat4 temp;
	int i;

	// build a list of grass objects that we need to place
	for(i = 0; i < maxBlades; i ++)
	{
		// pick a location on the terrain within the specified radius (rectangular area, actually...);
		// for some annoying reason, GLM's random is not very precise, and we can't seem to get values
		// above around 79.5 here, causing problems with pathways forming in the grass
		randomPos = vec3(linearRand(-grassAreaRadius, grassAreaRadius + GLM_RAND_FIX),
						 0.0,
						 linearRand(-grassAreaRadius, grassAreaRadius + GLM_RAND_FIX));

		// assign a random brightness value to the blade for extra realism; perlin noise is particularly nice since it will
		// darken/lighten the grass in patches for a much more convincing effect
		per = (1.0 + fakePerlinNoise(randomPos.x, randomPos.z)) / 2.0;
		*brightnessValuePtr++ = MIN_BRIGHTNESS + (BRIGHTNESS_RANGE * per);

		// now add offset to player position and make sure we're at the terrain height
		randomPos = randomPos + player -> getPos();
		randomPos.y = world -> getTerrainHeight(randomPos);

		// configure the position, size, and orientation of the blade
		temp = mat4(1.0);
		temp = translate(temp, randomPos);
		temp = scale(temp, vec3(1.0, linearRand(MIN_HEIGHT, MAX_HEIGHT), 1.0));
		temp = rotate(temp, (float)rand(), vec3(0.0, 1.0, 0.0));

		// now add the matrix to our modelview array
		*sidePtr++ = temp[0];
		*upPtr++ = temp[1];
		*forwardPtr++ = temp[2];
		*posPtr = temp[3];

		// assign a shadow value for the grass based on its position within the terrain's shadow map
		*shadowValuePtr++ = (float)world -> getShadowValue(vec3(*posPtr++)) / 255.0;
	}

	// pass brightness values into the GPU
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * maxBlades, brightnessValues);

	// pass shadow values into the GPU
	glBindBuffer(GL_ARRAY_BUFFER, vbos[3]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * maxBlades, shadowValues);

	// pass modelview data into the GPU
	glBindBuffer(GL_ARRAY_BUFFER, vbos[4]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * maxBlades * 4, modelMats);

	// free up our memory
	delete[] brightnessValues;
}

void GrassManager::updateLoop()
{
	vec4 *positionsPtr;			// used to update grass positions
	float *shadowsPtr;			// used to update grass shadow intensity

	vec4 playerPos;				// position of player
	vec4 pos;					// position of grass blade
	vec4 diff;					// difference between player and grass blade position
	bool shifted = false;		// has the current blade of grass moved?
	int i;

	// we're in a different thread; this can run in an infinite loop until the game quits
	while(!shutdown)
	{
		// no need to do any of this if the player isn't moving
		if(player -> getIsMoving())
		{
			// save our starting point for updating grass positions
			positionsPtr = &modelMats[maxBlades * 3];
			shadowsPtr = shadowValues;

			// now wrap the grass around the player
			playerPos = vec4(player -> getPos(), 1.0);
			playerPos.y = 0.0;					// important to zero our positions when we do this!
			for(i = 0; i < maxBlades; i ++)
			{
				// get our position from the player
				pos = *positionsPtr;
				pos.y = 0.0;					// important to zero our positions because height differences
				diff = playerPos - pos;			// shouldn't matter when we do this
				shifted = false;

				// figure out which direction we need to jump in to stay in the player's grass area
				if(diff.x > (grassAreaRadius + 1)){ pos.x += grassAreaRadius * 2.0; shifted = true; }
				else if(diff.x < -(grassAreaRadius + 1)) { pos.x -= grassAreaRadius * 2.0; shifted = true; }
				if(diff.z > (grassAreaRadius + 1)) { pos.z += grassAreaRadius * 2.0; shifted = true; }
				else if(diff.z < -(grassAreaRadius + 1)) { pos.z -= grassAreaRadius * 2.0; shifted = true; }

				// if we need to, re-position this blade of grass (but save us the calculation if we don't need it)
				if(shifted)
				{
					// get the terrain height at that position
					pos.y = world -> getTerrainHeight(pos);

					// insert the value back into the modelview matrix; we should be using mutexes here but I don't want to,
					// for performance reasons, and because the grass isn't really gameplay-critical anyways
					*positionsPtr = pos;

					// assign the shadow value at that position to the blade
					*shadowsPtr = (float)world -> getShadowValue(pos) / 255.0;
				}
				positionsPtr++;
				shadowsPtr++;
			}
		}
	}
}

void GrassManager::controlGrassWaving(float dt)
{
	const float WAVE_SPEED = 1.0 * dt;

	waveValue += WAVE_SPEED;
	shader -> bind();
	shader -> uniform1f("u_WaveTime", waveValue);
}

void GrassManager::update(float dt)
{
	controlGrassWaving(dt);
}

void GrassManager::render(mat4 &projection, mat4 &view, mat4 &model)
{
	const int NUM_BLADES_PER_UPDATE = 2000;		// this should be low enough that there's not too much data to send to the GPU, but
												// high enough that we don't have the blades struggling to catch up with the player
	const int RESET_POINTER_STEPS = maxBlades / NUM_BLADES_PER_UPDATE;

	shader -> bind();
	shader -> uniformMatrix4fv("u_Projection", 1, value_ptr(projection));
	shader -> uniformMatrix4fv("u_View", 1, value_ptr(view));

	// only update a small chunk of the grass items; if we've picked an appropriate value of NUM_BLADES_PER_UPDATE, we shouldn't
	// notice that we're only cycling through a small amount per update step; also, we should probably be using mutexes here
	// but I'm not going to bother, since the grass is pretty non-critical and I don't want to use locking for performance reasons
	glBindVertexArray(vao);

	// update the positions of the grass; the way we've structured our vertex attributes, we only need to send the position vector,
	// as opposed to the entire 4x4 matrix for each blade of grass---this results in a lot of performance savings
	glBindBuffer(GL_ARRAY_BUFFER, vbos[4]);
	glBufferSubData(GL_ARRAY_BUFFER,
					(sizeof(vec4) * maxBlades * 3) + (sizeof(vec4) * updateChunkIndex * NUM_BLADES_PER_UPDATE),
					sizeof(vec4) * NUM_BLADES_PER_UPDATE,
					modelMatUpdateChunk);

	// update the shadow intensities of the grass
	glBindBuffer(GL_ARRAY_BUFFER, vbos[3]);
	glBufferSubData(GL_ARRAY_BUFFER,
					(sizeof(float) * updateChunkIndex * NUM_BLADES_PER_UPDATE),
					sizeof(float) * NUM_BLADES_PER_UPDATE,
					shadowValuesUpdateChunk);

	// prepare to update the next chunk on the next time around
	modelMatUpdateChunk += NUM_BLADES_PER_UPDATE;
	shadowValuesUpdateChunk += NUM_BLADES_PER_UPDATE;
	updateChunkIndex ++;

	// note: another possible optimization to grass handling would be to divide it into patches of grass, and only update patches
	// based on their position relative to the player; this would require a fairly-major restructuring of how the grass manager
	// works and might be tricky to implement from an OpenGL perspective...I'll give this some more thought

	// wrap back around to the first chunk if we need to
	if(updateChunkIndex >= RESET_POINTER_STEPS)
	{
		modelMatUpdateChunk = &modelMats[maxBlades * 3];
		shadowValuesUpdateChunk = shadowValues;
		updateChunkIndex = 0;
	}

	// finally, draw the grass
	glEnable(GL_BLEND);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 3, maxBlades);
	glDisable(GL_BLEND);
}
