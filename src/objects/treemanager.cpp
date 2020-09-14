#include "objects/treemanager.h"
#include "objects/tree.h"
#include "objects/complexcollider.h"
#include "objects/player.h"
extern bool fogFlag;

#include "world/world.h"

#include "util/loadtexture.h"
#include "util/shader.h"

#include "glmmodel/glmmodel.h"

#include "GL/glew.h"

#include "glm/glm.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"
using namespace glm;

#include <iostream>
#include <string>
using namespace std;

TreeManager::TreeManager(World *world, int maxTrees)
{
	this -> world = world;
	this -> maxTrees = maxTrees;
	numTrees = 0;
	treePlacementFinalized = false;
	numVerticesPerTree = 0;

	modelMats = new mat4[maxTrees];
	modelMatPtr = modelMats;

    loadTree();
    loadTextures();
    loadShaders();
}

TreeManager::~TreeManager()
{
	delete[] modelMats;

	glDeleteBuffers(4, vbos);
	glDeleteVertexArrays(1, &vao);
	delete treeShader;
}

void TreeManager::loadTree()
{
	GLMmodel *geometry;

    // attempt to read the file; glmReadObj() will just quit if we can't
    geometry = glmReadOBJ((char*)"../mesh/tree.obj");
    if(geometry)
    {
		glmScale(geometry, 1.0);

		// build our buffer objects and then fill them with the geometry data we loaded
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(4, vbos);
		glmBuildVBO(geometry, &numVerticesPerTree, &vao, vbos);

		// set aside some memory for our tree modelviews (i.e., prepare for instanced rendering)
		glBindBuffer(GL_ARRAY_BUFFER, vbos[3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * maxTrees, NULL, GL_STATIC_DRAW);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)0);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)sizeof(vec4));
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)(sizeof(vec4) * 2));
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)(sizeof(vec4) * 3));
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
    }

	// attempt to read the collision geometry; glmReadObj() will just quit if we can't
	collider = glmReadOBJ((char*)"../mesh/tree-collider.obj");
	if(collider)
	{
		glmScale(collider, 1.0);
	}
}

void TreeManager::loadTextures()
{
	diffuseMap = loadPNG("../png/tree-diffuse-map.png");
	normalMap = loadPNG("../png/tree-normal-map.png");
}

void TreeManager::loadShaders()
{
	treeShader = new Shader("../shaders/tree.vert", "../shaders/tree.frag");
	treeShader -> bindAttrib("a_Vertex", 0);
	treeShader -> bindAttrib("a_Normal", 1);
	treeShader -> bindAttrib("a_TexCoord", 2);
	treeShader -> bindAttrib("a_InstanceMatrix", 3);
	treeShader -> link();
	treeShader -> bind();
	treeShader -> uniform1i("u_DiffuseMap", 0);
	treeShader -> uniform1i("u_NormalMap", 1);
	treeShader -> uniform1f("u_NormalMapStrength", 2.0);
	treeShader -> uniformVec3("u_Sun", World::SUN_DIRECTION);
	treeShader -> uniformVec3("fogColor", vec3(0.5, 0.5, 0.5));
	treeShader -> uniform1i("fogFlag", fogFlag);
	treeShader -> unbind();
}

Tree *TreeManager::addTree(vec3 pos)
{
	const float MIN_TREE_SIZE = 15.0;
	const float MAX_TREE_SIZE = 30.0;

	Tree *result;

	mat4 modelMat;

	// disallow tree adding if we've already thrown everything on the GPU
	if(!treePlacementFinalized)
	{
		// make sure we don't put more trees in than we're allowed
		if(numTrees < maxTrees)
		{
			// compute how high the tree would be on the terrain
			pos.y = world -> getTerrainHeight(pos);

			// now compute a model matrix for the tree with a random rotation and size
			modelMat = mat4(1.0);
			modelMat = translate(modelMat, pos);
			//modelMat = rotate(modelMat, (float)linearRand(-M_PI, M_PI), vec3(0.0, 1.0, 0.0));
			modelMat = scale(modelMat, vec3(linearRand(MIN_TREE_SIZE, MAX_TREE_SIZE)));

			// set the position and orientation of the tree and track the number of trees we have
			*modelMatPtr++ = modelMat;

			// assign a collision object and assign it's model matrix
			result = new Tree();
			result -> setComplexCollider(new ComplexCollider(collider));
			result -> setModelMat(modelMat);
			result -> updateComplexCollider();

			// add one to the tree count!
			numTrees ++;
		}
		else
		{
			cerr << "TreeManager::addTree() cannot add a tree because the maximum of " << maxTrees << " is reached" << endl;
			exit(1);
		}
	}
	else
	{
		cerr << "TreeManager::addTree() cannot add a tree because finalizeTreePlacement() has already been called" << endl;
		exit(1);
	}

	return result;
}

void TreeManager::finalizeTreePlacement()
{
	// we need this method because we would prefer to send all of the tree positions to the GPU all at once,
	// rather than individually...this is much faster
	if(!treePlacementFinalized)
	{
		// store the tree positions in the GPU
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbos[3]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(mat4) * numTrees, modelMats);

		// prevent any trees from being added after this point forward
		treePlacementFinalized = true;
	}
	else
	{
		cerr << "TreeManager::finalizeTreePlacement() has already been called" << endl;
		exit(1);
	}
}

void TreeManager::render(mat4 &projection, mat4 &view, mat4 &model)
{
	// importantly, the trees suffer from the same lighting problem as the drones do;
	// (see dronemananger.cpp for more details) to get around this, we just don't
	// rotate the trees at all; a full solution will use the mat3 inverse transpose
	// of the instance matrices and pass those in as well...I'll probably get to that
	// another time when I feel like it
	if(treePlacementFinalized && numTrees > 0)
	{
		treeShader -> bind();
		treeShader -> uniformMatrix4fv("u_Projection", 1, value_ptr(projection));
		treeShader -> uniformMatrix4fv("u_View", 1, value_ptr(view));

		// render the trees

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMap);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glBindVertexArray(vao);
		glDrawArraysInstanced(GL_TRIANGLES, 0, numVerticesPerTree, numTrees);
	}
}
