#include "world/terrain.h"
#include "world/world.h"

#include "util/shader.h"
#include "util/loadtexture.h"

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"
using namespace glm;

#include <iostream>
using namespace std;

Terrain::Terrain(World *world, int width, int length, float squareSize, float *heights)
{
	this -> world = world;

	setupVBOs(width, length, squareSize, heights);
	loadShader();
	loadTextures();
}

Terrain::~Terrain()
{
	delete[] terrainHeights;

	glDeleteBuffers(4, vbos);
	glDeleteVertexArrays(1, &vao);
	delete shader;
}

void Terrain::setupVBOs(int width, int length, float squareSize, float *heights)
{
	const int NUM_VERTICES = width * length;
	const int NUM_INDICES = (width - 1) * (length - 1) * 6;
	const int NUM_TRIANGLES = NUM_INDICES / 3;

	vec3 *vertices = new vec3[NUM_VERTICES];
	vec3 *vertexPtr = vertices;
	GLfloat *terrainHeightPtr;

	vec2 *baseTexCoords = new vec2[NUM_VERTICES];
	vec2 *baseTexCoordPtr = baseTexCoords;

	GLuint *indices = new GLuint[NUM_INDICES];
	GLuint *indexPtr = indices;

	vec3 *normals = new vec3[NUM_VERTICES];
	short *shareCount = new short[NUM_VERTICES];
	vec3 v1, v2, v3;
	vec3 va, vb;
	vec3 normal;

	float average;
	int i, j, x, z;

	// save terrain characteristics; we need the heights for later for figuring out how high off the ground we are
	this -> width = width;
	this -> length = length;
	this -> squareSize = squareSize;
	terrainHeights = new float[width * length];
	memcpy(terrainHeights, heights, sizeof(float) * width * length);
	terrainHeightPtr = terrainHeights;

	// smooth out out terrain a bit since 256 greyscale values not good
	for(i = 0; i < width; i ++)
	{
		for(j = 0; j < width; j ++)
		{
			average = 0.0;
			for(z = -1; z <= 1; z ++)
			{
				for(x = -1; x <= 1; x ++)
				{
					if(i + z > 0 && i + z < width &&
					   j + x > 0 && j + x < width)
					{
						average += heights[((i * width) + j) + ((z * width) + x)];
					}
				}
			}
			*terrainHeightPtr++ = average / 8.0;		// instead of 9, let's do 8 for fun
		}
	}

	// set the vertex positions
	terrainHeightPtr = terrainHeights;
	for(i = 0; i < length; i ++)
	{
		for(j = 0; j < width; j ++)
		{
			*vertexPtr++ = vec3(j * squareSize, (float)*terrainHeightPtr++, -(i * squareSize));
		}
	}

	// the base texture is one that is widely spread over the terrain
	for(i = 0; i < length; i ++)
	{
		for(j = 0; j < width; j ++)
		{
			*baseTexCoordPtr++ = vec2((float)j / ((float)width / 2048.0), (float)i / ((float)length / 2048.0));
		}
	}

	// compute the indices used to render the terrain;
	// each terrain square is made up of 2 triangles, each using 3 of the vertices from above
	for(i = 0; i < length - 1; i ++)
	{
		for(j = 0; j < width - 1; j ++)
		{
			// top left triangle
			*indexPtr++ = (i * width) + j;					// bottom left corner
			*indexPtr++ = ((i + 1) * width) + j;			// top left corner
			*indexPtr++ = ((i + 1) * width) + j + 1;		// top right corner

			// bottom right triangle
			*indexPtr++ = (i * width) + j;					// bottom left corner
			*indexPtr++ = ((i + 1) * width) + j + 1;		// top right corner
			*indexPtr++ = (i * width) + j + 1;				// bottom right corner
		}
	}

	// compute the normals of the terrain now that we have the indices
	memset(shareCount, 0, sizeof(short) * NUM_VERTICES);
	for(i = 0; i < NUM_TRIANGLES; i ++)
	{
		// get the three vertices that make up this face
		v1 = vertices[indices[i * 3]];
		v2 = vertices[indices[(i * 3) + 1]];
		v3 = vertices[indices[(i * 3) + 2]];

		// compute the normal of the face
        va = v2 - v1;
        vb = v3 - v1;
        normal = normalize(cross(va, vb));

        // now assign this normal to the three vertices that make up this face
        for(j = 0; j < 3; j ++)
        {
			normals[indices[(i * 3) + j]] += normal;
			shareCount[indices[(i * 3) + j]]++;
        }
	}

	// the normals for each vertex are accumulated above, so we need to divide these down
	for(i = 0; i < NUM_VERTICES; i ++)
	{
		normals[i] = normalize(normals[i] / (float)shareCount[i]);
	}

	// create our vertex array object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(4, vbos);

	// our terrain vertices are known so we can set those right away
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * NUM_VERTICES, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// we've also computed our base texture coordinates, so fill those in
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * NUM_VERTICES, baseTexCoords, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// our normals are also computed, so pass those in too
	glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * NUM_VERTICES, normals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// our terrain indices define how our terrain should be constructed using the vertices defined above
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * NUM_INDICES, indices, GL_STATIC_DRAW);

	// free up the memory we've used
	delete[] shareCount;
	delete[] normals;
	delete[] indices;
	delete[] baseTexCoords;
	delete[] vertices;
}

