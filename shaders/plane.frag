#version 150

uniform sampler2D u_Texture;

uniform vec4 u_Color;
uniform vec3 fogColor;
uniform bool fogFlag;

in vec2 v_TexCoord;
in float visibility;

out vec4 f_FragColor;

void main()
{
	f_FragColor = texture(u_Texture, v_TexCoord) * u_Color;
	if(fogFlag) {
		f_FragColor = mix(vec4(fogColor, 1.0), f_FragColor, visibility);
	}
}
