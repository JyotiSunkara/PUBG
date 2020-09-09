/*
No-Fly Zone
Geoff Nagy

This is a small first-person shooter where the goal is to stay alive and shoot down quadcopters that have
become sentient and are trying to kill you. I wrote this game to provide a learning experience for anyone
wanting to learn about game development.

The main development philosophy behind this project was to keep everything as simple as possible. To keep
things simple I've intentionally tried to avoid overengineering.

This project uses OpenGL 3.2 Core. This GL version is widely supported by older and more modern hardware
and prevents the use of deprecated functionality. While the source code is heavily commented, it doesn't
spend a lot of time explaining how OpenGL works or the particular facilities I've chosen to use. A good
starting point is Luke Benstead's "Beginning OpenGL Game Programming, 2nd Edition", although it is now
slightly out of date. A few of my GitLab projects go into more detail about things like instanced
rendering, for example, which I make heavy use of in this project.

Cross-platform. Tested in Linux Mint (3.19.0-32-generic) and Windows 10.

See README.md for more information.
*/

#include "world/world.h"

#include "util/gldebugging.h"
#include "util/planerenderer.h"
#include "util/loadtexture.h"

#include "GL/glew.h"

#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"				// for lookAt() and perspective()
#include "glm/gtx/rotate_vector.hpp"
using namespace glm;

#include <ctime>
#include <cstdlib>
#include <iostream>
#include <string>
using namespace std;

// GLFW window and characteristics
GLFWwindow *window;
vec2 windowSize;				// changed by openWindow();

// intialization functions
void openWindow();
void prepareOpenGL();
void showLoadingScreen();

// main function; everything starts here
int main(int args, char *argv[])
{
	const string WORLD_FILE = "../png/world.png";

	// some important objects
	World *world;

	// used to control frame timing
	const double TARGET_FRAME_INTERVAL = 0.016665;		// slightly smaller than 1/60, our desired time (sec.) between frames
	const double FRAME_TIME_ALPHA = 0.25;				// degree to which previous frame time is used for delta time computation

	// these handle our very smooth frame timing
	double dt;											// time since last cycle through loop (*not* time between update/render steps)
	double currentTime;									// current time according to GLFW
	double oldTime;										// what the value of currentTime was on the last cycle through loop
	double frameTime = 0.0;								// delta time of the next frame to render to keep things smooth
	double renderAccum = 0.0;							// accumulated time between update/render steps
	double smoothFrameTime = TARGET_FRAME_INTERVAL;		// weighted average delta time used to smooth out next frame

	// reseed with the clock
	srand(time(NULL));

	// create our OpenGL window and context, and set some rendering options
	openWindow();
	prepareOpenGL();

	// render a quick-and-dirty loading screen while everything is loaded and built;
	// this will be cleared the next time the buffers are swapped
	showLoadingScreen();

	// construct our world using a PNG image that describes how it is built
	world = new World(window, windowSize, WORLD_FILE);

	// prime our time tracking
	currentTime = glfwGetTime();
	oldTime = currentTime;

	// loop until we're done
	while(!glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE) && !world -> isGameDone())
	{
		// compute a time multiplier based on our frame rate
		oldTime = currentTime;
		currentTime = glfwGetTime();
		dt = currentTime - oldTime;

		// track the time that has passed since the last loop cycle
		renderAccum += dt;
		frameTime += dt;

		// is it time to update the scene and render a new frame?
		if(renderAccum >= TARGET_FRAME_INTERVAL)
		{
			// clear our colour and depth buffers
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// reset the timer for our next frame after this one
			renderAccum -= TARGET_FRAME_INTERVAL;

			// use our previous frame time to compute a smoother frame time for this update cycle
			smoothFrameTime = (smoothFrameTime * FRAME_TIME_ALPHA) + (frameTime * (1.0 - FRAME_TIME_ALPHA));
			frameTime = 0.0;

			// update the world state and render everything
			world -> update(smoothFrameTime);
			world -> render();

			// get input events and update our framebuffer
			glfwPollEvents();
			glfwSwapBuffers(window);
		}
	}

	// terminate our world
	delete world;

	// shut down GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	// and we're done
	return 0;
}

