#version 150

uniform mat4 u_Projection;
uniform mat4 u_Model;
uniform mat4 u_View;

uniform float u_GrassAreaRadius;
uniform float u_WaveTime;
uniform float u_WaveStrength;

in vec3 a_Vertex;
in vec4 a_Color;
in float a_Brightness;
in float a_ShadowValue;
in mat4 a_InstanceMatrix;

out vec4 v_Color;

void main()
{
	mat4 modelview = u_View * a_InstanceMatrix;

	// extract our scale vectors so we can keep them (we don't need Z because we don't zero out the 2nd modelview column,
	// since this is a cylindrical billboard, not a spherical one)
	// http://stackoverflow.com/questions/15937842/glsl-billboard-shader-keep-the-scaling
	//vec3 scaleX = modelview[0].xyz;
	//vec3 scaleY = modelview[1].xyz;

	// zero out out first column for a cylindrical billboard
	//modelview[0][0] = 0;//length(scaleX);
	modelview[0][1] = 0;
	modelview[0][2] = 0;

	// zero out our second column for a cylindrical billboard
	modelview[1][0] = 0;
	//modelview[1][1] = 0;//length(scaleY);
	modelview[1][2] = 0;

	// compute a wave amount that is dependent on our position and the height of the current vertex
	float waveAmount = (-0.5 + sin(u_WaveTime + (a_InstanceMatrix[3][0] + a_InstanceMatrix[3][2]))) * u_WaveStrength * a_Vertex.y;
	vec4 vertex = vec4(a_Vertex.x + waveAmount, a_Vertex.y, a_Vertex.z + waveAmount, 1.0);

	// now compute the position of this vertex based on the calculated wave and the cylindrical billboard
	vec4 pos = modelview * vertex;

	// fade out based on the distance from the camera
	float opacity = 1.0 - (-pos.z / u_GrassAreaRadius);

	// assign colour based on brightness and distance
	v_Color = a_Color * a_Brightness * (1.0 - a_ShadowValue);
	v_Color.a = opacity;

	// assign final vertex position
	gl_Position = u_Projection * pos;
}
