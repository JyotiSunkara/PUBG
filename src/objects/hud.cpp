#include "objects/hud.h"
#include "objects/player.h"
#include "objects/dronemanager.h"

#include "util/planerenderer.h"
#include "util/loadtexture.h"
#include "util/shader.h"
#include "util/planerenderer.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtc/type_ptr.hpp"
using namespace glm;

#include <iostream>
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H  

const int HUD::NUM_BLOOD_SPLATTERS = 5;

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

std::map<char, Character> Characters;
unsigned int VAO, VBO;

HUD::HUD(Player *player, mat4 &orthoProjection, mat4 &orthoView, vec2 windowSize) {

	
	// Text shader
	textShader = new Shader("../shaders/text.vert", "../shaders/text.frag");
	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	int windowWidth = mode -> width;
	int windowHeight = mode -> height;
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(windowWidth), 0.0f, static_cast<float>(windowHeight));
	textShader -> link();
	textShader -> bind();
	textShader -> uniformMatrix4fv("projection", 1, value_ptr(projection));
	textShader -> unbind();
	
	
	// FreeType BitMap and Glyph processing
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		fprintf(stderr, "FreeType: Could not init FreeType Library\n");
		return;
	}

	FT_Face face;
	if (FT_New_Face(ft, "/usr/share/fonts/truetype/lato/Lato-Bold.ttf", 0, &face))
	{
		fprintf(stderr, "FreeType: Failed to load font\n");
		return;
	}

	FT_Set_Pixel_Sizes(face, 0, 48);  
	if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
	{
		fprintf(stderr, "FreeType: Failed to load Glyph\n");
		return;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
  
	for (unsigned char c = 0; c < 128; c++)
	{
		// load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			fprintf(stderr, "FreeType: Failed to load Glyph\n");
			continue;
		}
		// Generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// now store character for later use
		Character character = {
			texture, 
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			(unsigned int) face->glyph->advance.x
		};
		Characters.insert(std::pair<char, Character>(c, character));
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	// Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

	// Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	

	this -> player = player;

	// Save our screen characteristics
	orthoSize = windowSize;
	orthoCenter = orthoSize / 2.0f;

	// We only need to send this once, so let's do it now
	plane = PlaneRenderer::getInstance();
	plane -> bindShader();
	plane -> setProjectionMatrix(orthoProjection);				// Projection mat never changes
	plane -> setViewMatrix(orthoView);							// Ortho mat never changes

	// Turn off blood effect for now
	enableBlood(false);

	// Load up any resources we need
	loadTextures();
	setupBlood();
}

HUD::~HUD() {
	delete plane;
	delete[] bloodSplatters;
}

void HUD::loadTextures() {
	reticleTexture = loadPNG("../png/reticle.png");
	bulletIconTexture = loadPNG("../png/bullet-icon.png");
	bloodTexture = loadPNG("../png/blood.png");
	blackTexture = loadPNG("../png/black.png");
}

void HUD::setupBlood() {
	const float MIN_BLOOD_SPLATTER_SIZE = 700.0;
	const float MAX_BLOOD_SPLATTER_SIZE = 1100.0;
	int i;

	// pre-compute random positions, scales, and orientations for all of the blood splatters we want
	bloodSplatters = new mat4[NUM_BLOOD_SPLATTERS];
	for(i = 0; i < NUM_BLOOD_SPLATTERS; i ++) {
		bloodSplatters[i] = mat4(1.0);
		bloodSplatters[i] = translate(bloodSplatters[i], linearRand(vec3(0.0, 0.0, 0.0), vec3(orthoSize, 0.0)));
		bloodSplatters[i] = scale(bloodSplatters[i], linearRand(vec3(MIN_BLOOD_SPLATTER_SIZE), vec3(MAX_BLOOD_SPLATTER_SIZE)));
		bloodSplatters[i] = rotate(bloodSplatters[i], (float)linearRand(-M_PI_2, M_PI_2), vec3(0.0, 0.0, 1.0));
	}
}


