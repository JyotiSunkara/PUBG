#include "profiling.h"

#include "GLFW/glfw3.h"

#include <string>
#include <iostream>
using namespace std;

static string profileTask;
static float profileStartTime;

void profileStart(std::string task)
{
	profileStartTime = glfwGetTime();
	profileTask = task;
}

float profileEnd(bool output)
{
	float time = glfwGetTime() - profileStartTime;
	if(output)
	{
		cout << time << "s for task " << profileTask << endl;
	}
	return time;
}
