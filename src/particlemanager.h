#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include <vector>

class ParticleConfig;
class Particle;
class Shader;

class ParticleManager
{
private:
	int maxParticles;										// number of active particles we allow at one time
	int freeParticleIndex;									// decent guess as to where the next unused particle is located in our pool

	int numInActivePool;									// how many particles are active in our active particle list
	int numToRecycle;										// used to prevent unnecessary iteration when removing dead particles from active pool

	Shader *shader;											// shader program used when rendering particles

	GLuint vao;												// vertex array that encapsulates vertex buffer states
	GLuint vbo;												// vertex buffer object

	Particle *particlePool;									// we allocate a large pool of particles and draw from it/replace as necessary
	Particle **activeParticles;								// only contains the particles that are currently active
	std::vector<int> textureIndices;						// where each texture index begins inside the sorted (by texture) list of active particles
	std::vector<ParticleConfig*> buffered;					// particles waiting to be inserted into the active particle list

	float *vertexAttribData;								// where we store the vertex attributes for each particle texture group

	// load resources
	void setupVBOs();
	void loadShader();

	// some misc. methods for efficiently handling our particles
	void insertIntoActivePool(Particle *particle);
	Particle *getFreeParticle();

public:
	ParticleManager(int maxParticles);
	~ParticleManager();

	// insert a particle at the given position, with a life factor varying from 0 to 1 (useful for particles emitting children particles)
	void add(ParticleConfig *particle, glm::vec3 pos, float lifeFactor);

	void insertBuffered();				// required to update the system with particles inserted via add()
	void update(double dt);				// updates all active particles in the system
	void recycle();						// removes dead particles from active service

	// batch render our particles by texture object
	void render(glm::mat4 &projection, glm::mat4 &view, glm::vec3 &cameraRight, glm::vec3 &cameraUp);

	// how many particles are currently active?
	int getNumActiveParticles();
};
