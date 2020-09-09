#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

class Shader;

class Sky
{
private:
	Shader *shader;					// used when rendering skydome

	GLuint vao;						// GL rendering state used
	GLuint vbos[3];					// GL vertex buffer object for vertex positions, tex coords, and normals

	GLuint skyTexture;				// large sky image used for the dome

	int numVertices;				// GL rendering calls require the vertex count

	// load up required resources
	void loadSkyModel();
	void loadTextures();
	void loadShader();

public:
	Sky();
	~Sky();

	void render(glm::mat4 &projection, glm::mat4 &view, glm::vec3 &playerPos);
};
