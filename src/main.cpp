#undef GLFW_DLL
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
using namespace std;

int main() {
  if (!glfwInit()) {
    cerr << "Can't initialize GLFW" << endl;
    exit (EXIT_FAILURE);
  }

  GLFWwindow *win;
  win = glfwCreateWindow (450, 300, "Hello", NULL, NULL);
  glfwMakeContextCurrent(win);
  while (!glfwWindowShouldClose(win)) {
    glfwWaitEvents();
  }
  glfwDestroyWindow(win);
  glfwTerminate();
  return 0;
}
