#include "particleconfig.h"

#include "glm/glm.hpp"
using namespace glm;

ParticleConfig::ParticleConfig()
{
	// assign some default values, self-explanatory //

	motionLimits[0] = vec3(0.0, 0.0, 0.0);
	motionLimits[1] = vec3(0.0, 0.0, 0.0);

	spinLimits[0] = 0.0;
	spinLimits[1] = 0.0;
	randomOrientation = true;

	startSizeLimits[0] = 0.0;
	startSizeLimits[1] = 0.0;
	endSizeLimits[0] = 0.0;
	endSizeLimits[1] = 0.0;

	startColorLimits[0] = vec4(0.0, 0.0, 0.0, 0.0);
	startColorLimits[1] = vec4(0.0, 0.0, 0.0, 0.0);
	endColorLimits[0] = vec4(0.0, 0.0, 0.0, 0.0);
	endColorLimits[1] = vec4(0.0, 0.0, 0.0, 0.0);

	lifeLimits[0] = 1.0;
	lifeLimits[1] = 1.0;

	initialGravityLimits[0] = 0.0;
	initialGravityLimits[1] = 0.0;
	gravityRateLimits[0] = 0.0;
	gravityRateLimits[1] = 0.0;

	additiveBlending = false;

	texture = 0;

	childEmissionInterval = 0.0;
}

ParticleConfig::~ParticleConfig() { }