void openWindow()
{
	const char *TITLE = "No-Fly Zone";

	int windowWidth;
	int windowHeight;
	GLenum error;

	// we need to intialize GLFW before we create a GLFW window
	if(!glfwInit())
	{
		cerr << "openWindow() could not initialize GLFW" << endl;
		exit(1);
	}

	// explicitly set our OpenGL context to something that doesn't support any old-school shit
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_REFRESH_RATE, 60);
	#ifdef DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	#endif

	// use the current desktop mode to decide on a suitable resolution
	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	windowWidth = mode -> width;
	windowHeight = mode -> height;

	// create our OpenGL window using GLFW
    window = glfwCreateWindow(windowWidth, windowHeight,		// specify width and height
							  TITLE,							// title of window
							  #ifdef DEBUG
								NULL,							// windowed mode
							  #else
								glfwGetPrimaryMonitor(),		// fullscreen mode
							  #endif
							  NULL);							// not sharing resources across monitors
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// disable the cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// configure our viewing area
	glViewport(0, 0, windowWidth, windowHeight);

	// enable our extensions handler
	glewExperimental = true;		// GLEW bug: glewInit() doesn't get all extensions, so we have it explicitly search for everything it can find
	error = glewInit();
	if(error != GLEW_OK)
	{
		cerr << glewGetErrorString(error) << endl;
		exit(1);
	}

	// clear the OpenGL error code that results from initializing GLEW
	glGetError();

	// save our window size for later on
	windowSize = vec2(windowWidth, windowHeight);

	// print our OpenGL version info
    cout << "-- GL version:   " << (char*)glGetString(GL_VERSION) << endl;
    cout << "-- GL vendor:    " << (char*)glGetString(GL_VENDOR) << endl;
    cout << "-- GL renderer:  " << (char*)glGetString(GL_RENDERER) << endl;
	cout << "-- GLSL version: " << (char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	// OpenGL callback debugging is extremely useful, so enable it if we're in debug mode
	#ifdef DEBUG
		initGLDebugger();
	#else
		cout << "-- GL callback debugging is disabled in non-Debug configurations" << endl;
	#endif
}

void prepareOpenGL()
{
	// turn on depth testing and enable blending
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// sky blue-ish in background
	glClearColor(0.529, 0.808, 0.922, 0.0);
}

void showLoadingScreen()
{
	// near and far clip range for ortho perspective; good defaults are -1.0 and 1.0
	const float ORTHO_NEAR_RANGE = -1.0;
	const float ORTHO_FAR_RANGE = 1.0;

	// how we want to the logo to appear
	vec4 LOGO_COLOR(1.0, 1.0, 1.0, 0.1);
	const vec3 LOGO_SIZE(256.0, 256.0, 0.0);
	const vec3 LOGO_POSITION(windowSize.x / 2.0f, windowSize.y / 2.0f, 0.0);

	// how we want the loading message to appear
	vec4 LOADING_COLOR(1.0, 1.0, 1.0, 0.7);
	const vec3 LOADING_SIZE(128.0, 128.0, 0.0);
	const vec2 LOADING_MARGIN(100, 50);
	const vec3 LOADING_POSITION(windowSize.x - LOADING_MARGIN.x, LOADING_MARGIN.y, 0.0);

	// this handles our rendering efficiently
	PlaneRenderer *planes = PlaneRenderer::getInstance();

	// load the "no drones" texture and the "loading" texture
	GLuint logo = loadPNG("../png/no-drones.png");
	GLuint loading = loadPNG("../png/loading.png");

	// compute the plane renderer's viewing params
	mat4 orthoProjection = ortho(0.0f, (float)windowSize.x, 0.0f, (float)windowSize.y, ORTHO_NEAR_RANGE, ORTHO_FAR_RANGE);
	mat4 orthoView = mat4(1.0);
	mat4 model;

	// dark grey background
	glClearColor(0.09, 0.09, 0.09, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the plane renderer's viewing parameters
	planes -> bindShader();
	planes -> setProjectionMatrix(orthoProjection);
	planes -> setViewMatrix(orthoView);

	// draw "no drones" logo
	glBindTexture(GL_TEXTURE_2D, logo);
	model = mat4(1.0);
	model = translate(model, LOGO_POSITION);
	model = scale(model, LOGO_SIZE);
	planes -> setColor(LOGO_COLOR);
	planes -> setModelMatrix(model);
    planes -> render();

	// draw loading message
	glBindTexture(GL_TEXTURE_2D, loading);
	model = mat4(1.0);
	model = translate(model, LOADING_POSITION);
	model = scale(model, LOADING_SIZE);
	planes -> setColor(LOADING_COLOR);
	planes -> setModelMatrix(model);
    planes -> render();

    // swap buffers so we can see everything
	glfwPollEvents();
	glfwSwapBuffers(window);
}
