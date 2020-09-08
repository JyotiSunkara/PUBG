#include "sign.h"
#include "complexcollider.h"

#include "world.h"

#include "loadtexture.h"
#include "shader.h"

#include "glmmodel.h"

#include "GL/glew.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
using namespace glm;

Sign::Sign(World *world, vec3 pos, float angle)
{
	this -> world = world;

	setupModelMatrix(pos, angle);
	loadMesh();
	loadTextures();
	loadShader();

	// initialize collision object
	setComplexCollider(new ComplexCollider(collider));
	updateComplexCollider();
}

Sign::~Sign()
{
	glDeleteBuffers(3, vbos);
	glDeleteVertexArrays(1, &vao);
	delete shader;
}

void Sign::setupModelMatrix(vec3 pos, float angle)
{
	const float SIGN_SIZE = 5.5;			// pretty big sign

	mat4 modelMatrix(1.0);

	// we need to compute how high off the ground the sign should be
	pos.y = world -> getTerrainHeight(pos);

	// we can build our model matrix ahead of time since it never changes
	modelMatrix = translate(modelMatrix, pos);
	modelMatrix = rotate(modelMatrix, angle, vec3(0.0, 1.0, 0.0));
	modelMatrix = scale(modelMatrix, vec3(SIGN_SIZE));
	setModelMat(modelMatrix);
}

void Sign::loadMesh()
{
	GLMmodel *geometry;

    // attempt to read the file; glmReadObj() will just quit if we can't
    geometry = glmReadOBJ((char*)"../mesh/sign.obj");
    if(geometry)
    {
		glmScale(geometry, 1.0);

		// build our buffer objects and then fill them with the geometry data we loaded
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(3, vbos);
		glmBuildVBO(geometry, &numVertices, &vao, vbos);
	}

	// attempt to read the collision geometry; glmReadObj() will just quit if we can't
	collider = glmReadOBJ((char*)"../mesh/sign-collider.obj");
	if(collider)
	{
		glmScale(collider, 1.0);
	}
}

void Sign::loadTextures()
{
	diffuseMap = loadPNG("../png/sign-diffuse-map.png");
	normalMap = loadPNG("../png/sign-normal-map.png");
	specularMap = loadPNG("../png/sign-specular-map.png");
	emissionMap = loadPNG("../png/black.png");
}

void Sign::loadShader()
{
	shader = new Shader("../shaders/solid.vert", "../shaders/solid.frag");
	shader -> bindAttrib("a_Vertex", 0);
	shader -> bindAttrib("a_Normal", 1);
	shader -> bindAttrib("a_TexCoord", 2);
	shader -> link();
	shader -> bind();
	shader -> uniform1i("u_DiffuseMap", 0);
	shader -> uniform1i("u_NormalMap", 1);
	shader -> uniform1i("u_SpecularMap", 2);
	shader -> uniform1i("u_EmissionMap", 3);
	shader -> uniformVec3("u_Sun", World::SUN_DIRECTION);
	shader -> uniformVec3("u_MaterialDiffuse", vec3(1.0, 1.0, 1.0));
	shader -> uniformVec3("u_MaterialSpecular", vec3(1.0, 1.0, 1.0));
	shader -> uniform1f("u_SpecularIntensity", 6.0);
	shader -> uniform1f("u_SpecularHardness", 16.0);
	shader -> uniform1f("u_NormalMapStrength", 1.5);
	shader -> unbind();
}

void Sign::render(mat4 &projection, mat4 &view)
{
	mat4 modelMatrix;
	mat3 normal;

	// compute our normal matrix for lighting
	getModelMat(&modelMatrix);
	normal = inverseTranspose(mat3(modelMatrix));

	// send in the cavalry
	shader -> bind();
	shader -> uniformMatrix4fv("u_Projection", 1, value_ptr(projection));
	shader -> uniformMatrix4fv("u_View", 1, value_ptr(view));
	shader -> uniformMatrix4fv("u_Model", 1, value_ptr(modelMatrix));
	shader -> uniformMatrix3fv("u_Normal", 1, value_ptr(normal));

	// we use diffuse, normal, specular, and emission texture maps when rendering for a really nice effect
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, specularMap);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, emissionMap);

	// we don't want this transparent
	glDisable(GL_BLEND);

	// finally, render it
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, numVertices);
}
