#pragma once

#include "glm/glm.hpp"

// a simple upwards-facing cylinder implementation for handling sliding collision
// against other points
class CylinderCollider
{
public:
	CylinderCollider(glm::vec3 pos, float radius, float height);
	~CylinderCollider();

	bool testSlidingCollision(glm::vec3 point, glm::vec3 *newPoint, float *closestDist);

private:
	glm::vec3 pos;					// global 3D position (center of cylinder)
	float radius;					// radius of cylinder
	float radiusSquared;			// okay, I shouldn't have to explain these ones
	float halfHeight;
};
