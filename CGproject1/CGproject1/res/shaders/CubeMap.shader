#shader vertex
#version 330 core

layout(location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 TexCoords;

void main()
{
	TexCoords = pos;
	vec4 tmpPos = projection * view * vec4(pos, 1);
	gl_Position = tmpPos;
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
	color = texture(skybox, TexCoords);
};