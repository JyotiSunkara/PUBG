#include "audio/soundmanager.h"

#include "AL/al.h"
#include "AL/alc.h"

#include <glm/glm.hpp>
#include <fstream>
#include <iostream>
#include <cstdlib>
using namespace std;

// No-Fly Zone owes thanks to the following websites for their usefulness:
//	- for loading wav files without ALUT: http://www.dunsanyinteractive.com/blogs/oliver/?p=72
//		- a few changes had to be made to port this to pure C, including the use of portable types (uint32_t, etc.) instead of long, short, etc.
//  - for initializing/deinitializing OpenAL without ALUT: http://kcat.strangesoft.net/openal-tutorial.html

/*
 * (From http://www.dunsanyinteractive.com/blogs/oliver/?p=72)
 * Struct that holds the RIFF data of the Wave file.
 * The RIFF data is the meta data information that holds,
 * the ID, size and format of the wave file
 */
struct RIFF_Header
{
	int8_t chunkID[4];
	int32_t chunkSize;			// size not including chunkSize or chunkID; we use int32_t for guaranteed type sizes (original code uses long)
	int8_t format[4];
};

/*
 * (From http://www.dunsanyinteractive.com/blogs/oliver/?p=72)
 * Struct to hold fmt subchunk data for WAVE files.
 */
struct WAVE_Format
{
	int8_t subChunkID[4];
	int32_t subChunkSize;
	int16_t audioFormat;
	int16_t numChannels;
	int32_t sampleRate;
	int32_t byteRate;
	int16_t blockAlign;
	int16_t bitsPerSample;
};

/*
 * (From http://www.dunsanyinteractive.com/blogs/oliver/?p=72)
 * Struct to hold the data of the wave file
 */
struct WAVE_Data
{
	int8_t subChunkID[4]; 		// should contain the word data
	int32_t subChunk2Size; 		// stores the size of the data block
};

const int SoundManager::MAX_SOURCES = 200;

SoundManager *SoundManager::instance = NULL;

SoundManager::SoundManager()
{
	volume = 1.0;											// default full volume
	sources = new ALuint[MAX_SOURCES];						// how many sounds we can play at once (hardware permitting)
	nextFreeSourceIndex = 0;								// next index in "sources" is probably free, duh

	// open our audio device
    alDevice = alcOpenDevice(NULL);
    if(!alDevice)
    {
        cerr << "SoundManager::SoundManager() could not open audio device" << endl;
        exit(1);
    }

    // create an OpenAL context
    alContext = alcCreateContext(alDevice, NULL);
    alcMakeContextCurrent(alContext);
    if(!alContext)
    {
        cerr << "SoundManager::SoundManager() could not create audio context" << endl;
        exit(1);
    }

    // configure our OpenAL model
    alGenSources(MAX_SOURCES, sources);						// initialize OpenAL source objects (these represent "playable" resources)
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);			// I believe this is the default distance model anyways...
    alDopplerFactor(1.5);									// good for increasing the strength of doppler effects, if velocity is used
}

SoundManager::~SoundManager()
{
	// deallocate OpenAL resources
    alDeleteSources(MAX_SOURCES, sources);
	delete[] sources;

	// shut down our context and close the audio device
	alcMakeContextCurrent(NULL);
    alcDestroyContext(alContext);
    alcCloseDevice(alDevice);
}

SoundManager *SoundManager::getInstance()
{
	if(instance == NULL)
	{
		instance = new SoundManager();
	}

	return instance;
}

