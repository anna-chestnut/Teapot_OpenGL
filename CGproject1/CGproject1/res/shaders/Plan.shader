#shader vertex
#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 aTexCoord;

out vec2 TexCoords;

void main()
{
	TexCoords = aTexCoord;
	gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 TexCoord;

uniform sampler2D screenTexture;

void main()
{
	vec3 col = texture(screenTexture, TexCoord).rgb;
	color = vec4(col, 1.0);
};