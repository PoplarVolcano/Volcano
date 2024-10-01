#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;
layout(location = 5) in ivec4 a_BoneIds; 
layout(location = 6) in vec4 a_Weights;

layout(std140, binding = 0) uniform Camera
{
	// projection * view
	mat4 u_ViewProjection;
};

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

layout(std140, binding = 15) uniform BonesMatrices
{
    mat4 u_FinalBonesMatrices[MAX_BONES];
};

layout (location = 0) out vec2 v_TexCoords;


void main()
{
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(a_BoneIds[i] == -1) 
            continue;
        if(a_BoneIds[i] >=MAX_BONES) 
        {
            totalPosition = vec4(a_Position,1.0f);
            break;
        }
        vec4 localPosition = u_FinalBonesMatrices[a_BoneIds[i]] * vec4(a_Position,1.0f);
        totalPosition += localPosition * a_Weights[i];
        vec3 localNormal = mat3(u_FinalBonesMatrices[a_BoneIds[i]]) * a_Normal;
   }
	
    //mat4 viewModel = view * model;
    gl_Position =  u_ViewProjection * totalPosition;
	v_TexCoords = a_TexCoords;
}


#type fragment
#version 450 core
layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 v_TexCoords;

layout (binding = 0) uniform sampler2D texture_diffuse1;

void main()
{    
    FragColor = texture(texture_diffuse1, v_TexCoords);
}