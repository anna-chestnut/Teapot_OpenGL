#shader vertex
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1);
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    
    vs_out.TexCoords = aTexCoords;
        
}

#shader fragment
#version 330 core

out vec4 FragColor;

in VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
} fs_in;

//uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vec3 col = vec3(1, 0, 0);
    
    vec3 ambient = 0.1 * col;

    vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space

    // diffuse 
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * col;

    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    vec3 result = (ambient + diffuse + specular);

    //texture
    FragColor = vec4(result, 1.0);

}