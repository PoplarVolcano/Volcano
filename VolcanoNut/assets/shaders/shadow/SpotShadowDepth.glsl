#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

layout(std140, binding = 10) uniform LightSpaceMatrix
{
	mat4 u_LightSpaceMatrix;
    float u_FarPlane;
};

void main()
{
    gl_Position = u_LightSpaceMatrix * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(std140, binding = 10) uniform LightSpaceMatrix
{
	mat4 u_LightSpaceMatrix;
    float u_FarPlane;
};

void main()
{             
    // back to NDC(标准化设备坐标)[-1,1]
	//float z = gl_FragCoord.z * 2.0 - 1.0;
	//z = (2.0 * 0.1 * 1000) / (1000 + 0.1 - z * (1000 - 0.1));
    //float z = gl_FragCoord.z / gl_FragCoord.w;
    gl_FragDepth = gl_FragCoord.z;
}