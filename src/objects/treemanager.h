#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include <string>

class World;
class Shader;
class Tree;

typedef struct _GLMmodel GLMmodel;

class TreeManager
{
private:
	World *world;							// used for terrain height and sun direction

	Shader *treeShader;						// shader program used when rendering trees

	GLMmodel *collider;						// complex collider geometry for tree

	GLuint vao;								// GL state used when rendering trees
	GLuint vbos[4];							// GL vertex buffer object for vertex coords, tex coords, normals, and instance model matrices

	int numVerticesPerTree;					// required for GL call to render trees

	GLuint diffuseMap;						// diffuse texture
	GLuint normalMap;						// how the texture will be lit

	glm::mat4 *modelMats;					// model matrices of trees, passed to GPU when all trees are added
	glm::mat4 *modelMatPtr;					// used to track current model matrix we're updating when calling addTree()

	bool treePlacementFinalized;			// have we called finalizeTreePlacement()?

	int maxTrees;							// how many trees we're allowed to have (this is a silly limit---see the constructor comments below)
	int numTrees;							// how many trees we actually do have

	// load up our resources, pretty self-explanatory
	void loadTree();
	void loadTextures();

public:
	// technically, in this case, there's no reason we can't just use a std::vector or something rather than specifying a silly
	// limit on the number of trees we're allowed...I may fix this in the future
	TreeManager(World *world, int maxTrees);
	~TreeManager();

	Tree *addTree(glm::vec3 pos);			// we cannot exceed maxTrees when adding trees, otherwise we get in trouble
	void finalizeTreePlacement();			// called to send data to GPU, when we've called addTree() enough and won't need to call it ever again
	void loadShaders();

	// render the trees and their leaves
	void render(glm::mat4 &projection, glm::mat4 &view, glm::mat4 &model);
};