void Terrain::loadShader()
{
	const float REGION_1_MIN = -200;
	const float REGION_1_MAX = 50;
	const float REGION_2_MIN = REGION_1_MAX;
	const float REGION_2_MAX = 450;
	const float REGION_3_MIN = REGION_2_MAX;
	const float REGION_3_MAX = 650;

	shader = new Shader("../shaders/terrain.vert", "../shaders/terrain.frag");
	shader -> bindAttrib("a_Vertex", 0);
	shader -> bindAttrib("a_BaseTexCoord", 1);
	shader -> bindAttrib("a_Normal", 2);
	shader -> link();
	shader -> bind();
	shader -> uniform1i("u_Region1Tex", 0);
	shader -> uniform1i("u_Region2Tex", 1);
	shader -> uniform1i("u_Region3Tex", 2);
	shader -> uniform1i("u_ShadowTex", 3);
	shader -> uniform1f("u_Region1Max", REGION_1_MAX);
	shader -> uniform1f("u_Region1Range", REGION_1_MAX - REGION_1_MIN);
	shader -> uniform1f("u_Region2Max", REGION_2_MAX);
	shader -> uniform1f("u_Region2Range", REGION_2_MAX - REGION_2_MIN);
	shader -> uniform1f("u_Region3Max", REGION_3_MAX);
	shader -> uniform1f("u_Region3Range", REGION_3_MAX - REGION_3_MIN);

	shader -> uniformVec3("u_Sun", World::SUN_DIRECTION);
	shader -> unbind();
}

void Terrain::loadTextures()
{
	region1Texture = loadPNG("../png/grass.png");
	region2Texture = loadPNG("../png/rock.png");
	region3Texture = loadPNG("../png/snow.png");
}

void Terrain::render(mat4 &projection, mat4 &model, mat4 &view)
{
	//mat3 normal = inverseTranspose(mat3(model));

	shader -> bind();
	shader -> uniformMatrix4fv("u_Projection", 1, value_ptr(projection));
	shader -> uniformMatrix4fv("u_Model", 1, value_ptr(model));
	shader -> uniformMatrix4fv("u_View", 1, value_ptr(view));
	//shader -> uniformMatrix3fv("u_Normal", 1, value_ptr(normal));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, region1Texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, region2Texture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, region3Texture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, shadowTexture);

	glDisable(GL_BLEND);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, (width - 1) * (length - 1) * 6, GL_UNSIGNED_INT, NULL);
}

