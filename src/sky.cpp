#include "sky.h"

#include "loadtexture.h"
#include "shader.h"

#include "glmmodel.h"

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

Sky::Sky()
{
	loadSkyModel();
	loadTextures();
	loadShader();
}

Sky::~Sky()
{
	glDeleteBuffers(3, vbos);
	glDeleteVertexArrays(1, &vao);
	delete shader;
}

void Sky::loadSkyModel()
{
	GLMmodel *geometry;

    // attempt to read the file; glmReadObj() will just quit if we can't
    geometry = glmReadOBJ((char*)"../mesh/skydome.obj");
    if(geometry)
    {
		glmScale(geometry, 1.0);

		// build our buffer objects and then fill them with the geometry data we loaded
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(3, vbos);
		glmBuildVBO(geometry, &numVertices, &vao, vbos);
	}
}

void Sky::loadTextures()
{
	skyTexture = loadPNG("../png/sky.png");
}

void Sky::loadShader()
{
	shader = new Shader("../shaders/sky.vert", "../shaders/sky.frag");
	shader -> bindAttrib("a_Vertex", 0);
	shader -> bindAttrib("a_Normal", 1);
	shader -> bindAttrib("a_TexCoord", 2);
	shader -> link();
	shader -> bind();
	shader -> unbind();
}

void Sky::render(mat4 &projection, mat4 &view, vec3 &playerPos)
{
	mat4 modelMat(1.0);
	modelMat = translate(modelMat, vec3(playerPos.x, -600.0, playerPos.z));
	modelMat = scale(modelMat, vec3(6000.0));

	// send in the cavalry
	shader -> bind();
	shader -> uniformMatrix4fv("u_Projection", 1, value_ptr(projection));
	shader -> uniformMatrix4fv("u_View", 1, value_ptr(view));
	shader -> uniformMatrix4fv("u_Model", 1, value_ptr(modelMat));

	// bind the sky texture to the current context
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, skyTexture);

	// we don't want the sky transparent and we don't want it doing depth writes
	glDisable(GL_BLEND);
	glDepthMask(GL_FALSE);

	// finally, render it
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, numVertices);

	glDepthMask(GL_TRUE);
}
