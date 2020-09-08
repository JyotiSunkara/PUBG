#pragma once

#include "glm/glm.hpp"

class ComplexCollider;

class Object
{
private:
	glm::vec3 pos;							// position of object
	glm::vec3 forward;						// forward vector of object
	glm::vec3 side;							// sideways vector of object
	glm::vec3 up;							// upwards vector of object
	glm::mat4 modelMat;						// model matrix of object (not including scale) made up of the above 4 vectors

	ComplexCollider *collider;				// optional complex collision geometry

	bool garbage;							// can the object be removed from play?

public:
    Object();
    virtual ~Object() = 0;									// must create derived classes

    void setPos(glm::vec3 pos);								// 3D position (this and the other 3 vectors form the object's
    glm::vec3 getPos();										// model matrix, which is convenient to separate out this way)

    void setForward(glm::vec3 forward);						// front forward facing vector
    glm::vec3 getForward();

    void setSide(glm::vec3 side);							// sideways vector
    glm::vec3 getSide();

    void setUp(glm::vec3 up);								// upwards vector
    glm::vec3 getUp();

    bool flaggedAsGarbage();								// is the object marked for deletion?
    void flagAsGarbage();									// mark the object for deletion from game-related processing

    void setModelMat(glm::mat4 &mat);						// manually sets the object's model matrix
    void computeModelMat();									// computes the modelview matrix based on the values of the pos, forward, side, and up vectors
    void getModelMat(glm::mat4 *mat);						// returns a copy of this object's currently computed model matrix (does not call computeModelMat())

    void setComplexCollider(ComplexCollider *collider);		// sets the collider geometry this object will use for collision testing
    void updateComplexCollider();							// updates the transform of the collision geometry to the value most recently computed by computeModelMat()

    // determine if this object has been hit by a ray, and where
	bool collidesWithRay(glm::vec3 &start, glm::vec3 &dir, float length, glm::vec3 &intersect);

    // handle a collision dealt by a ray which impacts at the given point from the given direction
    virtual void handleRayCollision(glm::vec3 dir, glm::vec3 point);
};
