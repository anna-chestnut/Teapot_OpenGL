#shader vertex
#version 330 core

layout(location = 0) in vec3 pos;
uniform mat4 MVP;

void main()
{
	gl_Position = 0.05 * MVP * vec4( pos, 1);
};

#shader fragment
#version 330 core//410

layout(location = 0) out vec4 color;

uniform vec4 u_Color;

void main()
{
	color = u_Color;
};