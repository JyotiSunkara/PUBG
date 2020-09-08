#include "image.h"

#include "GL/glew.h"

#include "glm/glm.hpp"

#include <stdint.h>
#include <cstring>
#include <iostream>
using namespace std;

Image::Image(unsigned int width, unsigned int height)
{
	this -> width = width;
	this -> height = height;
	pixels = new uint8_t[width * height];
	memset(pixels, 0, width * height);
}

Image::~Image()
{
	delete[] pixels;
}

GLuint Image::makeGLTexture()
{
	// generate and bind our texture we want to work with
	GLuint result;
	glGenTextures(1, &result);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, result);

	// fill our OpenGL texture data with the image we just created
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);

	// set texture wrapping params
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// generate mipmaps; this must be specified in order to have a complete texture object
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return result;
}

bool Image::inRange(unsigned int x, unsigned int y)
{
	return x < width && y < height;
}

void Image::setPixel(unsigned int x, unsigned int y, uint8_t value)
{
	pixels[y * width + x] = value;
}

uint8_t Image::getPixel(unsigned int x, unsigned int y)
{
	return pixels[y * width + x];
}

void Image::setRectangle(unsigned int x, unsigned int y, unsigned int width, unsigned int height, uint8_t value)
{
	unsigned int i, j;
	for(i = y; i <= y + height; i ++)
	{
		for(j = x; j <= x + width; j ++)
		{
			if(inRange(j, i))
			{
				setPixel(j, i, value);
			}
		}
	}
}

void Image::setCircle(unsigned int x, unsigned int y, unsigned int radius, uint8_t value)
{
	unsigned int startX = x - radius;
	unsigned int endX = x + radius;
	unsigned int startY = y - radius;
	unsigned int endY = y + radius;
	float dist;
	unsigned int i, j;

	for(i = startY; i <= endY; i ++)
	{
		for(j = startX; j <= endX; j ++)
		{
			if(inRange(j, i))
			{
				dist = sqrt((float)((x - j) * (x - j) + (y - i) * (y - i)));
				if(dist < radius)
				{
					setPixel(j, i, value * (1.0 - (dist / radius)));
				}
			}
		}
	}
}
