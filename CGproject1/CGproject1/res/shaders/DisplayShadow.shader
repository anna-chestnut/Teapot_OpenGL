#shader vertex
#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 Normal;
out vec3 Position;
out vec2 TexCoords;
out vec4 lightView_Position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
//vec3 aNormal = vec3(0, 1, 0);

void main()
{
	TexCoords = aTexCoord;
	Normal = mat3(transpose(inverse(model))) * aNormal;
	Position = vec3(model * vec4(pos, 1.0));// Position is in world-space
	lightView_Position = lightSpaceMatrix * vec4(pos, 1);
	gl_Position = projection * view * model * vec4(pos, 1);
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 Position;
in vec4 lightView_Position;

uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

    return shadow;
}

void main()
{
    //vec3 color = texture(diffuseTexture, TexCoords).rgb;
    vec3 color = vec3(1, 0, 0);
    vec3 normal = normalize(Normal);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    vec3 lightDir = normalize(lightPos - Position);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - Position);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;
    // calculate shadow
    float shadow = ShadowCalculation(lightView_Position);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    FragColor = vec4(lighting, 1.0);
}
//void main()
//{
//    float shadow = ShadowCalculation(lightView_Position);
//    vec3 color = vec3(0.75, 0.75, 0.75);
//    //color += texture(shadow, 1);
//    vec3 lighting = (1.0 - shadow) * color;
//
//    FragColor = vec4(lighting, 1.0);
//	//color = vec4(0.75, 0.75, 0.75, 1.0)
//    //vec3 p = lightView_Position.xyz / lightView_Position.w;
//    //color *= texture(shadow, p.xy).r < p.z ? 0 : 1;
//};