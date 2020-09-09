#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include <map>
#include <vector>

class ParticleConfig;
class ParticleManager;

class Particle
{
public:
	Particle();
	~Particle();

	void configure(ParticleConfig *config, glm::vec3 pos, float lifeFactor, ParticleManager *particleManager);

	void update(float dt);
	// no render() is supplied; ParticleManager renders everything in batches according to texture

	// retrieve current properties of particle; needed by ParticleManager to send to GPU
	void getPos(float *positionArray);
	void getColor(float *colorArray);
	float getAngle();
	float getSize();
	bool getDead();
	GLuint getTexture();

private:
	// -- properties received from ParticleConfig -- //

	glm::vec3 motion;									// randomly decided motion of particle

    float spin;											// value of spin, in rad/sec

    float startSize;									// square size of particle at beginning of life
    float endSize;										// square size of particle at end of life

    glm::vec4 startColor;								// colour and alpha of particle at beginning of life
    glm::vec4 endColor;									// colour and alpha of particle at end of life

    float maxLife;										// life value this particle started off with, in seconds

    float gravityRate;									// downwards acceleration due to gravity

    bool additiveBlending;								// type of blending this particle uses: UNUSED

    GLuint texture;										// texture used when rendering this particle

    float childEmissionInterval;						// how often to emit children particles
    std::vector<ParticleConfig*> children;				// the children particles to emit

    // -- current real-time properties -- //

    ParticleManager *particleManager;					// owner of this particle, used to add children

    glm::vec3 pos;										// current world position of particle
    float angle;										// current angle of particle (roll, after billboarding)

    float size;											// current square size of particle

    glm::vec4 color;									// current colour and alpha of particle

    float life;											// current life remaining, in seconds

    float gravity;										// accumulated acceleration downwards due to gravity

    float childEmissionTimer;							// controls child particle emission

    // -- functions -- //

    void emitChildren(glm::vec3 pos, float lifeFactor);
};
