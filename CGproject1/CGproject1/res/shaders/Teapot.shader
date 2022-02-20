#shader vertex
#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec2 aSpecCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord;
out vec2 SpecCoord;

void main()
{
	gl_Position = projection * view * model * vec4(pos, 1);
	FragPos = vec3(model * vec4(pos, 1.0));
	Normal = mat3(transpose(inverse(view * model))) * aNormal;

	TexCoord = aTexCoord;//vec2(aTexCoord.x, aTexCoord.y);
	SpecCoord = aSpecCoord;
};

#shader fragment
#version 330 core//410

layout(location = 0) out vec4 color;

uniform vec3 lightPos;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform vec3 viewPos;
uniform vec3 objectColor;
uniform float specularExponent;
uniform float specularStrength;
uniform sampler2D tex;
uniform sampler2D specTex;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

void main()
{
	// ambient
	float ambientStrength = 0.1;
	vec3 ambient = ambientColor * texture(tex, TexCoord).rgb;

	// diffuse 
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * diffuseColor * texture(tex, TexCoord).rgb;

	// specular
	//float specularStrength = 5.0;//0.5
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), specularExponent); // pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * specularColor * texture(specTex, TexCoord).rgb;

	vec3 result = (ambient + diffuse + specular);

	//texture
	color = vec4(result, 1.0);
};