#shader vertex
#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT{
    vec3 color;
} vs_out;

void main()
{
    vs_out.color = aColor;
    //gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, 0.1, 1.0);
}

#shader fragment
#version 330 core
out vec4 FragColor;

in vec3 fColor;

void main()
{
    FragColor = vec4(fColor, 1.0);
}

#shader geometry
#version 330 core

layout(triangles) in;
layout(line_strip, max_vertices = 3) out;

in VS_OUT{
    vec3 color;
} gs_in[];

out vec3 fColor;

void main() {
    //build_house(gl_in[0].gl_Position);
    fColor = vec3(0,1,0);

    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    EndPrimitive();

}

#shader tesscontrol
#version 410 core

layout(vertices = 4) out;

//uniform mat4 model;
//uniform mat4 view;

in vec3 myInData[];
out vec3 myData[];

void main()
{
    float num = 8.0;
    gl_TessLevelOuter[0] = num;
    gl_TessLevelOuter[1] = num;
    gl_TessLevelOuter[2] = num;
    gl_TessLevelOuter[3] = num;

    gl_TessLevelInner[0] = num;
    gl_TessLevelInner[1] = num;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    //TextureCoord[gl_InvocationID] = TexCoord[gl_InvocationID];

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

    gl_Position = interpolate(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position);
}