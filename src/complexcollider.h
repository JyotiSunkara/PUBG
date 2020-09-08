#pragma once

#include "collision_model_3d.h"

#include "glm/glm.hpp"

class CollisionModel3D;
typedef struct _GLMmodel GLMmodel;

class ComplexCollider
{
public:
	ComplexCollider(GLMmodel *geometry);				// requires Wavefront .OBJ geometry
	~ComplexCollider();

	void setTransform(glm::mat4 &transform);			// model matrix

	// simple raytest powered by the Claudette library
    bool collidesWithRay(glm::vec3 &start, glm::vec3 &dir, float length, glm::vec3 &intersect);

private:
	glm::mat4 transform;								// model matrix---global model position and orientation
	Claudette::CollisionModel3D *geometry;				// Claudette representation of .OBJ geometry

	void setGeometry(GLMmodel *geometry);				// build Claudette representation
};
