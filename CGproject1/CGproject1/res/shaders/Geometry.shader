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
layout(line_strip, max_vertices = 6) out;

in VS_OUT{
    vec3 color;
} gs_in[];

out vec3 fColor;

void build_house(vec4 position)
{
    fColor = gs_in[0].color; // gs_in[0] since there's only one input vertex
    gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0); // 1:bottom-left   
    EmitVertex();
    gl_Position = position + vec4(0.2, -0.2, 0.0, 0.0); // 2:bottom-right
    EmitVertex();
    gl_Position = position + vec4(-0.2, 0.2, 0.0, 0.0); // 3:top-left
    EmitVertex();
    gl_Position = position + vec4(0.2, 0.2, 0.0, 0.0); // 4:top-right
    EmitVertex();
    gl_Position = position + vec4(0.0, 0.4, 0.0, 0.0); // 5:top
    fColor = vec3(1.0, 1.0, 1.0);
    EmitVertex();
    EndPrimitive();
}

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