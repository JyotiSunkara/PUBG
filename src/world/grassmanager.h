#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include "pthread.h"

class World;
class Player;
class Shader;

class GrassManager
{
private:
	World *world;								// used to get terrain height
	Player *player;								// used to wrap grass around player when they're moving

	int maxBlades;								// number of blades of grass we want to have
	float grassAreaRadius;						// how large the area of grass is that wraps around the player

	glm::vec4 *modelMats;						// instance model matrices we pass to the GPU periodically
	glm::vec4 *modelMatUpdateChunk;				// pointer to beginning of current chunk of model mats we want to pass to GPU
	float *shadowValues;						// instance shadow darkness values we pass to the GPU periodically
	float *shadowValuesUpdateChunk;				// pointer to beginning of current chunk of shadow values we want to pass to GPU
	int updateChunkIndex;						// zero-based index of the current chunk we want to update (used to reset above pointers)

	float waveValue;							// we only need a single value to control the different waving of all of the blades

	Shader *shader;								// shadow program we use when rendering the grass

	// wrapping all the grass positions around the player is costly, so we do it in another thread
	pthread_t updateThread;
	bool shutdown;

	GLuint vao;									// GL rendering state
	GLuint vbos[5];								// vertex positions, colours, brightness values, shadow values, model matrices

	static void *invokeWrapLoop(void *arg);		// arg is expected to be the GrassManager, and starts the loop that winds the grass

	void updateLoop();							// loop for wrapping grass that runs in another thread

	// initialization stuff
	void setupVBOs();
	void loadShader();
	void placeGrass();

	// handles a simple wave animation of the grass
	void controlGrassWaving(float dt);

public:
	GrassManager(World *world, Player *player, int maxBlades, float grassAreaRadius);
	~GrassManager();

	// called once after initialized has taken place
	void beginWrapThread();

	// handles waving
	void update(float dt);

	// updates a chunk of grass and renders all of the grass
	void render(glm::mat4 &projection, glm::mat4 &view, glm::mat4 &model);
};
