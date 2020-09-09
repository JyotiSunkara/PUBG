#version 150

uniform sampler2D u_Texture;

in vec2 v_TexCoord;

out vec4 f_FragColor;

void main()
{
	f_FragColor = texture(u_Texture, v_TexCoord);
}
