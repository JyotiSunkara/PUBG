#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include<iostream>

class Shader;
class Player;
class PlaneRenderer;

extern int hudDrones;
extern int hudScore;
extern int hudBullets;
extern int hudHealth;
extern int hudTime;

class HUD {
private:
	static const int NUM_BLOOD_SPLATTERS;				// how many splatter instances we want when the player dies

	Player *player;										// used to detect if the player is dead

	glm::vec2 orthoSize;								// we need the screen size to determine how to place the splatters
	glm::vec2 orthoCenter;								// ditto

	PlaneRenderer *plane;								// handle to simple plane rendering util

	GLuint reticleTexture;								// various textures used by the HUD; self-explanatory
	GLuint bulletIconTexture;
	GLuint bloodTexture;
	GLuint blackTexture;

	bool bloodEnabled;									// show the splatters? Or no?
	glm::mat4 *bloodSplatters;							// pre-computed, randomized model matrices for the splatters

	float fade;											// strength of black fade plane (0 is transparent, 1 is opaque)

	Shader *textShader;	

	// load up our resources
	void loadTextures();
	void setupBlood();

	// render whatever assets we need
	void renderReticle();
	void renderAmmo();
	void renderBlood();
	void renderFade();
	void renderDashboard();

public:
	HUD(Player *player, glm::mat4 &orthoProjection, glm::mat4 &orthoView, glm::vec2 windowSize);
	~HUD();

	// turn on blood splatters or render a black fade-out
	void enableBlood(bool bloodEnabled);
	void setFade(float fade);

	// usual update and render routines
	void update(float dt);
	void render();

	void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);
};
