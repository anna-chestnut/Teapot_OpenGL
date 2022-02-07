#shader vertex
#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragPos;

void main()
{
	//gl_Position = 0.05 * MVP * vec4(pos, 1);
	gl_Position = projection * view * model * vec4(pos, 1);

	FragPos = vec3(model * vec4(pos, 1.0));
	Normal = mat3(transpose(inverse(view * model))) * aNormal;   //clamp(transpose(inverse(view * model)) * vec4(aNormal, 1.0f), 0.0f, 1.0f);
};

#shader fragment
#version 330 core//410

layout(location = 0) out vec4 color;

//uniform vec4 u_Color;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 objectColor;

in vec3 Normal;
in vec3 FragPos;

void main()
{
	// ambient
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;

	// diffuse 
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	//color = clamp(u_Color * vec4(Normal, 1), 0.0f, 1.0f);
	//color = u_Color * vec4(Normal, 1.0f);

	vec3 result = (ambient + diffuse) * objectColor;// + diffuse + specular
	color = vec4(result, 1.0);

	// color = vec4( Normal, 1);
};