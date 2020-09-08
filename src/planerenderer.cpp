#include "planerenderer.h"
#include "shader.h"

#include "GL/glew.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

#include <iostream>
using namespace std;

const int PlaneRenderer::NUM_VERTICES = 4;

PlaneRenderer *PlaneRenderer::instance = NULL;

PlaneRenderer::PlaneRenderer()
{
	setupVBOs();
	loadShader();
}

PlaneRenderer::~PlaneRenderer()
{
	glDeleteBuffers(1, vbos);
	glDeleteVertexArrays(1, &vao);
	delete shader;
	instance = NULL;
}

PlaneRenderer *PlaneRenderer::getInstance()
{
	if(instance == NULL)
	{
		instance = new PlaneRenderer();
	}

	return instance;
}

void PlaneRenderer::setupVBOs()
{
	const vec2 VERTICES[] = {vec2(-0.5, -0.5),
							 vec2(-0.5, 0.5),
							 vec2(0.5, -0.5),
							 vec2(0.5, 0.5)};
	const vec2 TEX_COORDS[] = {vec2(0.0, 0.0),
							   vec2(0.0, 1.0),
							   vec2(1.0, 0.0),
							   vec2(1.0, 1.0)};

	// generate our vertex array object and buffers
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(2, vbos);

	// throw the vertex data onto the GPU
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * NUM_VERTICES, VERTICES, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// throw the tex coord data onto the GPU
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * NUM_VERTICES, TEX_COORDS, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
}

void PlaneRenderer::loadShader()
{
	shader = new Shader("../shaders/plane.vert", "../shaders/plane.frag");
	shader -> bindAttrib("a_Vertex", 0);
	shader -> bindAttrib("a_TexCoord", 1);
	shader -> link();
	shader -> bind();
	shader -> uniform1i("u_Texture", 0);
	shader -> unbind();
}

void PlaneRenderer::bindShader()
{
	shader -> bind();
}

void PlaneRenderer::setProjectionMatrix(mat4 &projectionMatrix)
{
	shader -> uniformMatrix4fv("u_Projection", 1, value_ptr(projectionMatrix));
}

void PlaneRenderer::setViewMatrix(mat4 &viewMatrix)
{
	shader -> uniformMatrix4fv("u_View", 1, value_ptr(viewMatrix));
}

void PlaneRenderer::setModelMatrix(mat4 &modelMatrix)
{
	shader -> uniformMatrix4fv("u_Model", 1, value_ptr(modelMatrix));
}

void PlaneRenderer::setColor(vec4 &color)
{
	shader -> uniformVec4("u_Color", color);
}

void PlaneRenderer::render()
{
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, NUM_VERTICES);
}
