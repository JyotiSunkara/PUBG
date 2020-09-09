#include "particles/particlemanager.h"
#include "particles/particleconfig.h"
#include "particles/particle.h"

#include "util/shader.h"

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
using namespace glm;

#include <cstring>
#include <vector>
#include <iostream>
using namespace std;

ParticleManager::ParticleManager(int maxParticles)
{
	numInActivePool = 0;
    freeParticleIndex = 0;
    this -> maxParticles = maxParticles;

    // allocate space for our particles
	particlePool = new Particle[maxParticles];
	activeParticles = new Particle*[maxParticles];

	// setup our GPU memory and shader programs
	setupVBOs();
	loadShader();
}

ParticleManager::~ParticleManager()
{
	// free OpenGL memory
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	// free particle data
	delete[] particlePool;					// de-allocates particle dynamic memory (the actual memory used by all particles)
	delete[] activeParticles;				// de-allocates our array of pointers (what they point to was freed in the line above)
	delete[] vertexAttribData;				// free the data associated with our vertex attributes
}

void ParticleManager::setupVBOs()
{
	// initialize buffer store for generic vertex attributes that we pass to the shader
    vertexAttribData = new float[maxParticles * 9];

	// set up our vertex buffers
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);

	// we store all vertex attributes inside a single vertex buffer for increased efficiency (fewer calls to glBufferSubData() later on)
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,							// target should always be GL_ARRAY_BUFFER here
				 sizeof(GLfloat) * maxParticles * 9,		// 9 floats (3 for pos, 4 for color, 1 each for size and angle) per particle
				 NULL,										// no data currently
				 GL_DYNAMIC_DRAW);							// performance hint for "written and drawn frequently"

	// set up generic vertex attributes for particle positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 9, (GLvoid*)0);
	glVertexAttribDivisor(0, 1);		// positions advance only once per primitive

	// set up generic vertex attributes for particle colour
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 9, (GLvoid*)(sizeof(GLfloat) * 3));
	glVertexAttribDivisor(1, 1);		// colours advance only once per primitive

	// set up generic vertex attributes for particle size
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 9, (GLvoid*)(sizeof(GLfloat) * 7));
	glVertexAttribDivisor(2, 1);		// sizes advance only once per primitive

	// set up generic vertex attributes for particle angle
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 9, (GLvoid*)(sizeof(GLfloat) * 8));
	glVertexAttribDivisor(3, 1);		// angles advance only once per primitive
}

void ParticleManager::loadShader()
{
	shader = new Shader("../shaders/particle.vert", "../shaders/particle.frag");
	shader -> bindAttrib("a_Position", 0);
	shader -> bindAttrib("a_Color", 1);
	shader -> bindAttrib("a_Size", 2);
	shader -> bindAttrib("a_Angle", 3);
	shader -> link();
	shader -> bind();
	shader -> uniform1i("u_Texture", 0);
	shader -> unbind();
}

void ParticleManager::add(ParticleConfig *config, vec3 pos, float lifeFactor)
{
	Particle *toAdd = getFreeParticle();
	if(toAdd)
	{
		// success, configure the particle and add it to the list of active particles in the system
		toAdd -> configure(config, pos, lifeFactor, this);
		insertIntoActivePool(toAdd);
	}
}

void ParticleManager::update(double dt)
{
	GLuint currentTexture = 0;
	Particle **curr = &activeParticles[0];
	int i = 0;

	// clear the texture index dividers
	textureIndices.clear();

	// reset the recycle count (how many particles the recycling routine should cover)
	numToRecycle = 0;

	// iterate through all particles
	while(i < numInActivePool)
	{
		// track texture object used by particle to determine how to divide rendering by texture
		if(currentTexture != (*curr) -> getTexture())
        {
            currentTexture = (*curr) -> getTexture();
            textureIndices.push_back(i);
        }

		// update the particle
		(*curr) -> update(dt);

		// if this particle is expected to die, increase the recycle count
		if((*curr) -> getDead())
		{
			numToRecycle ++;
		}

		// advance to next particle
		curr ++;
		i ++;
	}

	// add a final texture index divider to indicate the end of the last texture group
	if(numInActivePool > 0)
	{
		textureIndices.push_back(i);
	}
}

void ParticleManager::recycle()
{
    Particle **curr = &activeParticles[0];
    int numRecycled = 0;
    int i = 0;

	// either we go through all the particles or we recycle all the ones we need to erase (early exit to prevent unnecessary iteration)
    while(numRecycled < numToRecycle && i < numInActivePool)
    {
        if((*curr) -> getDead())
        {
            // shift memory over dead particle---this places the next particle at our current iterator position
            memmove(&activeParticles[i], &activeParticles[i + 1], sizeof(Particle*) * (numInActivePool - i - 1));
            numInActivePool --;
			numRecycled ++;
        }
        else
        {
			// advance to next particle if we did nothing
			curr ++;
			i ++;
        }
    }
}

