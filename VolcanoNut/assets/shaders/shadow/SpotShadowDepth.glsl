#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout(location = 5) in ivec4 a_BoneIds; 
layout(location = 6) in vec4  a_Weights;

layout(std140, binding = 6) uniform ModelTransform
{
	mat4 u_ModelTransform;
    mat4 u_NormalTransform;
};

layout(std140, binding = 10) uniform LightSpaceMatrix
{
	mat4 u_LightSpaceMatrix;
    float u_FarPlane;
};

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

layout(std140, binding = 15) uniform BonesMatrices
{
    mat4 u_FinalBonesMatrices[100];
};

void main()
{
    vec4 totalPosition = vec4(0.0);
    if (u_FinalBonesMatrices[0] != mat4(0.0))
    {
        for(int i = 0 ; i < 4 ; i++)
        {
            if(a_BoneIds[i] == -1) 
                continue;
            if(a_BoneIds[i] >= 100) 
            {
                totalPosition = vec4(a_Position,1.0f);
                break;
            }
            vec4 localPosition = u_FinalBonesMatrices[a_BoneIds[i]] * vec4(a_Position,1.0f);
            totalPosition += localPosition * a_Weights[i];
        }
	}
    else
    {
        totalPosition = vec4(a_Position, 1.0);
    }

    gl_Position = u_LightSpaceMatrix * u_ModelTransform * totalPosition;
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
    gl_FragDepth = gl_FragCoord.z;
}