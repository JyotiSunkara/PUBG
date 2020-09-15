#pragma once

#include <string>

#include "GL/glew.h"
#include "glm/glm.hpp"

// Shader wrapper class 
class Shader
{
private:
	GLuint fragmentShader;						// ID for fragment/pixel shader
	GLuint vertexShader;						// ID for vertex shader
	GLuint program;								// ID for entire damn thing

	GLint getUniLoc(GLuint, const char*);		// used internally when setting uniform variables
	void printShaderLogInfo(GLuint);			// used for error reporting/compiler errors
	char *textFileRead(const char*);			// used to load shader source from file

public:
	static void unbind();										// unattach shader from OpenGL context

	Shader(std::string vertexFile, std::string fragmentFile);	// specify vertex and fragment shaders separately and compile them together
	~Shader();

	void bind();								// used to bring shader into current GL context (to render with or specify uniform vars)
	void link();								// called only once, after attributions have been bound via bindAttrib()

	void bindAttrib(const char*, unsigned int);	// specify the locations of the named vertex attributes

	// various methods for setting uniform variables
	void uniform1i(const char*, int);
	void uniform1f(const char*, float);
	void uniform1fv(const char*, int, float*);
	void uniform2f(const char*, float, float);
	void uniform2fv(const char*, int, float*);
	void uniformVec2(const char*, glm::vec2);
	void uniform3iv(const char*, int, int*);
	void uniform3fv(const char*, int, float*);
	void uniform3f(const char*, const float, const float, const float);
	void uniformVec3(const char*, glm::vec3);
	void uniformMatrix3fv(const char*, int, GLfloat*, bool = false);
	void uniform4iv(const char*, int, int*);
	void uniform4fv(const char*, int, float*);
	void uniform4f(const char*, float, float, float, float);
	void uniformVec4(const char*, glm::vec4);
	void uniformMatrix4fv(const char*, int, GLfloat*, bool = false);
};
