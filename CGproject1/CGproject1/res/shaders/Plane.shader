#shader vertex
#version 330 core

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
#version 330 core

layout(location = 0) out vec4 color;

in vec2 TexCoords;

uniform sampler2D teapotTexture;
uniform vec3 planeColor;
uniform vec3 clearColor = vec3(0, 0, 0);

void main()
{
	vec3 col = texture(teapotTexture, TexCoords).rgb;
	if (col == clearColor) {
		col = planeColor;
	}
	color = vec4(col, 1.0);
};