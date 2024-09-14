#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

layout(std140, binding = 8) uniform LightSpaceMatrix
{
	mat4 u_LightSpaceMatrix;
};

void main()
{
    gl_Position = u_LightSpaceMatrix * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

void main()
{             
    // gl_FragDepth = gl_FragCoord.z;
}