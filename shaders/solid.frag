#version 150

uniform mat4 u_View;

uniform sampler2D u_DiffuseMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_SpecularMap;
uniform sampler2D u_EmissionMap;

uniform vec3 u_Sun;

uniform vec3 u_MaterialDiffuse;
uniform vec3 u_MaterialSpecular;
uniform float u_SpecularIntensity;
uniform float u_SpecularHardness;
uniform float u_NormalMapStrength;

in vec2 v_TexCoord;
in vec3 v_Normal;

out vec4 f_Color;

void main()
{
	vec3 litColor;
	float minBrightness = texture(u_EmissionMap, v_TexCoord.st).r;
	vec3 p = vec3((texture(u_NormalMap, v_TexCoord).rg) - vec2(0.5, 0.5), 0.0);
	vec3 texColor = texture(u_DiffuseMap, v_TexCoord.st).rgb;

	float d = dot(p, p);
	float f = inversesqrt(d + 1.0);

	vec3 sun = normalize(u_Sun);
	vec3 normDelta = normalize(v_Normal + (p * u_NormalMapStrength));
	float diffuse = max(dot(sun, normDelta), 0.1);

	vec3 reflectDir = reflect(normDelta, sun);
	float spec = max(dot(normalize(u_View[2].xyz), reflectDir), 0.0);
	spec = pow(spec, u_SpecularHardness);
	spec *= u_SpecularIntensity;

	litColor = texColor * u_MaterialDiffuse * diffuse;
	litColor = litColor + (litColor * spec * u_MaterialSpecular * texture(u_SpecularMap, v_TexCoord.st).r);
	litColor = mix(litColor, texColor, minBrightness);

	f_Color = vec4(litColor, 1.0);
}
