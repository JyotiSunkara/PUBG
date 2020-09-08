#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include <vector>

class ParticleConfig
{
public:
	// the idea here is that the ParticleConfig is used to generate a Particle instance using random choices of the
	// ranges below for each property: someProperty[0] is the min, and someProperty[1] is the max

	glm::vec3 motionLimits[2];					// motion limits (used when randomly assigning motion at particle creation)

	float spinLimits[2];						// spin limits (used when randomly assigning spin at particle creation)
	bool randomOrientation;

	float startSizeLimits[2];					// same for particle size
	float endSizeLimits[2];

	glm::vec4 startColorLimits[2];				// same for particle colour
	glm::vec4 endColorLimits[2];

	float lifeLimits[2];						// pick a life value between lifeLimits[0] and lifeLimits[1]

	float initialGravityLimits[2];				// strength of initial gravity value and also gravity accumulation
	float gravityRateLimits[2];

	bool additiveBlending;						// not used

	GLuint texture;								// the texture ID we use

	float childEmissionInterval;				// how often to emit other particles and what to emit
	std::vector<ParticleConfig*> children;

	ParticleConfig();
	~ParticleConfig();
};
