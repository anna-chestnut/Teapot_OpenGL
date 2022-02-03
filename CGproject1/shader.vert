#version 330 core

layout(location = 0) in vec3 pos;
uniform mat4 MVP;

void main()
{
	gl_Position = 0.05 * MVP * vec4( pos, 1);
};
