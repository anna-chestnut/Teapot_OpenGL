#shader vertex
#version 330 core

layout(location = 0) in vec3 pos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
	gl_Position = lightSpaceMatrix * model * vec4(pos, 1);
	//lightView_Position = matrixShadow * vec4(pos, 1);
};

#shader fragment
#version 330 core

layout(location = 0) out float fragmentdepth;

void main()
{
	fragmentdepth = gl_FragCoord.z;
};