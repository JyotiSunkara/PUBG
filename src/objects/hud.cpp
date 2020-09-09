#include "objects/hud.h"
#include "objects/player.h"

#include "util/planerenderer.h"
#include "util/loadtexture.h"
#include "util/planerenderer.h"

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/random.hpp"
using namespace glm;

const int HUD::NUM_BLOOD_SPLATTERS = 10;

HUD::HUD(Player *player, mat4 &orthoProjection, mat4 &orthoView, vec2 windowSize)
{
	this -> player = player;

	// save our screen characteristics
	orthoSize = windowSize;
	orthoCenter = orthoSize / 2.0f;

	// we only need to send this once, so let's do it now
	plane = PlaneRenderer::getInstance();
	plane -> bindShader();
	plane -> setProjectionMatrix(orthoProjection);				// projection mat never changes
	plane -> setViewMatrix(orthoView);							// ortho mat never changes

	// turn off blood effect for now
	enableBlood(false);

	// load up any resources we need
	loadTextures();
	setupBlood();
}

HUD::~HUD()
{
	delete plane;
	delete[] bloodSplatters;
}

void HUD::loadTextures()
{
	reticleTexture = loadPNG("../png/reticle.png");
	bulletIconTexture = loadPNG("../png/bullet-icon.png");
	bloodTexture = loadPNG("../png/blood.png");
	blackTexture = loadPNG("../png/black.png");
}

void HUD::setupBlood()
{
	const float MIN_BLOOD_SPLATTER_SIZE = 700.0;
	const float MAX_BLOOD_SPLATTER_SIZE = 1100.0;
	int i;

	// pre-compute random positions, scales, and orientations for all of the blood splatters we want
	bloodSplatters = new mat4[NUM_BLOOD_SPLATTERS];
	for(i = 0; i < NUM_BLOOD_SPLATTERS; i ++)
	{
		bloodSplatters[i] = mat4(1.0);
		bloodSplatters[i] = translate(bloodSplatters[i], linearRand(vec3(0.0, 0.0, 0.0), vec3(orthoSize, 0.0)));
		bloodSplatters[i] = scale(bloodSplatters[i], linearRand(vec3(MIN_BLOOD_SPLATTER_SIZE), vec3(MAX_BLOOD_SPLATTER_SIZE)));
		bloodSplatters[i] = rotate(bloodSplatters[i], (float)linearRand(-M_PI_2, M_PI_2), vec3(0.0, 0.0, 1.0));
	}
}

void HUD::render()
{
	glDisable(GL_DEPTH_TEST);								// never occluded, always visible
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ONE);
	glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);				// makes the HUD a bit easier to see
	glActiveTexture(GL_TEXTURE0);
	plane -> bindShader();

	// player aiming reticle and ammo stats
	renderReticle();
	renderAmmo();

	// blood (and everything else afterwards) uses standard blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	// render the blood if enabled
	if(bloodEnabled)
	{
		renderBlood();
	}
	renderFade();

	glEnable(GL_DEPTH_TEST);
}

void HUD::renderReticle()
{
	mat4 modelMatrix = mat4(1.0);
	vec4 color = vec4(0.1, 0.1, 0.1, 1.0);

	modelMatrix = translate(modelMatrix, vec3(orthoCenter, 0.0));
	modelMatrix = scale(modelMatrix, vec3(32.0f));

	glBindTexture(GL_TEXTURE_2D, reticleTexture);
	plane -> setModelMatrix(modelMatrix);
	plane -> setColor(color);
	plane -> render();
}

void HUD::renderAmmo()
{
	int numShotsInClip = player -> getNumShotsInClip();

	mat4 modelMatrix = mat4(1.0);
	vec4 color = vec4(0.3, 0.3, 0.3, 1.0);
	int i;

	glBindTexture(GL_TEXTURE_2D, bulletIconTexture);
	plane -> setColor(color);

	// this is somewhat expensive; it would be better to pre-compute these model matrices rather than
	// building them every render step, since they never change; we could even allow the plane renderer
	// to batch render an array of model matrices, for even more performance gains (but the impact here
	// is negligible anyways)
	for(i = 0; i < numShotsInClip; i ++)
	{
		modelMatrix = mat4(1.0);
		modelMatrix = translate(modelMatrix, vec3(orthoCenter, 0.0));
		modelMatrix = rotate(modelMatrix, (float)((float)i / (float)Player::MAX_ROUNDS_PER_CLIP * 2 * -M_PI), vec3(0.0, 0.0, 1.0));
		modelMatrix = translate(modelMatrix, vec3(0.0, 45.0, 0.0));
		modelMatrix = scale(modelMatrix, vec3(6.0, 6.0, 1.0));

		plane -> setModelMatrix(modelMatrix);
		plane -> render();
	}
}

void HUD::renderBlood()
{
	int i;
	vec4 BLOOD_COLOR(0.55, 0.03, 0.03, 1.0);

	glBindTexture(GL_TEXTURE_2D, bloodTexture);
	plane -> setColor(BLOOD_COLOR);

	for(i = 0; i < NUM_BLOOD_SPLATTERS; i ++)
	{
		plane -> setModelMatrix(bloodSplatters[i]);
		plane -> render();
	}
}

void HUD::renderFade()
{
	vec4 BLACK_COLOR(0.0, 0.0, 0.0, fade);
	mat4 modelMatrix;

	// don't render anything if the fade is practically invisible
	if(fade > 0.001)
	{
        glBindTexture(GL_TEXTURE_2D, blackTexture);

		modelMatrix = mat4(1.0);
		modelMatrix = translate(modelMatrix, vec3(orthoCenter, 0.0));
		modelMatrix = scale(modelMatrix, vec3(orthoSize.x, orthoSize.y, 1.0));

		plane -> setModelMatrix(modelMatrix);
		plane -> setColor(BLACK_COLOR);
		plane -> render();
	}
}

void HUD::enableBlood(bool bloodEnabled)
{
	this -> bloodEnabled = bloodEnabled;
}

void HUD::setFade(float fade)
{
	this -> fade = fade;
}
