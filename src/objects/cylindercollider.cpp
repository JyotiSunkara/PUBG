#include "objects/cylindercollider.h"

#include "glm/glm.hpp"
using namespace glm;

#include <iostream>
using namespace std;

CylinderCollider::CylinderCollider(vec3 pos, float radius, float height)
{
	this -> pos = pos;
	this -> radius = radius;
	radiusSquared = radius * radius;
	halfHeight = height / 2.0f;
}

CylinderCollider::~CylinderCollider() { }

bool CylinderCollider::testSlidingCollision(vec3 point, glm::vec3 *newPoint, float *distanceResult)
{
	vec3 offset;			// vector from point to center of cylinder
	float dist;				// distance from point to center of cylinder
	bool result = false;	// has a collision occurred?

	// compute offset and distance squared
	offset = vec3(point.x, 0.0, point.z) - vec3(pos.x, 0.0, pos.z);
	dist = (offset.x * offset.x) + (offset.z * offset.z);

	// if we get close enough to collide with this cylinder
	if(dist < radiusSquared && point.y >= pos.y - halfHeight && point.y <= pos.y + halfHeight)
	{
		// record the collision
		result = true;

		// back us away from the cylinder along the same vector we collided with it
		dist = sqrt(dist);
		offset /= dist;
		*newPoint = point + offset * (radius - dist);
		*distanceResult = dist;
	}

	// if we didn't get anything, the sqrt() above didn't happen, so do it now so we can return
	// how close we were to colliding
	if(!result)
	{
		*distanceResult = sqrt(dist);
	}

	return result;
}
