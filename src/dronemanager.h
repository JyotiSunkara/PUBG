#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include "soundmanager.h"

typedef struct _GLMmodel GLMmodel;

class World;
class Shader;
class Drone;

class DroneManager
{
private:
	World *world;						// handle to world for collision checks and anything else we need
	Shader *bodyShader;					// shader program used when rendering drone body
	Shader *bladesShader;				// shader program used when rendering drone blades

	GLMmodel *droneBodyModel;			// geometry for body
	GLMmodel *droneBladeModel;			// geometry for blades (just four quads at each rotor with a blurred blade texture)
	GLMmodel *droneColliderModel;		// collision geometry for the entire drone

	GLuint bodyVAO;						// GL state for rendering body
	GLuint bodyVBOs[4];					// GL vertex buffer objects for vertex position, tex coords, normals, and instance model matrices
	int numBodyVertices;				// required for GL rendering call

	GLuint bladesVAO;					// GL state for rendering blades
	GLuint bladesVBOs[4];				// GL vertex buffer objects for vertex position, tex coords, normals, and instance model matrices
	int numBladesVertices;				// required for GL rendering call

	GLuint diffuseMap;					// body diffuse texture
	GLuint normalMap;					// body normal map texture
	GLuint specularMap;					// body specular map texture for dust and dirt
	GLuint emissionMap;					// body emission map for any glowing lights, etc.
	GLuint bladesTexture;				// only one texture is needed for the blades

	ALuint hoverSound;					// AL buffer object for the hover sound
	ALuint warningSound;				// AL buffer object for the warning buzz
	ALuint explodeSound;				// AL buffer object for the explosion sound

	glm::mat4 *modelMats;				// list of instance model matrices

	int maxDrones;						// we must know the max number of drones in advance so we can work more efficiently with OpenGL
	int numDrones;						// number of drones actually present in the game (including ones that have been killed)
	int numDronesAlive;					// how many drones are still alive

	Drone *drones;						// array of drone objects
	float *cylinderTestTimers;			// used for temporal partitioning when doing collision checks against cylinders

	void initTemporalPartitioning();	// used to time collision checks in very large environments with few objects

    void loadModels();					// load resources, self-explanatory
    void loadTextures();
    void loadShader();
    void loadSounds();

    // batch-rendering the entire group of drones requires only two rendering calls: one for the body, one for the blades
    void renderBodies(glm::mat4 &projection, glm::mat4 &view, glm::mat3 &normal);
    void renderBlades(glm::mat4 &projection, glm::mat4 &view, glm::mat3 &normal);

public:
	DroneManager(World *world, int maxDrones);		// for simplicity's (and OpenGL's) sake, we must know the max number of drones
	~DroneManager();

	// insert a drone into the system
	Drone *addDrone(glm::vec3 pos);

	// update all drones
	void update(float dt);

	// render all drones
	void render(glm::mat4 &projection, glm::mat4 &view);

	// true iff a drone is within the specified distance of the given point; used for player collision
	bool isDroneCloseTo(glm::vec3 pos, float distance);
};