float Terrain::getHeight(vec3 pos)
{
	int p0, p1, p2, p3;
	float fracX, fracZ;
	float interp0, interp1;
	float result = 0.0;

	// scale the passed-in position into terrain coordinates
	float scaledX = pos.x / squareSize;
	float scaledZ = -pos.z / squareSize;

	// figure out which tile we're standing on
	int tileX = (int)scaledX;		// truncation
	int tileZ = (int)scaledZ;

	// make sure the computed tile positions are actually on the terrain
	if(tileX >= 0 && tileX < width && tileZ >= 0 && tileZ < width)
	{
		// now compute the indices the corners making up the tile we're standing on
		p0 = (tileZ * width + tileX);
		p1 = ((tileZ * width + tileX) + 1);
		p2 = ((tileZ + 1) * width + tileX);
		p3 = ((tileZ + 1) * width + tileX + 1);

		// figure out how much in each direction we've advanced across this tile
		fracX = scaledX - (float)tileX;
		fracZ = scaledZ - (float)tileZ;

		// now interpolate linearly between the height values
		interp0 = terrainHeights[p0] + (terrainHeights[p1] - terrainHeights[p0]) * fracX;
		interp1 = terrainHeights[p2] + (terrainHeights[p3] - terrainHeights[p2]) * fracX;
		result = interp0 + (interp1 - interp0) * fracZ;
	}

	return result;
}

bool Terrain::raycast(vec3 start, vec3 end, vec3 &intersect)
{
	const float RAYCAST_PRECISION = 0.01;			// 1cm


	vec3 offset = end - start;					// vector from start of ray to the end of the ray
	vec3 forward = normalize(offset);			// direction of the ray
	float rayLength = glm::length(offset);		// length of the ray

	vec3 fineRayEnd = start;					// where to end the second phase of our raycast (the fine-grained one)
	vec3 fineRayStart;							// where to start the second phase of our raycast (the fine-grained one)
	vec3 midway;								// midpoint as computed during our binary search
	float diff;									// height difference between predicted point of intersection and terrain height
	float travelled = 0.0;						// how far we've walked across the whole ray when we cross over the terrain the first time
	bool intersectionFound = false;				// result that we send back

	// do a coarse walk forward to find the first point of intersection, if it exists; this will yield two points that are 1 squareSize
	// unit away from each other, with one point above ground and one below
	while(!intersectionFound && travelled <= rayLength)
	{
		travelled += squareSize;
		fineRayEnd += forward * squareSize;

		// we've crossed over a point of intersection, so let's compute the previous point again so we can do a finer-grained
		// raycast between those two points using a binary search
		if(fineRayEnd.y < getHeight(fineRayEnd))
		{
			fineRayStart = fineRayEnd - forward * squareSize;
			intersectionFound = true;
		}
	}

	// now do a finer-grained raycast with a binary search; we can skip this bit if there's no intersection at all
	if(intersectionFound)
	{
		// we can now do a binary search between the two points we computed above to determine a "close enough" point of intersection
		// against the terrain; even with 1cm precision, we only require a handful of loops here to do this
		intersectionFound = false;
		while(!intersectionFound)
		{
			// find the halfway point between the two test points, and determine how far away from the terrain this is
			midway = fineRayStart + (fineRayEnd - fineRayStart) / 2.0f;
			diff = midway.y - getHeight(midway);

			// standard binary search between the two points we computed above in the first phase of the raycast
			if(diff > RAYCAST_PRECISION)
			{
				fineRayStart = midway;
			}
			else if(diff < -RAYCAST_PRECISION)
			{
				fineRayEnd = midway;
			}
			else
			{
				intersectionFound = true;
				intersect = midway;
			}
		}
	}

	return intersectionFound;
}

void Terrain::setShadowTexture(GLuint shadowTexture)
{
	this -> shadowTexture = shadowTexture;
}
