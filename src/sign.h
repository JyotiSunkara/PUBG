#pragma once

#include "object.h"

#include "GL/glew.h"
#include "glm/glm.hpp"

class World;
class Shader;

typedef struct _GLMmodel GLMmodel;

class Sign : public Object
{
private:
	World *world;					// handle to world for terrain height

	Shader *shader;					// shader program used to render the sign

	GLMmodel *collider;				// the collision geometry used by the sign to intercept bullet hits

	GLuint vao;						// OpenGL rendering state used when rendering this object
	GLuint vbos[3];					// vertex buffer objects referencing vertex positions, tex coords, and normals

	int numVertices;				// required for GL render call
	GLuint diffuseMap;				// plain sign texture
	GLuint normalMap;				// normal map for per-fragment lighting
	GLuint specularMap;				// used to simulate the presence of dust or dirt
	GLuint emissionMap;				// we use a black texture, since no part of the sign emits light

	// setup the resources needed to render the sign
	void setupModelMatrix(glm::vec3 pos, float angle);
	void loadMesh();
	void loadTextures();
	void loadShader();

public:
	Sign(World *world, glm::vec3 pos, float angle);
	~Sign();

	void render(glm::mat4 &projection, glm::mat4 &view);
};
