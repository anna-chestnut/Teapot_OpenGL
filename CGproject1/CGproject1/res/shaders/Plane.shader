#shader vertex
#version 330 core

layout(location = 0) in vec3 pos;
//layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 Normal;
out vec3 Position;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
vec3 aNormal = vec3(0, 1, 0);

void main()
{
	TexCoords = aTexCoord;
	Normal = mat3(transpose(inverse(model))) * aNormal;
	Position = vec3(model * vec4(pos, 1.0));// Position is in world-space
	gl_Position = projection * view * model * vec4(pos, 1);
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 TexCoords;
in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform sampler2D teapotReflect;
uniform vec3 planeColor;
uniform vec3 clearColor = vec3(0, 0, 0);

void main()
{
	//vec3 col = texture(teapotTexture, TexCoords).rgb;
	/*if (col == clearColor) {
		col = planeColor;
	//}*/
	//vec3 col = texture(teapotReflect, TexCoords).rgb;

	//if (col == clearColor) {
	//	// reflection
	//	vec3 I = normalize(Position - cameraPos);
	//	vec3 R = reflect(I, normalize(Normal));
	//	col = texture(skybox, R).rgb;
	//}

	//FragColor = vec4(texture(skybox, R).rgb, 1.0);
	//color = vec4(texture(skybox, R).rgb, 1.0);

	vec3 I = normalize(Position - cameraPos);
	vec3 R = reflect(I, normalize(Normal));
	vec3 col = texture(skybox, R).rgb;

	color = vec4(col, 1.0);
};