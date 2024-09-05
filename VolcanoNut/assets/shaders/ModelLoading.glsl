#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoords;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_Bitangent;
layout (location = 5) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	// projection * view
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
	vec3 Tangent;
	vec3 Bitangent;
};

layout (location = 0) out flat int v_EntityID;
layout (location = 1) out VertexOutput Output;

void main()
{
	v_EntityID = a_EntityID;
	Output.Position = a_Position;
	Output.Normal = a_Normal;
	Output.TexCoords = a_TexCoords;
	Output.Tangent = a_Tangent;
	Output.Bitangent = a_Bitangent;

    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
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
	vec3 Tangent;
	vec3 Bitangent;
};

layout (location = 0) in flat int v_EntityID;
layout (location = 1) in VertexOutput Input;

layout (binding = 0) uniform sampler2D texture_diffuse1;
layout (binding = 1) uniform sampler2D texture_specular1;
layout (binding = 2) uniform sampler2D texture_normal1;
layout (binding = 3) uniform sampler2D texture_height1;


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
	vec4 materialDiffuse = texture(texture_diffuse1, Input.TexCoords);

	vec4 materialSpecular = texture(texture_specular1, Input.TexCoords);
	
	vec4 materialNormal = texture(texture_normal1, Input.TexCoords);

	vec3 T = normalize(Input.Tangent);
    //vec3 B = normalize(Input.Bitangent);

    vec3 N = normalize(Input.Normal);
	vec3 B = cross(N, T);
	// 切线空间矩阵
	mat3 TBN = mat3(T, B, N);
	// 求逆，正交矩阵的逆就是转置
	// TBN = transpose(TBN);
	vec3 normal = normalize(materialNormal.rgb * 2.0 - 1.0);
	// 将法线贴图从切线空间转换到世界空间
	normal = normalize(TBN * normal);

	vec3 viewDirection = normalize(u_CameraPosition - Input.Position);

	vec3 result = vec3(0.0, 0.0, 0.0);

	// DirectionalLight
	{
		vec3 lightDirection = normalize(-u_DirectionalLightDirection);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		vec3 reflectDirection = reflect(-lightDirection, normal);
		float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), u_MaterialShininess);
		
		vec3 ambient  = u_DirectionalLightAmbient  * materialDiffuse.rgb;
		vec3 diffuse  = u_DirectionalLightDiffuse  * diff * materialDiffuse.rgb;
		vec3 specular = u_DirectionalLightSpecular * spec * materialSpecular.rgb;

		result += (ambient + diffuse + specular);
	}
	
	// PointLight
	{
		vec3 lightDirection = normalize(u_PointLightPosition - Input.Position);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		vec3 reflectDirection = reflect(-lightDirection, normal);
		float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), u_MaterialShininess);
		
		float distance = length(u_PointLightPosition - Input.Position);
		float attenuation = 1.0 / (u_PointLightConstant + u_PointLightLinear * distance +  u_PointLightQuadratic * (distance * distance));
		
	    vec3 ambient  = u_PointLightAmbient  * materialDiffuse.rgb;
		vec3 diffuse  = u_PointLightDiffuse  * diff * materialDiffuse.rgb;
		vec3 specular = u_PointLightSpecular * spec * materialSpecular.rgb;
	
	    ambient  *= attenuation;
		diffuse  *= attenuation;
		specular *= attenuation;
	
	    result += (ambient + diffuse + specular);
	}

	// SpotLight
	{
		vec3 lightDirection = normalize(u_SpotLightPosition - Input.Position);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		vec3 reflectDirection = reflect(-lightDirection, normal);  
		float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), u_MaterialShininess);
		
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

		result += (ambient + diffuse + specular);
	}


    FragmentColor = vec4(result, 1.0);

	o_EntityID = v_EntityID;
}

