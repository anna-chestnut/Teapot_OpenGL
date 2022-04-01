#shader vertex
#version 410 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;

void main()
{
	TexCoords = aTexCoord;
	gl_Position = projection * view * model * vec4(pos, 1);
};

#shader fragment
#version 410 core

layout(location = 0) out vec4 color;

in vec2 TexCoords;

uniform sampler2D shadowMap;
uniform vec3 planeColor;
uniform vec3 clearColor = vec3(0, 0, 0);
uniform float near_plane;
uniform float far_plane;
uniform sampler2D teapotTexture;
// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0; // Back to NDC 
	return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{/*
	vec3 col = texture(shadowMap, TexCoords).rgb;
	color = vec4(col, 1.0);*/

	vec3 col = texture(teapotTexture, TexCoords).rgb;
	/*if (col == clearColor) {
		col = planeColor;
	}*/
	color = vec4(col, 1.0);

	//float depthValue = texture(shadowMap, TexCoords).r;
	//color = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
	//color = vec4(vec3(depthValue), 1.0);
};