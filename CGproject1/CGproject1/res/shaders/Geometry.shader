#shader vertex
#version 330 core

layout(location = 0) in vec3 aPos;
//layout(location = 1) in vec3 aColor;

out VS_OUT{
    //vec4 gl_Position;
    vec3 color;
} vs_out;

void main()
{
    vs_out.color = vec3(0.0, 1.0, 0.0);
    gl_Position = vec4(aPos.x, aPos.y, 1.0, 1.0);
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
    //vec4 gl_Position;
    vec3 color;
} data_in[];

out vec3 fColor;

void main() {

    gl_Position = gl_in[0].gl_Position;
    fColor = data_in[0].color;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    fColor = data_in[1].color;
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    fColor = data_in[2].color;
    EmitVertex();

    EndPrimitive();
}