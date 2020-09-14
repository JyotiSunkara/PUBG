#version 150

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat3 u_Normal;

in vec3 a_Vertex;
in vec3 a_Normal;
in vec2 a_TexCoord;

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec4 v_VertexPos;
out float visibility;

const float density = 0.01;
const float gradient = 0.6;

void main()
{
	v_VertexPos = u_View * u_Model * vec4(a_Vertex, 1.0);
    v_Normal = normalize(u_Normal * a_Normal);
    v_TexCoord = a_TexCoord;

    gl_Position = u_Projection * v_VertexPos;
    float dist = length(gl_Position);
	visibility = exp(-pow(dist * density, gradient));
	visibility = clamp(visibility, 0.0, 1.0);
}
