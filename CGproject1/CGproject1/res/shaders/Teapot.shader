#shader vertex
#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 aNormal;
uniform mat4 MVP;
//niform mat3 MV;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 Normal;

void main()
{
	//gl_Position = 0.05 * MVP * vec4(pos, 1);
	gl_Position = projection * view * model * vec4(pos, 1);
	Normal = clamp(transpose(inverse(view * model)) * vec4(aNormal, 1.0f), 0.0f, 1.0f);
};

#shader fragment
#version 330 core//410

layout(location = 0) out vec4 color;

uniform vec4 u_Color;
in vec4 Normal;

void main()
{
	//color = clamp(u_Color * vec4(Normal, 1), 0.0f, 1.0f);
	color = u_Color * Normal;

	// color = vec4( Normal, 1);
};