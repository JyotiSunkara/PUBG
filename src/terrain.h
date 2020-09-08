#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

class World;
class Shader;

class Terrain
{
private:
	World *world;						// handle to world for sun direction

	Shader *shader;						// shader program used when rendering the terrain

	GLuint vao;							// GL state used when rendering the terrain
	GLuint vbos[4];						// GL vertex buffer object for vertex position, tex coords, normals, and element indices

	GLuint region1Texture;				// the terrain uses and blends together several different textures based
	GLuint region2Texture;				// on the terrain height to give a somewhat-convincing illusion of realistic
	GLuint region3Texture;				// grassy hills and mountains
	GLuint shadowTexture;				// this texture is laid on top of the entire terrain object for low-res, static shadows

	int width;							// width of terrain in tiles
	int length;							// length of terrain in tiles
	float squareSize;					// terrain XZ tile size
	float *terrainHeights;				// array of terrain heights

	// load up our resources, pretty self-explanatory
	void setupVBOs(int width, int length, float squareSize, float *heights);
	void loadShader();
	void loadTextures();

public:
	Terrain(World *world, int width, int length, float squareSize, float *heights);
	~Terrain();

	// render the entire terrain in one fell swoop
	void render(glm::mat4 &projection, glm::mat4 &view, glm::mat4 &model);

	// interaction with terrain
	float getHeight(glm::vec3 pos);
	bool raycast(glm::vec3 start, glm::vec3 end, glm::vec3 &intersect);

	// handle terrain shadow texture, which is built externally by the World object
	void setShadowTexture(GLuint shadowTexture);
};
