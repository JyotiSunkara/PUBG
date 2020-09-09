#include "util/math.h"

#include "glm/glm.hpp"
using namespace glm;

#include <iostream>
using namespace std;

// Fast Point-In-Cylinder Test by Greg James
// http://www.flipcode.com/archives/Fast_Point-In-Cylinder_Test.shtml
bool isPointInCylinder(const vec3 &p1, const vec3 & p2, double length_sq, double radius_sq, const vec3 &point)
{
	double dx, dy, dz;			// vector d  from line segment point 1 to point 2
	double pdx, pdy, pdz;		// vector pd from point 1 to test point
	double dot, dsq;

	dx = p2.x - p1.x;			// translate so pt1 is origin.  Make vector from
	dy = p2.y - p1.y;     		// pt1 to pt2.  Need for this is easily eliminated
	dz = p2.z - p1.z;

	pdx = point.x - p1.x;		// vector from pt1 to test point.
	pdy = point.y - p1.y;
	pdz = point.z - p1.z;

	// Dot the d and pd vectors to see if point lies behind the
	// cylinder cap at pt1.x, pt1.y, pt1.z

	dot = pdx * dx + pdy * dy + pdz * dz;

	// If dot is less than zero the point is behind the pt1 cap.
	// If greater than the cylinder axis line segment length squared
	// then the point is outside the other end cap at pt2.

	if( dot < 0.0 || dot > length_sq )
	{
		return false;
	}
	else
	{
		// Point lies within the parallel caps, so find
		// distance squared from point to line, using the fact that sin^2 + cos^2 = 1
		// the dot = cos() * |d||pd|, and cross*cross = sin^2 * |d|^2 * |pd|^2
		// Carefull: '*' means mult for scalars and dotproduct for vectors
		// In short, where dist is pt distance to cyl axis:
		// dist = sin( pd to d ) * |pd|
		// distsq = dsq = (1 - cos^2( pd to d)) * |pd|^2
		// dsq = ( 1 - (pd * d)^2 / (|pd|^2 * |d|^2) ) * |pd|^2
		// dsq = pd * pd - dot * dot / length_sq
		//  where length_sq is d*d or |d|^2 that is passed into this function

		// distance squared to the cylinder axis:

		dsq = (pdx*pdx + pdy*pdy + pdz*pdz) - dot*dot/length_sq;

		return dsq <= radius_sq;
	}
}

// continuous noise function, as a faster alternative to Perlin noise
// http://stackoverflow.com/questions/16569660/2d-perlin-noise-in-c
float fakePerlinNoise(int x, int y)
{
    int n;

    n = x + y * 57;
    n = (n << 13) ^ n;
    return (1.0 - ( (n * ((n * n * 15731) + 789221) +  1376312589) & 0x7fffffff) / 1073741824.0);
}
