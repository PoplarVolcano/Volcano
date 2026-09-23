#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

struct InstanceData
{            
	mat4 transform;
};

layout (std430, binding = 21) readonly buffer InstanceDataBuffer
{
    InstanceData u_InstanceData[];
};

layout(std140, binding = 8) uniform DirectionalLightShadowData
{
	mat4 u_LightSpaceMatrix;
};

void main()
{

    InstanceData instanceData = u_InstanceData[gl_InstanceIndex];

    gl_Position = u_LightSpaceMatrix * instanceData.transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

void main()
{             
    gl_FragDepth = gl_FragCoord.z;
}