void HUD::render() {

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
	if(bloodEnabled) {
		renderBlood();
	}
	renderFade();

	char buffer[20];

	snprintf(buffer, 20, "TIME: %d", 1000 - hudTime);
	RenderText(buffer, 20.0f, 20.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
	snprintf(buffer, 20, "SCORE: %d", hudScore);
	RenderText(buffer, 20.0f, 40.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
	int numShotsInClip = player -> getNumShotsInClip();
	snprintf(buffer, 20, "BULLETS: %d", numShotsInClip);
	RenderText(buffer, 20.0f, 60.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
	snprintf(buffer, 20, "DRONES: %d", hudDrones);
	RenderText(buffer, 20.0f, 80.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
	int numReloads = player -> getNumReloads();
	snprintf(buffer, 20, "HEALTH: %d", numReloads);
	RenderText(buffer, 20.0f, 100.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));




	glEnable(GL_DEPTH_TEST);
}

void HUD::renderReticle() {


	mat4 modelMatrix = mat4(1.0);
	vec4 color = vec4(0.1, 0.1, 0.1, 1.0);
	

	modelMatrix = translate(modelMatrix, vec3(orthoCenter, 0.0));
	modelMatrix = scale(modelMatrix, vec3(32.0f));

	glBindTexture(GL_TEXTURE_2D, reticleTexture);
	plane -> setModelMatrix(modelMatrix);
	plane -> setColor(color);
	plane -> render();

}

void HUD::renderAmmo() {
	int numShotsInClip = player -> getNumShotsInClip();

	mat4 modelMatrix = mat4(1.0);
	vec4 color = vec4(0.3, 0.3, 0.3, 1.0);
	int i;

	glBindTexture(GL_TEXTURE_2D, bulletIconTexture);
	plane -> setColor(color);

	// Expensive
	for(i = 0; i < numShotsInClip; i ++) {
		modelMatrix = mat4(1.0);
		modelMatrix = translate(modelMatrix, vec3(orthoCenter, 0.0));
		modelMatrix = rotate(modelMatrix, (float)((float)i / (float)Player::MAX_ROUNDS_PER_CLIP * 2 * -M_PI), vec3(0.0, 0.0, 1.0));
		modelMatrix = translate(modelMatrix, vec3(0.0, 45.0, 0.0));
		modelMatrix = scale(modelMatrix, vec3(6.0, 6.0, 1.0));

		plane -> setModelMatrix(modelMatrix);
		plane -> render();
	}


}

void HUD::renderBlood() {
	int i;
	vec4 BLOOD_COLOR(0.55, 0.03, 0.03, 1.0);

	glBindTexture(GL_TEXTURE_2D, bloodTexture);
	plane -> setColor(BLOOD_COLOR);

	for(i = 0; i < NUM_BLOOD_SPLATTERS; i ++) {
		plane -> setModelMatrix(bloodSplatters[i]);
		plane -> render();
	}
}

void HUD::renderFade() {
	vec4 BLACK_COLOR(0.0, 0.0, 0.0, fade);
	mat4 modelMatrix;

	// don't render anything if the fade is practically invisible
	if(fade > 0.001) {
        glBindTexture(GL_TEXTURE_2D, blackTexture);

		modelMatrix = mat4(1.0);
		modelMatrix = translate(modelMatrix, vec3(orthoCenter, 0.0));
		modelMatrix = scale(modelMatrix, vec3(orthoSize.x, orthoSize.y, 1.0));

		plane -> setModelMatrix(modelMatrix);
		plane -> setColor(BLACK_COLOR);
		plane -> render();
	}
}

void HUD::enableBlood(bool bloodEnabled) {
	this -> bloodEnabled = bloodEnabled;
}

void HUD::setFade(float fade)  {
	this -> fade = fade;
}

void HUD::RenderText(std::string text, float x, float y, float scale, glm::vec3 color) {

    // Activate corresponding render state	
    textShader -> bind();
	textShader -> uniformVec3("textColor", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

