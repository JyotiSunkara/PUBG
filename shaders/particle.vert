#version 150

uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;
uniform vec3 u_CameraRight;
uniform vec3 u_CameraUp;

in vec3 a_Position;
in vec4 a_Color;
in float a_Size;
in float a_Angle;

out vec4 v_Color;
out vec2 v_TexCoord;

const vec2 vertices[] = vec2[4](
  vec2(-0.5,  0.5),
  vec2(-0.5, -0.5),
  vec2(0.5,   0.5),
  vec2(0.5,  -0.5)
);

const vec2 texCoords[] = vec2[4](
  vec2(0.0, 1.0),
  vec2(0.0, 0.0),
  vec2(1.0, 1.0),
  vec2(1.0, 0.0)
);

void main()
{
	vec2 vertexCoord = vertices[gl_VertexID];
	vec2 texCoord = texCoords[gl_VertexID];

	// compute rotated corner points
	float c = cos(a_Angle);
	float s = sin(a_Angle);
	vec3 vertex = vec3(vertexCoord.x * c - vertexCoord.y * s,
					   vertexCoord.x * s + vertexCoord.y * c,
					   0.0);

	// apply particle size
	vertex.x *= a_Size;
	vertex.y *= a_Size;

	// assign vertex color and texture coordinates
	v_Color = a_Color;
    v_TexCoord = texCoord;

	// assign billboarded position based on camera orientation vectors
	mat4 viewProjection = u_ProjectionMatrix * u_ViewMatrix;
	gl_Position = viewProjection * vec4(a_Position + u_CameraRight * vertex.x +
													 u_CameraUp * vertex.y, 1.0);
}