void ParticleManager::render(mat4 &projection, mat4 &view, vec3 &cameraRight, vec3 &cameraUp)
{
	Particle **curr;			// allows easy iteration over particles we want to render
	float *attribPtr;			// allows easy iteration over the array containing vertex attributes we want to send to GL

	int groupStartIndex;		// index into the active pool where the current particle group starts
	int groupSize;				// number of particles in our current particle group

	int i;
	int j;

	// turn on the appropriate blending mode
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// make sure the first texture unit is the one we bind to
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// set up appropriate rendering options for point sprites
	glDepthMask(GL_FALSE);

	// enable our shader
    shader -> bind();
    shader -> uniformMatrix4fv("u_ProjectionMatrix", 1, value_ptr(projection));
    shader -> uniformMatrix4fv("u_ViewMatrix", 1, value_ptr(view));
	shader -> uniformVec3("u_CameraRight", cameraRight);
	shader -> uniformVec3("u_CameraUp", cameraUp);

	// bring in our vertex object states
	glBindVertexArray(vao);

	// render our particles according to texture group (particles have already been sorted by texture group by this time)
    for(i = 0; i < (int)textureIndices.size() - 1; i ++)
    {
		// get the first and last indices of where this particle group appears in our pool of active particles
		groupStartIndex = textureIndices[i];
		groupSize = textureIndices[i + 1] - groupStartIndex;

		// iterate through the members of this group and retrieve their state for transmission to the graphics card
		curr = &activeParticles[groupStartIndex];
		attribPtr = vertexAttribData;
		for(j = 0; j < groupSize; j ++)
		{
			// insert particle position attribute into appropriate slot in buffer we'll be sending to the graphics card
			(*curr) -> getPos(attribPtr);
			attribPtr += 3;

			// insert particle color attribute into appropriate slot in buffer we'll be sending to the graphics card
			(*curr) -> getColor(attribPtr);
			attribPtr += 4;

			// lastly, insert particle size and rotation
			(*attribPtr++) = (*curr) -> getSize();
			(*attribPtr++) = (*curr) -> getAngle();

			// advance to next particle
			curr ++;
		}

		// pass attribs along to graphics card for this texture group; this is more efficient than making separate calls for each attribute
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * groupSize * 9, &vertexAttribData[0]);

		// bind the texture we need and draw the group of particles
        glBindTexture(GL_TEXTURE_2D, activeParticles[groupStartIndex] -> getTexture());
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, groupSize);
    }

    glDepthMask(GL_TRUE);
}

void ParticleManager::insertIntoActivePool(Particle *particle)
{
    GLuint tex = particle -> getTexture();
    int start = 0;
    int end = numInActivePool - 1;
    int mid = start + (end - start) / 2;
    bool found = false;
    GLuint currTex;

    // do an ordered insert into the active particles pointer array
    while(!found && start <= end)
    {
        mid = start + (end - start) / 2;
        currTex = activeParticles[mid] -> getTexture();

        if(tex < currTex)
        {
            end = mid - 1;
        }
        else if(tex > currTex)
        {
            start = mid + 1;
        }
        else
        {
            found = true;
        }
    }

    // move the memory to make room for the new particle
    memmove(&activeParticles[mid + 1], &activeParticles[mid], sizeof(Particle*) * (numInActivePool - mid));

    // insert new particle
    activeParticles[mid] = particle;
    numInActivePool ++;
}

Particle *ParticleManager::getFreeParticle()
{
	int i = freeParticleIndex;
	Particle *curr = &particlePool[i];
	Particle *result = NULL;

	// don't bother trying to add a particle if we're fresh out
	if(numInActivePool < maxParticles)
	{
		// start at an index that is an educated guess for a particle that might be free
		do
		{
			// is the particle unavailable?
			if(!curr -> getDead())
			{
				// particle in use, advance to the next one
				i ++;
				if(i >= maxParticles)		// wrap around
				{
					i = 0;
					curr = &particlePool[0];
				}
				else
				{

					// advance to the next particle
					curr ++;
				}
			}
			else
			{
				// we found a free particle; record it and indicate where we should start looking next time
				result = curr;
				freeParticleIndex = (i + 1) % maxParticles;
			}
		}
		while(!result && i != freeParticleIndex);
	}

	return result;
}

int ParticleManager::getNumActiveParticles()
{
	return numInActivePool;
}
