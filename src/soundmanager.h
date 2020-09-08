#pragma once

#include "glm/glm.hpp"
#include "AL/al.h"
#include "AL/alc.h"

#include <string>

class SoundManager
{
private:
	static const int MAX_SOURCES;

	static SoundManager *instance;			// singleton instance

	ALCdevice *alDevice;                   // our audio device
    ALCcontext *alContext;                 // our OpenAL context

	ALuint *sources;						// sources playing or available to be played
	int nextFreeSourceIndex;				// best guess as to where the next available source is, in "sources"

	double volume;							// volume of entire sound system

	SoundManager();							// force use of getInstance()

	ALuint getUnusedSource();				// returns an ALuint from "sources" that is currently unused

public:
	static SoundManager *getInstance();		    // singleton design pattern

	~SoundManager();                            // deallocates OpenAL resources

    ALuint loadWAV(std::string file);           // load a .WAV file and return an OpenAL buffer associated with it

	void setVolume(double volume);			    // sets or gets volume of entire system
	double getVolume();

	// play sound with default settings
	ALuint playSound(ALuint);

	// play sound with specified 3D settings
	ALuint playSound(ALuint, glm::vec3 pos, double refDist, double maxDist);

	// loop sound with default settings
	ALuint loopSound(ALuint);

	// loop sound with specified 3D settings
	ALuint loopSound(ALuint, glm::vec3 pos, double refDist, double maxDist);

	// is the given source currently playing or paused?
	bool isSoundPlaying(ALuint);

	// sound control---pass in an OpenAL source, get the desired effect
	void stop(ALuint);
	void pause(ALuint);
	void resume(ALuint);

	// global sound controls
	void pauseAll();
	void resumeAll();
	void stopAllPaused();
	void stopAll();

	// 3D listener control
	void setListenerPos(glm::vec3);
	void setListenerVelocity(glm::vec3);
	void setListenerOrientation(glm::vec3, glm::vec3);
};
