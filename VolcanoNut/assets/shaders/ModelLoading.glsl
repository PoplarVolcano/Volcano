#type vertex
#version 450 core
layout (location = 0)  in vec3  a_Position;
layout (location = 1)  in vec3  a_Normal;
layout (location = 2)  in vec2  a_TexCoords;
layout (location = 3)  in vec3  a_Tangent;
layout (location = 4)  in vec3  a_Bitangent;
layout (location = 5)  in float a_DiffuseIndex;
layout (location = 6)  in float a_SpecularIndex;
layout (location = 7)  in float a_NormalIndex;
layout (location = 8)  in float a_ParallaxIndex;
layout (location = 9)  in int   a_EntityID;
layout (location = 10) in ivec4 a_BoneIds; 
layout (location = 11) in vec4  a_Weights;

layout(std140, binding = 0) uniform Camera
{
	// projection * view
	mat4 u_ViewProjection;
};

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

layout(std140, binding = 15) uniform BonesMatrices
{
    mat4 u_FinalBonesMatrices[100];
};

struct VertexOutput
{
	vec2 TexCoords;
};

layout (location = 0) out flat float v_DiffuseIndex;
layout (location = 1) out flat int   v_EntityID;
layout (location = 2) out VertexOutput Output;

void main()
{

     /*
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(a_BoneIds[i] == -1) 
            continue;
        if(a_BoneIds[i] >= MAX_BONES) 
        {
            totalPosition = vec4(a_Position,1.0f);
            break;
        }
        vec4 localPosition = u_FinalBonesMatrices[a_BoneIds[i]] * vec4(a_Position,1.0f);
        totalPosition += localPosition * a_Weights[i];
        vec3 localNormal = mat3(u_FinalBonesMatrices[a_BoneIds[i]]) * a_Normal;
    }
    */

	vec4 position = vec4(a_Position, 1.0);

    gl_Position = u_ViewProjection * position;
	
	v_DiffuseIndex   = a_DiffuseIndex;
	v_EntityID       = a_EntityID;
	Output.TexCoords = a_TexCoords;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 FragmentColor;
layout(location = 1) out int o_EntityID;

struct VertexOutput
{
	vec2 TexCoords;
};

layout (location = 0) in flat float v_DiffuseIndex;
layout (location = 1) in flat int   v_EntityID;
layout (location = 2) in VertexOutput Input;

layout (binding = 0) uniform sampler2D u_Textures[32];

void main()
{
	vec4 materialDiffuse  = texture(u_Textures[int(v_DiffuseIndex)],  Input.TexCoords);
    FragmentColor = materialDiffuse;

	o_EntityID = v_EntityID;
}

