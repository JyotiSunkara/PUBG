#pragma once

#include "GL/glew.h"

// returns a GL texture ID associated with the data in the named image file;
// NOTE: I've removed SOIL as a dependency and use LodePNG instead, so this
// function can only load PNGs now
GLuint loadPNG(const char *name, bool highQualityMipmaps = true);
