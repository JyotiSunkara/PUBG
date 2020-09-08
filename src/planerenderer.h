#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

class Shader;

class PlaneRenderer
{
private:
	static const int NUM_VERTICES;				// must be known to make GL render calls

	static PlaneRenderer *instance;				// our singleton instance

	Shader *shader;								// shader program used to render the plane

	GLuint vao;									// GL render state
	GLuint vbos[2];								// GL vertex buffer objects for vertex position and tex coords

	PlaneRenderer();							// force use of getInstance()

	// configure our geometry and setup our shader program
	void setupVBOs();
	void loadShader();

public:
	static PlaneRenderer *getInstance();		// singleton instance

	~PlaneRenderer();

	// since we'll probably be drawing multiples of certain objects using the PlaneRenderer, we can get extra
	// performance if we eliminate redundant commands to the GPU by only calling these methods below when we
	// really need to...instanced rendering would be more efficient but for something small-scale like this,
	// it's not worth the effort and should be fast enough anyways
	void bindShader();												// should be called first to bind the PlaneRenderer shader
	void setProjectionMatrix(glm::mat4 &projectionMatrix);			// should only be called once, when the ortho projection is computed
	void setViewMatrix(glm::mat4 &viewMatrix);						// should only be called once, since the ortho view matrix doesn't change
	void setModelMatrix(glm::mat4 &modelMatrix);					// should be called multiple times, i.e., once per object we want to render
	void setColor(glm::vec4 &color);								// can be called whenever, depending on the colours of the objects we draw
	void render();													// called once per object we want to render
};