ALuint SoundManager::loadWAV(string filename)
{
	ifstream soundFile;
	struct WAVE_Format wave_format;
	struct RIFF_Header riff_header;
	struct WAVE_Data wave_data;
	unsigned char *data;

	ALsizei size;
	ALsizei frequency;
	ALenum format = 0;
	ALenum error;
	ALuint buffer;

	// attempt to open the file to see if it exists and is readable
	soundFile.open(filename.c_str(), ifstream::binary);
	if(!soundFile.is_open())
	{
		cerr << "SoundManager::loadWAV() could not load " << filename << "---does the file exist?" << endl;
		exit(1);
	}

	// read in the first chunk into the struct
    soundFile.read((char*)&riff_header, sizeof(struct RIFF_Header));

	// check for RIFF and WAVE tag in memeory
	if((riff_header.chunkID[0] != 'R' || riff_header.chunkID[1] != 'I' || riff_header.chunkID[2] != 'F' || riff_header.chunkID[3] != 'F') ||
       (riff_header.format[0] !=  'W' || riff_header.format[1] !=  'A' || riff_header.format[2] !=  'V' || riff_header.format[3] !=  'E'))
	{
		cerr << "SoundManager::loadWAV() did not detect the RIFF/WAVE header combination in " << filename << endl;
		exit(1);
	}

	// read in the 2nd chunk for the wave info
	soundFile.read((char*)&wave_format, sizeof(struct WAVE_Format));

	// make sure the wave format is something we can work with
	if (wave_format.subChunkID[0] != 'f' || wave_format.subChunkID[1] != 'm' || wave_format.subChunkID[2] != 't' || wave_format.subChunkID[3] != ' ')
	{
		cerr << "SoundManager::loadWAV() detected invalid wave format in " << filename << endl;
		exit(1);
	}

	// check for extra parameters
	if (wave_format.subChunkSize > 16)
	{
        soundFile.seekg(sizeof(int16_t), ios_base::cur);
	}

	// read in the the last byte of data before the sound file
	soundFile.read((char*)&wave_data, sizeof(struct WAVE_Data));

	// check for data tag in memory
	if (wave_data.subChunkID[0] != 'd' || wave_data.subChunkID[1] != 'a' || wave_data.subChunkID[2] != 't' || wave_data.subChunkID[3] != 'a')
	{
		cerr << "SoundManager::loadWAV() detected invalid data header in " << filename << endl;
		exit(1);
	}

	// allocate memory for wave data
	data = (unsigned char*)malloc(wave_data.subChunk2Size);

	// read in the sound data into the soundData variable
	if(!soundFile.read((char*)data, wave_data.subChunk2Size))
	{
		cerr << "SoundManager::loadWAV() could not load wave data in " << filename << " into struct" << endl;
		exit(1);
	}

	// now we set the variables that we passed in with the data from the structs
	size = wave_data.subChunk2Size;
	frequency = wave_format.sampleRate;

	// the format is worked out by looking at the number of channels and the bits per sample.
	if (wave_format.numChannels == 1)
	{
		if (wave_format.bitsPerSample == 8 )
			format = AL_FORMAT_MONO8;
		else if (wave_format.bitsPerSample == 16)
			format = AL_FORMAT_MONO16;
	}
	else if (wave_format.numChannels == 2)
	{
		if (wave_format.bitsPerSample == 8 )
			format = AL_FORMAT_STEREO8;
		else if (wave_format.bitsPerSample == 16)
			format = AL_FORMAT_STEREO16;
	}

	// now we put our data into the OpenAL buffer and check for an error
	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, (void*)data, size, frequency);
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		cerr << "SoundManager::loadWAV() encountered AL error " << (int)error << " when buffering " << filename << endl;
        exit(1);
	}

	// clean up
	soundFile.close();

	// finally, return the buffer object
	return buffer;
}

ALuint SoundManager::playSound(ALuint buffer)
{
    ALuint source = getUnusedSource();								// retrieve a source that is not playing

    if(source > 0)
    {
        alSourcei(source, AL_BUFFER, buffer);						// assign the source to use the buffered sound
        alSourcef(source, AL_GAIN, volume);							// assign system volume
        alSourcei(source, AL_LOOPING, AL_FALSE);					// we want the sound to not loop
        alSource3f(source, AL_POSITION, 0.0, 0.0, 0.0);				// default 3D sound position
        alSource3f(source, AL_VELOCITY, 0.0, 0.0, 0.0);				// default 3D sound velocity
        alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);				// no 3D effects
        alSourcef(source, AL_REFERENCE_DISTANCE, FLT_MAX);			// no 3D effects
        alSourcef(source, AL_MAX_DISTANCE, FLT_MAX);				// no 3D effects
        alSourcePlay(source);										// play sound immediately
    }

    return source;
}

ALuint SoundManager::playSound(ALuint buffer, glm::vec3 pos, double refDist, double maxDist)
{
    ALuint source = getUnusedSource();

    if(source > 0)
    {
        alSourcei(source, AL_BUFFER, buffer);
        alSourcef(source, AL_GAIN, volume);
        alSourcei(source, AL_LOOPING, AL_FALSE);
        alSource3f(source, AL_POSITION, pos.x, pos.y, pos.z);
        alSource3f(source, AL_VELOCITY, 0.0, 0.0, 0.0);
        alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);
        alSourcef(source, AL_REFERENCE_DISTANCE, refDist);
        alSourcef(source, AL_MAX_DISTANCE, maxDist);
        alSourcePlay(source);
    }

    return source;
}

ALuint SoundManager::loopSound(ALuint buffer)
{
    ALuint source = getUnusedSource();

    if(source > 0)
    {
        alSourcei(source, AL_BUFFER, buffer);
        alSourcef(source, AL_GAIN, volume);
        alSourcei(source, AL_LOOPING, AL_TRUE);
        alSource3f(source, AL_POSITION, 0.0, 0.0, 0.0);
        alSource3f(source, AL_VELOCITY, 0.0, 0.0, 0.0);
        alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);
        alSourcef(source, AL_REFERENCE_DISTANCE, FLT_MAX);
        alSourcef(source, AL_MAX_DISTANCE, FLT_MAX);
        alSourcePlay(source);
    }

    return source;
}

