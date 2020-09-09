#version 150

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat3 u_Normal;

in vec3 a_Vertex;
in vec3 a_Normal;
in vec2 a_TexCoord;
in mat4 a_Model;

out vec2 v_TexCoord;
out vec3 v_Normal;

void main()
{
	vec4 v_VertexPos = u_View * a_Model * vec4(a_Vertex, 1.0);
    v_Normal = normalize(u_Normal * a_Normal);
    v_TexCoord = a_TexCoord;

    gl_Position = u_Projection * v_VertexPos;
}
