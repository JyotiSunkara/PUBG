#include "object.h"
#include "complexcollider.h"

#include "glm/glm.hpp"
using namespace glm;

Object::Object()
{
	forward = vec3(0.0, 0.0, 1.0);
	side = vec3(1.0, 0.0, 0.0);
	up = vec3(0.0, 1.0, 0.0);
	computeModelMat();
	collider = NULL;
	garbage = false;
}

Object::~Object()
{
	if(collider != NULL)
	{
		delete collider;
	}
}

void Object::setPos(vec3 pos) { this -> pos = pos; }
vec3 Object::getPos() { return pos; }

void Object::setForward(vec3 forward) { this -> forward = forward; }
vec3 Object::getForward() { return forward; }

void Object::setSide(vec3 side) { this -> side = side; }
vec3 Object::getSide() { return side; }

void Object::setUp(vec3 up) { this -> up = up; }
vec3 Object::getUp() { return up; }

bool Object::flaggedAsGarbage() { return garbage; }
void Object::flagAsGarbage() { garbage = true; }

void Object::setModelMat(mat4 &modelMat)
{
	this -> modelMat = modelMat;
}

void Object::computeModelMat()
{
	// update our model matrix based on our direction vectors
	modelMat[0] = vec4(side, 0.0f);
	modelMat[1] = vec4(up, 0.0f);
	modelMat[2] = vec4(forward, 0.0f);
	modelMat[3] = vec4(pos, 1.0f);
}

void Object::getModelMat(mat4 *modelMat)
{
	*modelMat = this -> modelMat;
}

void Object::setComplexCollider(ComplexCollider *collider)
{
	this -> collider = collider;
}

void Object::updateComplexCollider()
{
	collider -> setTransform(modelMat);
}

bool Object::collidesWithRay(vec3 &start, vec3 &dir, float length, vec3 &intersect)
{
	return collider && collider -> collidesWithRay(start, dir, length, intersect);
}

void Object::handleRayCollision(vec3 dir, vec3 point) { }
