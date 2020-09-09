#include "objects/complexcollider.h"

#include "glmmodel/glmmodel.h"

#include "claudette/collision_model_3d.h"		// a really nice open-source, minimal collision library based on coldet that
#include "claudette/ray_collision_test.h"		// I stumbled upon rather late in NFZ's development
using namespace Claudette;

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
using namespace glm;

#include <iostream>
using namespace std;

ComplexCollider::ComplexCollider(GLMmodel *geometry)
{
	transform = mat4(1.0);
	setGeometry(geometry);
}

ComplexCollider::~ComplexCollider()
{
	// does claudette provide a way to free the collision geometry data?
}

void ComplexCollider::setTransform(mat4 &transform)
{
	this -> transform = transform;
	geometry -> setTransform(value_ptr(transform));
}

void ComplexCollider::setGeometry(GLMmodel *model)
{
    GLMgroup *groups = model -> groups;					// multi-group meshes not supported (yet)...I'm lazy
    int numTriangles = model -> numtriangles;

    float vertices[3][3];
    int i, j, k;

    // prime our collision object
    geometry = new Claudette::CollisionModel3D();
    geometry -> setTriangleCount(numTriangles);

	// this loops forms one model
    for(i = 0; i < numTriangles; i ++)
    {
		// this loop forms one triangle
        for(j = 0; j < 3; j ++)
        {
			// this loop forms one vertex
			for(k = 0; k < 3; k ++)
			{
				vertices[j][k] = model -> vertices[3 * model -> triangles[(groups[0].triangles[i])].vindices[j] + k];
			}
        }

		geometry -> addTriangle(vertices[0], vertices[1], vertices[2]);
    }

	geometry -> finalize();
}

bool ComplexCollider::collidesWithRay(vec3 &start, vec3 &dir, float length, vec3 &intersect)
{
    RayCollisionTest rayTest;
    vec4 modelCoords;
    bool result;

    // configure our ray to start at the given coordinates, with the given length and direction
    rayTest.setRayOrigin(start.x, start.y, start.z);
    rayTest.setRayDirection(dir.x, dir.y, dir.z);
    rayTest.setRaySegmentBounds(0.0, length);
    rayTest.setRaySearch(RayCollisionTest::SearchClosestTriangle);		// expensive but accurate, since we want
																		// the most realistic collision details

    // now test the geometry against the ray
    result = geometry -> rayCollision(&rayTest);
	if(result)
	{
		// bring the collision point into world coordinates and return it (annoyingly, Claudette
		// doesn't seem to have the option of doing that for us, although coldet did...)
        const float *point = rayTest.point();
		modelCoords = transform * vec4(point[0], point[1], point[2], 1.0);
		intersect = vec3(modelCoords);
	}

	return result;
}
