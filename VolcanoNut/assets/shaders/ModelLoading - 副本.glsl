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

layout(std140, binding = 6) uniform Transform
{
	mat4 u_Transform;
	mat3 u_NormalTransform;
};

layout(std140, binding = 8) uniform LightSpaceMatrix
{
	mat4 u_LightSpaceMatrix;
};

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

layout(std140, binding = 15) uniform BonesMatrices
{
    mat4 u_FinalBonesMatrices[MAX_BONES];
};

struct VertexOutput
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
	mat3 TBN;
    vec4 PositionLightSpace;
};

layout (location = 0) out flat float v_DiffuseIndex;
layout (location = 1) out flat float v_SpecularIndex;
layout (location = 2) out flat float v_NormalIndex;
layout (location = 3) out flat float v_ParallaxIndex;
layout (location = 4) out flat int   v_EntityID;
layout (location = 5) out VertexOutput Output;

void main()
{

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

	//vec4 position = u_Transform * vec4(a_Position, 1.0);
	//vec3 T = normalize(u_NormalTransform * a_Tangent);
    //vec3 B = normalize(u_NormalTransform * a_Bitangent);
    //vec3 N = normalize(u_NormalTransform * a_Normal);

	vec4 position = vec4(a_Position, 1.0);

    gl_Position = u_ViewProjection * totalPosition;
	
	vec3 T = normalize(a_Tangent);
    vec3 B = normalize(a_Bitangent);
    vec3 N = normalize(a_Normal);
	mat3 TBN = mat3(T, B, N);
	
	v_DiffuseIndex   = a_DiffuseIndex;
	v_SpecularIndex  = a_SpecularIndex;
	v_NormalIndex    = a_NormalIndex;
	v_ParallaxIndex  = a_ParallaxIndex;
	v_EntityID       = a_EntityID;
	Output.Position  = position.rgb;
	Output.Normal    = N;
	Output.TexCoords = a_TexCoords;
	Output.TBN       = TBN;
	Output.PositionLightSpace = u_LightSpaceMatrix * vec4(Output.Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 FragmentColor;
layout(location = 1) out int o_EntityID;

struct VertexOutput
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
	mat3 TBN;
    vec4 PositionLightSpace;
};

layout (location = 0) in flat float v_DiffuseIndex;
layout (location = 1) in flat float v_SpecularIndex;
layout (location = 2) in flat float v_NormalIndex;
layout (location = 3) in flat float v_ParallaxIndex;
layout (location = 4) in flat int   v_EntityID;
layout (location = 5) in VertexOutput Input;

layout (binding = 0) uniform sampler2D u_Textures[32];


layout(std140, binding = 1) uniform CameraPosition
{
	vec3 u_CameraPosition;
};


layout (std140, binding = 2) uniform DirectionalLight
{
	vec3 u_DirectionalLightDirection;
	vec3 u_DirectionalLightAmbient;
	vec3 u_DirectionalLightDiffuse;
	vec3 u_DirectionalLightSpecular;
};

layout (std140, binding = 3) uniform PointLight
{
	vec3  u_PointLightPosition;
	vec3  u_PointLightAmbient;
	vec3  u_PointLightDiffuse;
	vec3  u_PointLightSpecular;
	float u_PointLightConstant;
    float u_PointLightLinear;
    float u_PointLightQuadratic;
};


layout (std140, binding = 4) uniform SpotLight
{
	vec3  u_SpotLightPosition;
	vec3  u_SpotLightDirection;
	vec3  u_SpotLightAmbient;
	vec3  u_SpotLightDiffuse;
	vec3  u_SpotLightSpecular;
	float u_SpotLightConstant;
    float u_SpotLightLinear;
    float u_SpotLightQuadratic;
	float u_SpotLightCutOff;
	float u_SpotLightOuterCutOff;
};

layout (std140, binding = 5) uniform Material
{
	float u_MaterialShininess;
};

void main()
{
	vec4 materialDiffuse  = texture(u_Textures[int(v_DiffuseIndex)],  Input.TexCoords);
	vec4 materialSpecular = texture(u_Textures[int(v_SpecularIndex)], Input.TexCoords);
	vec4 materialNormal   = texture(u_Textures[int(v_NormalIndex)],   Input.TexCoords);

	
	vec3 normal = vec3(0.0);
	if(v_NormalIndex == 0.0)
	    normal = Input.Normal;
	else
	{
	    // 将法线贴图从切线空间转换为世界空间
	    normal = normalize(materialNormal.rgb * 2.0 - 1.0);
	    normal = normalize(Input.TBN * normal);
	}

	vec3 viewDirection = normalize(u_CameraPosition - Input.Position);
	
	vec3 Ambient  = vec3(0.0, 0.0, 0.0);
	vec3 Diffuse  = vec3(0.0, 0.0, 0.0);
	vec3 Specular = vec3(0.0, 0.0, 0.0);

	// DirectionalLight
	{
		vec3 lightDirection = normalize(-u_DirectionalLightDirection);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		//vec3 reflectDirection = reflect(-lightDirection, normal);
		//float spec = pow(max(dot(normal, reflectDirection), 0.0), u_MaterialShininess);
		vec3 halfwayDirection = normalize(lightDirection + viewDirection);
		float spec = pow(max(dot(normal, halfwayDirection), 0.0), u_MaterialShininess);
		
		vec3 ambient  = u_DirectionalLightAmbient  * materialDiffuse.rgb;
		vec3 diffuse  = u_DirectionalLightDiffuse  * diff * materialDiffuse.rgb;
		vec3 specular = u_DirectionalLightSpecular * spec * materialSpecular.rgb;
		
		Ambient  += ambient;
		Diffuse  += diffuse;
		Specular += specular;
	}
	
	// PointLight
	{
		vec3 lightDirection = normalize(u_PointLightPosition - Input.Position);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		//vec3 reflectDirection = reflect(-lightDirection, normal);
		//float spec = pow(max(dot(normal, reflectDirection), 0.0), u_MaterialShininess);
		vec3 halfwayDirection = normalize(lightDirection + viewDirection);
		float spec = pow(max(dot(normal, halfwayDirection), 0.0), u_MaterialShininess);
		
		float distance = length(u_PointLightPosition - Input.Position);
		float attenuation = 1.0 / (u_PointLightConstant + u_PointLightLinear * distance +  u_PointLightQuadratic * (distance * distance));
		
	    vec3 ambient  = u_PointLightAmbient  * materialDiffuse.rgb;
		vec3 diffuse  = u_PointLightDiffuse  * diff * materialDiffuse.rgb;
		vec3 specular = u_PointLightSpecular * spec * materialSpecular.rgb;
	
	    ambient  *= attenuation;
		diffuse  *= attenuation;
		specular *= attenuation;
	
		Ambient  += ambient;
		Diffuse  += diffuse;
		Specular += specular;
	}

	// SpotLight
	{
		vec3 lightDirection = normalize(u_SpotLightPosition - Input.Position);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		//vec3 reflectDirection = reflect(-lightDirection, normal);
		//float spec = pow(max(dot(normal, reflectDirection), 0.0), u_MaterialShininess);
		vec3 halfwayDirection = normalize(lightDirection + viewDirection);
		float spec = pow(max(dot(normal, halfwayDirection), 0.0), u_MaterialShininess);
		
		float distance    = length(u_SpotLightPosition - Input.Position);
		float attenuation = 1.0 / (u_SpotLightConstant + u_SpotLightLinear * distance + u_SpotLightQuadratic * (distance * distance));    
		
		// spotlight (soft edges)
		float theta = dot(-lightDirection, normalize(u_SpotLightDirection)); 
        float epsilon = (u_SpotLightCutOff - u_SpotLightOuterCutOff);
        float intensity = clamp((theta - u_SpotLightOuterCutOff) / epsilon, 0.0, 1.0);
		
		vec3 ambient  = u_SpotLightAmbient  * materialDiffuse.rgb;
		vec3 diffuse  = u_SpotLightDiffuse  * diff * materialDiffuse.rgb;  
		vec3 specular = u_SpotLightSpecular * spec * materialSpecular.rgb;

		ambient  *= intensity * attenuation;
        diffuse  *= intensity * attenuation;
        specular *= intensity * attenuation;
		
		Ambient  += ambient;
		Diffuse  += diffuse;
		Specular += specular;
	}
	
    vec3 lighting = (Ambient + Diffuse + Specular);

    FragmentColor = vec4(lighting, 1.0);

	o_EntityID = v_EntityID;
}

