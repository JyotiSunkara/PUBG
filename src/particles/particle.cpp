#include "particles/particle.h"
#include "particles/particleconfig.h"
#include "particles/particlemanager.h"

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/random.hpp"
using namespace glm;

#include <vector>
using namespace std;

Particle::Particle()
{
	// default properties
	texture = 0;
	life = 0.0;
}

Particle::~Particle() { }

void Particle::configure(ParticleConfig *config, vec3 pos, float lifeFactor, ParticleManager *particleManager)
{
	this -> pos = pos;

	// pick a random motion based on config params
	motion = linearRand(config -> motionLimits[0], config -> motionLimits[1]);

	// randomize spin if requested
	spin = linearRand(config -> spinLimits[0], config -> spinLimits[1]);
	if(config -> randomOrientation)
		angle = linearRand(-M_PI, M_PI);
	else
		angle = 0.0;

	// pick a random start and end size for the particle
    startSize = linearRand(config -> startSizeLimits[0], config -> startSizeLimits[1]) * lifeFactor;
    endSize = linearRand(config -> endSizeLimits[0], config -> endSizeLimits[1]) * lifeFactor;

    // pick a random start and end colour for the particle
    startColor = linearRand(config -> startColorLimits[0], config -> startColorLimits[1]);
    endColor = linearRand(config -> endColorLimits[0], config -> endColorLimits[1]);

    // pick a random life value for the particle
    maxLife = linearRand(config -> lifeLimits[0], config -> lifeLimits[1]);
    life = maxLife;

    // same with gravity strength
    gravity = linearRand(config -> initialGravityLimits[0], config -> initialGravityLimits[1]);
    gravityRate = linearRand(config -> gravityRateLimits[0], config -> gravityRateLimits[1]);

    additiveBlending = config -> additiveBlending;			// unused, I think...
    texture = config -> texture;							// assign desired texture

    // configure child particle emission
    childEmissionInterval = config -> childEmissionInterval;
    children = config -> children;
    childEmissionTimer = 0.0;

    // assign our parent/owner
    this -> particleManager = particleManager;
}

void Particle::update(float dt)
{
	float lifeFactor = 1.0 - (life / maxLife);

	// update position and orientation
	pos += motion + vec3(0.0, gravity, 0.0);
	angle += spin;

	// compute current size and colour value by interpolating linearly between start and end appearance
	size = startSize + (endSize - startSize) * lifeFactor;
	color = startColor + (endColor - startColor) * lifeFactor;

	// accumulate gravity effects
	gravity += gravityRate * dt;

	// control child particle emission...happens at regular intervals
	childEmissionTimer -= dt;
	if(childEmissionTimer <= 0.0)
	{
		childEmissionTimer = childEmissionInterval;
		emitChildren(pos, 1.0 - lifeFactor);
	}

	life -= dt;
}

void Particle::emitChildren(vec3 pos, float lifeFactor)
{
	vector<ParticleConfig*>::iterator i;

	for(i = children.begin(); i != children.end(); ++i)
	{
		particleManager -> add(*i, pos, lifeFactor);
	}
}

void Particle::getPos(float *positionArray)
{
    positionArray[0] = pos.x;
    positionArray[1] = pos.y;
    positionArray[2] = pos.z;
}

void Particle::getColor(float *colorArray)
{
    colorArray[0] = color.x;
    colorArray[1] = color.y;
    colorArray[2] = color.z;
    colorArray[3] = color.w;
}

float Particle::getAngle()
{
	return angle;
}

float Particle::getSize()
{
	return size;
}

bool Particle::getDead()
{
	return life <= 0.0;
}

GLuint Particle::getTexture()
{
	return texture;
}
