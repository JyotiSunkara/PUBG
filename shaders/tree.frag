#version 150

uniform sampler2D u_DiffuseMap;
uniform sampler2D u_NormalMap;

uniform vec3 u_Sun;
uniform vec3 fogColor;
uniform bool fogFlag;
uniform float u_NormalMapStrength;

in vec4 v_VertexPos;
in vec2 v_TexCoord;
in vec3 v_Normal;
in float visibility;

out vec4 f_FragColor;

void main()
{
	f_FragColor = texture(u_DiffuseMap, v_TexCoord);
	if(f_FragColor.a < 0.6) discard;

	vec3 p = (texture(u_NormalMap, v_TexCoord).rgb - vec3(0.5)) * u_NormalMapStrength;
	vec3 normDelta = normalize(v_Normal + p);
	float diffuse = max(dot(u_Sun, normDelta), 0.3);

	f_FragColor.rgb *= diffuse;
	if(fogFlag) {
		f_FragColor = mix(vec4(fogColor, 1.0), f_FragColor, visibility);
	}
}
