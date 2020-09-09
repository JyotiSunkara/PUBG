#pragma once

#include <string>

// simple functions for timing how long tasks take; assumes GLFW is available and an OpenGL window is active
void profileStart(std::string task);
float profileEnd(bool output = true);
