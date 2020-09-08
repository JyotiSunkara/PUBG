#include "aabbcollider.h"

#include "glm/glm.hpp"
using namespace glm;

AABBCollider::AABBCollider(vec3 pos, vec3 size)
{
	vec3 halfSize = size / 2.0f;

	this -> pos = pos;
	minBounds = pos - halfSize;
	maxBounds = pos + halfSize;
}

AABBCollider::~AABBCollider() { }

bool AABBCollider::testSlidingCollision(vec3 point, glm::vec3 *newPoint)
{
	vec3 depth;				// how far into each dimension we've penetrated
	bool result = false;	// has a collision occurred?

	// determine if we're in the AABB at all
	if(point.x > minBounds.x && point.x < maxBounds.x &&
	   point.y > minBounds.y && point.y < maxBounds.y &&
	   point.z > minBounds.z && point.z < maxBounds.z)
	{
		// record a collision
		result = true;

		// get the penetration depth along each axis we care about
		depth = glm::min(abs(point - minBounds), abs(point - maxBounds));

		// only move away the minimum amount necessary to avoid a collision
		if(depth.x < depth.y && depth.x < depth.z)
		{
			// x penetration is the smallest
			if(point.x > pos.x) point.x += depth.x;
			else if(point.x < pos.x) point.x -= depth.x;
		}
		else if(depth.y < depth.x && depth.y < depth.z)
		{
			// y penetration is the smallest
			if(point.y > pos.y) point.y += depth.y;
			else if(point.y < pos.y) point.y -= depth.y;
		}
		else
		{
			// z penetration is the smallest
			if(point.z > pos.z) point.z += depth.z;
			else if(point.z < pos.z) point.z -= depth.z;
		}

		// set the new point, if the collision occurred; this will slide the
		// point along the AABB the shortest distance so as to avoid a collision
		*newPoint = point;
	}

	return result;
}
