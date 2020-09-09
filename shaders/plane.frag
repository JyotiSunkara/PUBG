#version 150

uniform sampler2D u_Texture;

uniform vec4 u_Color;

in vec2 v_TexCoord;

out vec4 f_FragColor;

void main()
{
	f_FragColor = texture(u_Texture, v_TexCoord) * u_Color;
}
