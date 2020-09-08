#include "loadtexture.h"
#include "profiling.h"

#include "lodepng.h"

#include "GL/glew.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
using namespace std;

GLuint loadPNG(const char *name, bool highQualityMipmaps)
{
    unsigned int width;
    unsigned int height;
    unsigned char *data;
    unsigned char *temp;
    unsigned int i;
    GLuint result = 0;

	// use a simple stand-alone library to load our PNGs
	lodepng_decode32_file(&data, &width, &height, name);

	// make sure the load was successful
	if(data)
	{
		// the pixel data is flipped vertically, so we need to flip it back with an in-place reversal
		temp = new unsigned char[width * 4];
		for(i = 0; i < height / 2; i ++)
		{
			memcpy(temp, &data[i * width * 4], (width * 4));								// copy row into temp array
			memcpy(&data[i * width * 4], &data[(height - i - 1) * width * 4], (width * 4));	// copy other side of array into this row
			memcpy(&data[(height - i - 1) * width * 4], temp, (width * 4));					// copy temp into other side of array
		}

		// we can generate a texture object since we had a successful load
		glGenTextures(1, &result);
		glBindTexture(GL_TEXTURE_2D, result);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		// texture UVs should not clamp
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// generate high-quality mipmaps for this texture?
		if(highQualityMipmaps)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
		else
		{
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}

		// release the memory used to perform the loading
		delete[] data;
		delete[] temp;
	}
	else
	{
		cerr << "loadTexture() could not load " << name << endl;
		exit(1);
	}

    return result;
}
