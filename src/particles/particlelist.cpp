#include "particles/particlelist.h"
#include "particles/particleconfig.h"

#include "util/loadtexture.h"

#include "glm/glm.hpp"
using namespace glm;

ParticleConfig *muzzleFlash = NULL;
ParticleConfig *smoke = NULL;
ParticleConfig *spark = NULL;
ParticleConfig *impactFlare = NULL;
ParticleConfig *dirtSpray = NULL;

ParticleConfig *trailSmoke = NULL;
ParticleConfig *trailFire = NULL;
ParticleConfig *explodeEmitter = NULL;

void initParticleList()
{
	muzzleFlash = new ParticleConfig();
	muzzleFlash -> motionLimits[0] = vec3(0.0);
	muzzleFlash -> motionLimits[1] = vec3(0.0);
	muzzleFlash -> spinLimits[0] = 0.0;
	muzzleFlash -> spinLimits[1] = 0.0;
	muzzleFlash -> startSizeLimits[0] = 0.45;
	muzzleFlash -> startSizeLimits[1] = 0.45;
	muzzleFlash -> endSizeLimits[0] = 0.45;
	muzzleFlash -> endSizeLimits[1] = 0.45;
	muzzleFlash -> startColorLimits[0] = vec4(1.0, 1.0, 1.0, 1.0);
	muzzleFlash -> startColorLimits[1] = vec4(1.0, 1.0, 1.0, 1.0);
	muzzleFlash -> endColorLimits[0] = vec4(1.0, 1.0, 1.0, 0.0);
	muzzleFlash -> endColorLimits[1] = vec4(1.0, 1.0, 1.0, 0.0);
	muzzleFlash -> lifeLimits[0] = 0.05;
	muzzleFlash -> lifeLimits[1] = 0.05;
    muzzleFlash -> initialGravityLimits[0] = 0.0;
    muzzleFlash -> initialGravityLimits[1] = 0.0;
    muzzleFlash -> gravityRateLimits[0] = 0.0;
    muzzleFlash -> gravityRateLimits[1] = 0.0;
    muzzleFlash -> additiveBlending = false;
    muzzleFlash -> texture = loadPNG("../png/muzzle-flash.png", true);

	smoke = new ParticleConfig();
	smoke -> motionLimits[0] = vec3(-0.008, 0.003, -0.008);
	smoke -> motionLimits[1] = vec3(0.008, 0.006, 0.008);
	smoke -> spinLimits[0] = -0.0008;
	smoke -> spinLimits[1] = 0.0008;
	smoke -> startSizeLimits[0] = 0.1;
	smoke -> startSizeLimits[1] = 0.1;
	smoke -> endSizeLimits[0] = 0.7;
	smoke -> endSizeLimits[1] = 0.85;
	smoke -> startColorLimits[0] = vec4(0.5, 0.5, 0.5, 1.0);
	smoke -> startColorLimits[1] = vec4(0.5, 0.5, 0.5, 1.0);
	smoke -> endColorLimits[0] = vec4(0.5, 0.5, 0.5, 0.0);
	smoke -> endColorLimits[1] = vec4(0.5, 0.5, 0.5, 0.0);
	smoke -> lifeLimits[0] = 0.9;
	smoke -> lifeLimits[1] = 1.2;
    smoke -> initialGravityLimits[0] = 0.0;
    smoke -> initialGravityLimits[1] = 0.0;
    smoke -> gravityRateLimits[0] = 0.0;
    smoke -> gravityRateLimits[1] = 0.0;
    smoke -> additiveBlending = false;
    smoke -> texture = loadPNG("../png/smoke.png", true);

	spark = new ParticleConfig();
	spark -> motionLimits[0] = vec3(-0.07, 0.0, -0.07);
	spark -> motionLimits[1] = vec3(0.07, 0.0, 0.07);
	spark -> spinLimits[0] = 0.0;
	spark -> spinLimits[1] = 0.0;
	spark -> startSizeLimits[0] = 0.05;
	spark -> startSizeLimits[1] = 0.3;
	spark -> endSizeLimits[0] = 0.0;
	spark -> endSizeLimits[1] = 0.0;
	spark -> startColorLimits[0] = vec4(1.0, 1.0, 0.5, 1.0);
	spark -> startColorLimits[1] = vec4(1.0, 1.0, 1.0, 1.0);
	spark -> endColorLimits[0] = vec4(1.0, 1.0, 0.5, 1.0);
	spark -> endColorLimits[1] = vec4(1.0, 1.0, 0.5, 1.0);
	spark -> lifeLimits[0] = 1.0;
	spark -> lifeLimits[1] = 1.0;
    spark -> initialGravityLimits[0] = 0.05;
    spark -> initialGravityLimits[1] = 0.1;
    spark -> gravityRateLimits[0] = -0.3;
    spark -> gravityRateLimits[1] = -0.3;
    spark -> additiveBlending = false;
    spark -> texture = loadPNG("../png/spark.png", false);

	impactFlare = new ParticleConfig();
	impactFlare -> motionLimits[0] = vec3(0.0);
	impactFlare -> motionLimits[1] = vec3(0.0);
	impactFlare -> spinLimits[0] = 0.0;
	impactFlare -> spinLimits[1] = 0.0;
	impactFlare -> startSizeLimits[0] = 1.0;
	impactFlare -> startSizeLimits[1] = 1.0;
	impactFlare -> endSizeLimits[0] = 0.0;
	impactFlare -> endSizeLimits[1] = 0.0;
	impactFlare -> startColorLimits[0] = vec4(1.0, 0.9, 0.2, 1.0);
	impactFlare -> startColorLimits[1] = vec4(1.0, 0.9, 0.2, 1.0);
	impactFlare -> endColorLimits[0] = vec4(1.0, 0.9, 0.2, 0.0);
	impactFlare -> endColorLimits[1] = vec4(1.0, 0.9, 0.2, 0.0);
	impactFlare -> lifeLimits[0] = 0.1;
	impactFlare -> lifeLimits[1] = 0.1;
    impactFlare -> initialGravityLimits[0] = 0.0;
    impactFlare -> initialGravityLimits[1] = 0.0;
    impactFlare -> gravityRateLimits[0] = 0.0;
    impactFlare -> gravityRateLimits[1] = 0.0;
    impactFlare -> additiveBlending = false;
    impactFlare -> texture = loadPNG("../png/flare.png", true);

	dirtSpray = new ParticleConfig();
	dirtSpray -> motionLimits[0] = vec3(-0.025, 0.0, -0.025);
	dirtSpray -> motionLimits[1] = vec3(0.025, 0.0, 0.025);
	dirtSpray -> spinLimits[0] = 0.0;
	dirtSpray -> spinLimits[1] = 0.0;
	dirtSpray -> startSizeLimits[0] = 0.1;
	dirtSpray -> startSizeLimits[1] = 0.2;
	dirtSpray -> endSizeLimits[0] = 1.0;
	dirtSpray -> endSizeLimits[1] = 1.5;
	dirtSpray -> startColorLimits[0] = vec4(0.32, 0.18, 0.04, 2.0) * 0.5f;
	dirtSpray -> startColorLimits[1] = vec4(0.37, 0.23, 0.09, 2.0) * 0.5f;
	dirtSpray -> endColorLimits[0] = vec4(0.32, 0.18, 0.04, 2.0) * 0.5f;
	dirtSpray -> endColorLimits[1] = vec4(0.37, 0.23, 0.09, 2.0) * 0.5f;
	dirtSpray -> lifeLimits[0] = 0.5;
	dirtSpray -> lifeLimits[1] = 0.8;
    dirtSpray -> initialGravityLimits[0] = 0.035;
    dirtSpray -> initialGravityLimits[1] = 0.06;
    dirtSpray -> gravityRateLimits[0] = -0.3;
    dirtSpray -> gravityRateLimits[1] = -0.3;
    dirtSpray -> additiveBlending = false;
    dirtSpray -> texture = loadPNG("../png/dirt-spray.png", true);

	trailSmoke = new ParticleConfig();
	trailSmoke -> motionLimits[0] = vec3(-0.001, 0.001, -0.001);
	trailSmoke -> motionLimits[1] = vec3(0.001, 0.003, 0.001);
	trailSmoke -> spinLimits[0] = -0.001;
	trailSmoke -> spinLimits[1] = 0.001;
	trailSmoke -> startSizeLimits[0] = 0.5;
	trailSmoke -> startSizeLimits[1] = 0.7;
	trailSmoke -> endSizeLimits[0] = 1.5;
	trailSmoke -> endSizeLimits[1] = 1.8;
	trailSmoke -> startColorLimits[0] = vec4(0.5, 0.5, 0.5, 0.2);
	trailSmoke -> startColorLimits[1] = vec4(0.5, 0.5, 0.5, 0.4);
	trailSmoke -> endColorLimits[0] = vec4(0.5, 0.5, 0.5, 0.0);
	trailSmoke -> endColorLimits[1] = vec4(0.5, 0.5, 0.5, 0.0);
	trailSmoke -> lifeLimits[0] = 2.5;
	trailSmoke -> lifeLimits[1] = 3.5;
    trailSmoke -> initialGravityLimits[0] = 0.0;
    trailSmoke -> initialGravityLimits[1] = 0.0;
    trailSmoke -> gravityRateLimits[0] = 0.0;
    trailSmoke -> gravityRateLimits[1] = 0.0;
    trailSmoke -> additiveBlending = false;
    trailSmoke -> texture = loadPNG("../png/smoke.png", true);

	trailFire = new ParticleConfig();
	trailFire -> motionLimits[0] = vec3(-0.001, 0.001, -0.001);
	trailFire -> motionLimits[1] = vec3(0.001, 0.003, 0.001);
	trailFire -> spinLimits[0] = 0.0;
	trailFire -> spinLimits[1] = 0.0;
	trailFire -> randomOrientation = false;
	trailFire -> startSizeLimits[0] = 0.5;
	trailFire -> startSizeLimits[1] = 2.0;
	trailFire -> endSizeLimits[0] = 0.0;
	trailFire -> endSizeLimits[1] = 0.0;
	trailFire -> startColorLimits[0] = vec4(1.0, 0.8, 0.1, 0.2);
	trailFire -> startColorLimits[1] = vec4(1.0, 0.8, 0.3, 0.9);
	trailFire -> endColorLimits[0] = vec4(1.0, 0.4, 0.0, 0.0);
	trailFire -> endColorLimits[1] = vec4(1.0, 0.7, 0.2, 0.0);
	trailFire -> lifeLimits[0] = 0.05;
	trailFire -> lifeLimits[1] = 0.4;
    trailFire -> initialGravityLimits[0] = 0.0;
    trailFire -> initialGravityLimits[1] = 0.0;
    trailFire -> gravityRateLimits[0] = 0.3;
    trailFire -> gravityRateLimits[1] = 0.3;
    trailFire -> additiveBlending = false;
    trailFire -> texture = loadPNG("../png/flame.png", true);

    explodeEmitter = new ParticleConfig();
    explodeEmitter -> motionLimits[0] = vec3(-0.06, 0.0, -0.06);
    explodeEmitter -> motionLimits[1] = vec3(0.06, 0.0, 0.06);
    explodeEmitter -> spinLimits[0] = 0.0;
    explodeEmitter -> spinLimits[1] = 0.0;
    explodeEmitter -> lifeLimits[0] = 0.7;
    explodeEmitter -> lifeLimits[1] = 1.2;
    explodeEmitter -> initialGravityLimits[0] = 0.04;
    explodeEmitter -> initialGravityLimits[1] = 0.07;
    explodeEmitter -> gravityRateLimits[0] = -0.25;
    explodeEmitter -> gravityRateLimits[1] = -0.25;
    explodeEmitter -> childEmissionInterval = 0.01;
    explodeEmitter -> children.push_back(trailFire);
    explodeEmitter -> children.push_back(trailSmoke);
}

void deinitParticleList()
{
	delete muzzleFlash;
	delete smoke;
	delete spark;
	delete impactFlare;
	delete dirtSpray;
	delete trailSmoke;
	delete trailFire;
	delete explodeEmitter;
}
