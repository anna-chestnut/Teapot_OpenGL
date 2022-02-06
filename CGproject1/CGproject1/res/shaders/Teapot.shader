#shader vertex
#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 aNormal;
uniform mat4 MVP;
uniform mat3 MV;

out vec3 Normal;

void main()
{
	gl_Position = 0.05 * MVP * vec4(pos, 1);
	Normal = MV * aNormal;
};

#shader fragment
#version 330 core//410

layout(location = 0) out vec4 color;

uniform vec4 u_Color;
in vec3 Normal;

void main()
{
	color = clamp(u_Color * vec4(Normal, 1), 0.0f, 1.0f);
	//color = u_Color * vec4(Normal, 1);

	// color = vec4( Normal, 1);
};