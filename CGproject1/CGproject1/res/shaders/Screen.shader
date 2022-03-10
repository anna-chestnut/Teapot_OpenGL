#shader vertex
#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
	TexCoords = aTexCoords;
	gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
}

#shader fragment
#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 clearColor = vec3(0, 0, 0);
uniform sampler2D screenTexture;

void main()
{
	vec3 col = texture(screenTexture, TexCoords).rgb;

	if (col == clearColor) {
		discard;
	}
	else {
		FragColor = vec4(col, 1.0);
	}
	//vec3 col = texture(screenTexture, TexCoords).rgb;
	//FragColor = vec4(col, 1.0);
}