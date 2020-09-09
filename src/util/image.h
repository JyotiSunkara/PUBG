#pragma once

#include "GL/glew.h"

#include <inttypes.h>

// this is a useful class for handling simple shadow textures; ideally, we should also use this class to load images
// from file, too, but currently that's not implemented and we use the functions in loadtexture.h/.cpp for this
class Image
{
private:
	unsigned int width;				// width of image in pixels
	unsigned int height;			// height of image in pixels
	uint8_t *pixels;				// actual greyscale pixel data (one byte per pixel)

public:
	Image(unsigned int width, unsigned int height);
	~Image();

	// generate an OpenGL texture object from this image
	GLuint makeGLTexture();

	// pixel manip methods
	bool inRange(unsigned int x, unsigned int y);						// is the pixel coordinate within the image dimensions?
	void setPixel(unsigned int x, unsigned int y, uint8_t value);		// set given pixel to a specified value
	uint8_t getPixel(unsigned int x, unsigned int y);					// retrieve value of pixel at specified location

	// sets a given rectangular area of pixels to a specified colour
	void setRectangle(unsigned int x, unsigned int y, unsigned int width, unsigned int height, uint8_t value);

	// sets a given circular area of pixels to a specified colour based on their distance from the center of the circle
	void setCircle(unsigned int x, unsigned int, unsigned int radius, uint8_t value);
};
