#shader vertex
#version 330 core

layout(location = 0) in vec3 pos;
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
#version 330 core

layout(location = 0) out vec4 color;

uniform vec3 planColor;

in vec2 TexCoord;

uniform sampler2D screenTexture;

void main()
{
	vec3 col = planColor * texture(screenTexture, TexCoord).rgb;
	color = vec4(col, 1.0);
};