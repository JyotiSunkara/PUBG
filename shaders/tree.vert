#version 150

uniform mat4 u_Projection;
uniform mat4 u_View;

in vec3 a_Vertex;
in vec3 a_Normal;
in vec2 a_TexCoord;
in mat4 a_InstanceMatrix;

out vec4 v_VertexPos;
out vec2 v_TexCoord;
out vec3 v_Normal;

void main()
{
	v_VertexPos = u_View * a_InstanceMatrix * vec4(a_Vertex, 1.0);;
	v_TexCoord = a_TexCoord;
	v_Normal = a_Normal;

	gl_Position = u_Projection * v_VertexPos;
}
