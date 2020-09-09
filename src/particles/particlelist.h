#pragma once

// admittedly, this isn't the best way to store the particles we want to use; ideally, we should be reading these from a file
// and then storing them somewhere...but this is the quickest and easiest solution given the simplicity of this project, which
// doesn't really merit a full-blown particle engine and editor like Gateway does

class ParticleConfig;

extern "C" ParticleConfig *muzzleFlash;
extern "C" ParticleConfig *smoke;
extern "C" ParticleConfig *spark;
extern "C" ParticleConfig *impactFlare;
extern "C" ParticleConfig *dirtSpray;

extern "C" ParticleConfig *trailSmoke;
extern "C" ParticleConfig *trailFire;
extern "C" ParticleConfig *explodeEmitter;

void initParticleList();
void deinitParticleList();