ALuint SoundManager::loopSound(ALuint buffer, glm::vec3 pos, double refDist, double maxDist)
{
    ALuint source = getUnusedSource();

    if(source > 0)
    {
        alSourcei(source, AL_BUFFER, buffer);
        alSourcef(source, AL_GAIN, volume);
        alSourcei(source, AL_LOOPING, AL_TRUE);
        alSource3f(source, AL_POSITION, pos.x, pos.y, pos.z);
        alSource3f(source, AL_VELOCITY, 0.0, 0.0, 0.0);
        alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);
        alSourcef(source, AL_REFERENCE_DISTANCE, refDist);
        alSourcef(source, AL_MAX_DISTANCE, maxDist);
        alSourcePlay(source);
    }

    return source;
}

bool SoundManager::isSoundPlaying(ALuint source)
{
	int state;
	bool result = false;

	if(source > 0)
	{
		// retrieve the current state of the OpenAL source
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		result = state == AL_PLAYING || state == AL_PAUSED;
	}

	return result;
}

void SoundManager::stop(ALuint source)
{
	int state;

    if(source > 0)
    {
		// only stop if it's currently playing or paused
		alGetSourcei(source, AL_SOURCE_STATE, &state);
        if(state == AL_PLAYING || state == AL_PAUSED)
        {
            alSourceStop(source);
		}
	}
}

void SoundManager::setVolume(double volume)
{
    int state;
    int i;

    // record the desired volume for later
    this -> volume = volume;

	// we may need to change the volume of an already playing piece
	for(i = 0; i < MAX_SOURCES; i ++)
	{
		if(sources[i] > 0)
		{
			alGetSourcei(sources[i], AL_SOURCE_STATE, &state);
			if(state == AL_PAUSED || state == AL_PLAYING)
			{
				alSourcef(sources[i], AL_GAIN, volume);
			}
		}
	}
}

double SoundManager::getVolume()
{
	return volume;
}

void SoundManager::pause(ALuint source)
{
	int state;

    if(source != 0)
    {
		alGetSourcei(source, AL_SOURCE_STATE, &state);
        if(state == AL_PLAYING)
        {
            alSourcePause(source);
		}
	}
}

void SoundManager::resume(ALuint source)
{
	int state;

    if(source != 0)
    {
		alGetSourcei(source, AL_SOURCE_STATE, &state);
        if(state == AL_PAUSED)
        {
            alSourcePlay(source);
		}
	}
}

void SoundManager::pauseAll()
{
    ALuint current;

    int state;
    int i = 0;

    while(i < MAX_SOURCES)
    {
        current = sources[i++];

        alGetSourcei(current, AL_SOURCE_STATE, &state);
        if(state == AL_PLAYING)
        {
            alSourcePause(current);
		}
    }
}

void SoundManager::resumeAll()
{
    ALuint current;

    int state;
    int i = 0;

    while(i < MAX_SOURCES)
    {
        current = sources[i++];

        alGetSourcei(current, AL_SOURCE_STATE, &state);
        if(state == AL_PAUSED)
        {
            alSourcePlay(current);
		}
    }
}

void SoundManager::stopAllPaused()
{
    ALuint current;

    int state;
    int i = 0;

    while(i < MAX_SOURCES)
    {
        current = sources[i++];

        alGetSourcei(current, AL_SOURCE_STATE, &state);
        if(state == AL_PAUSED)
        {
            alSourceStop(current);
		}
    }
}

void SoundManager::stopAll()
{
    ALuint current;

    int state;
    int i = 0;

    while(i < MAX_SOURCES)
    {
        current = sources[i++];

        alGetSourcei(current, AL_SOURCE_STATE, &state);
        if(state == AL_PAUSED || state == AL_PLAYING)
        {
            alSourceStop(current);
		}
    }
}

void SoundManager::setListenerPos(glm::vec3 pos)
{
    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
}

void SoundManager::setListenerVelocity(glm::vec3 velocity)
{
    alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

void SoundManager::setListenerOrientation(glm::vec3 forward, glm::vec3 up)
{
    float orientation[] = {forward.x, forward.y, forward.z,
						   up.x, up.y, up.z};
    alListenerfv(AL_ORIENTATION, orientation);
}

ALuint SoundManager::getUnusedSource()
{
	int state;
	int i = nextFreeSourceIndex;
	ALuint result = 0;

	// start at an index that is an educated guess for a sound that might be free
	do
	{
		// get the state of the next source that might be available
		alGetSourcei(sources[i], AL_SOURCE_STATE, &state);
		if(state == AL_STOPPED || state == AL_INITIAL)
		{
			result = sources[i];
			nextFreeSourceIndex = (i + 1) % MAX_SOURCES;
		}
		else
		{
			i = (i + 1) % MAX_SOURCES;
		}
	}
	while(result == 0 && i != nextFreeSourceIndex);

	// return the AL identifier for the free source
	return result;
}
