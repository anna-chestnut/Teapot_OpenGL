#shader vertex
#version 330 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 Position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    Normal = mat3(transpose(inverse(model))) * aNormal;
    Position = vec3(model * vec4(pos, 1.0));// Position is in world-space
    gl_Position = projection * view * model * vec4(pos, 1.0);
}

#shader fragment
#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform vec3 lightPos;
uniform float specularExponent;
uniform float specularStrength;

void main()
{

    // reflection
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    //FragColor = vec4(texture(skybox, R).rgb, 1.0);
    vec3 color = texture(skybox, R).rgb;

    //ambient
    vec3 ambient = 0.05 * color;

    //diffuse
    vec3 lightDir = normalize(lightPos - Position);
    vec3 normal = normalize(Normal);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * color;

    //specular
    vec3 viewDir = normalize(cameraPos - Position);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), specularExponent);
    vec3 specular = vec3(0.3) * spec; // assuming bright white light color

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}