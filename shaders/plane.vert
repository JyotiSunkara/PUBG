#version 150

uniform mat4 u_Projection;
uniform mat4 u_Model;
uniform mat4 u_View;

in vec2 a_Vertex;
in vec2 a_TexCoord;

out vec2 v_TexCoord;
out float visibility;

const float density = 0.01;
const float gradient = 0.7;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = u_Projection * u_View * u_Model * vec4(a_Vertex, 0.0, 1.0);
	
	float dist = length(gl_Position);
	visibility = exp(-pow(dist * density, gradient));
	visibility = clamp(visibility, 0.0, 1.0);
}
