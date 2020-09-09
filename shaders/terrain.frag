#version 150

uniform sampler2D u_Region1Tex;
uniform sampler2D u_Region2Tex;
uniform sampler2D u_Region3Tex;
uniform sampler2D u_ShadowTex;

uniform float u_Region1Max;
uniform float u_Region1Range;
uniform float u_Region2Max;
uniform float u_Region2Range;
uniform float u_Region3Max;
uniform float u_Region3Range;

in vec2 v_BaseTexCoord;
in vec4 v_Color;
in float v_Height;

out vec4 f_FragColor;

void main()
{
	float shadow = 1.0 - clamp(texture(u_ShadowTex, v_BaseTexCoord / 2048).r, 0.0, 1.0);

	float region1Contrib = max(0.0, (u_Region1Range - abs(v_Height - u_Region1Max)) / u_Region1Range);
	float region2Contrib = max(0.0, (u_Region2Range - abs(v_Height - u_Region2Max)) / u_Region2Range);
	float region3Contrib = max(0.0, (u_Region3Range - abs(v_Height - u_Region3Max)) / u_Region3Range);

	vec4 region1Color = (texture(u_Region1Tex, v_BaseTexCoord /   1.0) * 0.75) * region1Contrib + (texture(u_Region1Tex, v_BaseTexCoord / 512.0) * 0.25) * region1Contrib;
	vec4 region2Color = (texture(u_Region2Tex, v_BaseTexCoord / 128.0) * 0.75) * region2Contrib + (texture(u_Region2Tex, v_BaseTexCoord / 256.0) * 0.25) * region2Contrib;
	vec4 region3Color = (texture(u_Region3Tex, v_BaseTexCoord / 128.0) * 0.75) * region3Contrib + (texture(u_Region3Tex, v_BaseTexCoord / 512.0) * 0.25) * region3Contrib;

	f_FragColor = ((region1Color + region2Color + region3Color) * v_Color) * vec4(shadow, shadow, shadow, 0.0);
}
