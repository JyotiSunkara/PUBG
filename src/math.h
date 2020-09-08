#pragma once

#include <glm/glm.hpp>

// Fast Point-In-Cylinder Test by Greg James
// http://www.flipcode.com/archives/Fast_Point-In-Cylinder_Test.shtml
bool isPointInCylinder(const glm::vec3 &p1, const glm::vec3 & p2, double length_sq, double radius_sq, const glm::vec3 &point);

// continuous noise function, as a faster alternative to Perlin noise
// http://stackoverflow.com/questions/16569660/2d-perlin-noise-in-c
float fakePerlinNoise(int x, int y);
