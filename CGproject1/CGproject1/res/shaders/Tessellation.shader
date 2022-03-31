#shader vertex
#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;

out vec2 TexCoord;

void main()
{
    //vs_out.color = aColor;
    //gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    gl_Position = vec4(aPos.x, aPos.y, 0.1, 1.0);
    TexCoord = aTex;
}


#shader tesscontrol
#version 410 core

layout(vertices = 4) out;

//uniform mat4 model;
//uniform mat4 view;

in vec2 TexCoord[];
out vec2 TextureCoord[];

void main()
{
    float num = 50.0;
    gl_TessLevelOuter[0] = num;
    gl_TessLevelOuter[1] = num;
    gl_TessLevelOuter[2] = num;
    gl_TessLevelOuter[3] = num;

    gl_TessLevelInner[0] = num;
    gl_TessLevelInner[1] = num;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    TextureCoord[gl_InvocationID] = TexCoord[gl_InvocationID];

}


#shader tessevaluation
#version 410 core
layout(quads, equal_spacing, ccw) in;

uniform sampler2D heightMap;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec2 TextureCoord[];

out float Height;
out vec3 tePosition;

out vec2 teTexCoord;

vec4 interpolate(vec4 v0, vec4 v1, vec4 v2, vec4 v3) {

    vec4 a = mix(v0, v1, gl_TessCoord.x);
    vec4 b = mix(v3, v2, gl_TessCoord.x);
    return mix(a, b, gl_TessCoord.y);
}


void main()
{
    /*gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position +
        gl_TessCoord.y * gl_in[1].gl_Position +
        gl_TessCoord.z * gl_in[2].gl_Position);*/

    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec2 t00 = TextureCoord[1];
    vec2 t01 = TextureCoord[0];
    vec2 t10 = TextureCoord[2];
    vec2 t11 = TextureCoord[3];

    vec2 t0 = (t01 - t00) * u + t00;
    vec2 t1 = (t11 - t10) * u + t10;
    teTexCoord = (t1 - t0) * v + t0;

    //vec3 normal = texture(heightMap, texCoord).rgb;
    //normal = normalize(normal * 2.0 - 1.0);

    //heightColor = texture(heightMap, texCoord).rgb;

    Height = texture(heightMap, teTexCoord).z * 10.0;// * 64.0 - 16.0
    vec4 inter = interpolate(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position);
    vec4 newPos = vec4(inter.x, inter.y, inter.z + Height, 1.0);
    gl_Position = projection * view * model * newPos;
    tePosition = vec3(model * newPos).xyz;
}


#shader geometry
#version 330 core

layout(triangles) in;
layout(line_strip, max_vertices = 3) out;

//in vec3 tePosition[3];
//out vec3 gPosition;

in vec2 teTexCoord[3];
out vec2 gTexCoord;

void main() {
    //build_house(gl_in[0].gl_Position);

    gl_Position = gl_in[0].gl_Position;
    gTexCoord = teTexCoord[0];
    //gPosition = tePosition[0];
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    gTexCoord = teTexCoord[1];
    //gPosition = tePosition[1];
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    gTexCoord = teTexCoord[2];
    //gPosition = tePosition[2];
    EmitVertex();

    //gl_Position = gl_in[0].gl_Position;
    //gTexCoord = teTexCoord[0];
    //EmitVertex();

    EndPrimitive();

}


#shader fragment
#version 330 core
out vec4 FragColor;

//in float Height;
in vec2 gTexCoord;
in vec2 teTexCoord;

in vec3 tePosition;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform sampler2D normalMap;

void main()
{

    vec3 col = vec3(1, 0, 0);

    vec3 ambient = 0.1 * col;

    //FragColor = vec4(fColor, 1.0);
    vec3 normal = texture(normalMap, teTexCoord).rgb;
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space

    // diffuse 
    vec3 lightDir = normalize(lightPos - tePosition);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * col;

    // specular
    vec3 viewDir = normalize(viewPos - tePosition);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    vec3 result = (ambient + diffuse + specular);

    FragColor = vec4(result, 1.0);

    /*float h = (Height) / 10.0f;
    FragColor = vec4(h, 0.0, 0.0, 1.0);*/
}
