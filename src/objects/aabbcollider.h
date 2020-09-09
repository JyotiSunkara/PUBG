#pragma once

#include "glm/glm.hpp"

// a simple axis-aligned bounding box implementation for handling sliding collision
// against other points
class AABBCollider
{
public:
	AABBCollider(glm::vec3 pos, glm::vec3 size);
	~AABBCollider();

	// returns true iff a collision occurs, and also returns where the tested point
	// should be moved to if it did
	bool testSlidingCollision(glm::vec3 point, glm::vec3 *newPoint);

private:
	glm::vec3 pos;					// global 3D center of AABB
	glm::vec3 minBounds;			// minimum bounds in global 3D space
	glm::vec3 maxBounds;			// maximum bounds in global 3D space
};
