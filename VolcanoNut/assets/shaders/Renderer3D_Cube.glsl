#type vertex
#version 450 core

// �����position��normal�Ѿ���C++�������model�任
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec2 a_TexCoords;
layout(location = 4) in float a_DiffuseIndex;
layout(location = 5) in float a_SpecularIndex;
layout(location = 6) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	// projection * view
	mat4 u_ViewProjection;
};

layout(std140, binding = 8) uniform LightSpaceMatrix
{
	mat4 u_LightSpaceMatrix;
};

struct VertexOutput
{
	vec3 Position;
	vec3 Normal;
	vec4 Color;
	vec2 TexCoords;
    vec4 PositionLightSpace;
};

layout (location = 0) out flat float v_DiffuseIndex;
layout (location = 1) out flat float v_SpecularIndex;
layout (location = 2) out flat int v_EntityID;
layout (location = 3) out VertexOutput Output;

void main()
{
	v_DiffuseIndex = a_DiffuseIndex;
	v_SpecularIndex = a_SpecularIndex;
	v_EntityID = a_EntityID;
	Output.Position = a_Position;
	Output.Normal = a_Normal;
	Output.Color = a_Color;
	Output.TexCoords = a_TexCoords;
	Output.PositionLightSpace = u_LightSpaceMatrix * vec4(Output.Position, 1.0);

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

struct VertexOutput
{
	vec3 Position;
	vec3 Normal;
	vec4 Color;
	vec2 TexCoords;
    vec4 PositionLightSpace;
};

layout (location = 0) in flat float v_DiffuseIndex;
layout (location = 1) in flat float v_SpecularIndex;
layout (location = 2) in flat int v_EntityID;
layout (location = 3) in VertexOutput Input;

layout (binding = 0)  uniform sampler2D u_Textures[30];
layout (binding = 30) uniform sampler2D u_SpotDepthMap;
layout (binding = 31) uniform sampler2D u_DepthMap;
layout (binding = 0)  uniform samplerCube u_DepthCubeMap;

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
	vec3 u_PointLightPosition;
	vec3 u_PointLightAmbient;
	vec3 u_PointLightDiffuse;
	vec3 u_PointLightSpecular;
	float u_PointLightConstant;
    float u_PointLightLinear;
    float u_PointLightQuadratic;
};


layout (std140, binding = 4) uniform SpotLight
{
	vec3 u_SpotLightPosition;
	vec3 u_SpotLightDirection;
	vec3 u_SpotLightAmbient;
	vec3 u_SpotLightDiffuse;
	vec3 u_SpotLightSpecular;
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

layout(std140, binding = 9) uniform PointLightShadow
{
    mat4 u_ShadowMatrices[6];
    float u_PointFarPlane;
};

layout(std140, binding = 10) uniform SpotLightShadow
{
    mat4 u_SpotLightSpace;
    float u_SpotFarPlane;
};

void main()
{
	vec3 Ambient  = vec3(0.0, 0.0, 0.0);
	vec3 Diffuse  = vec3(0.0, 0.0, 0.0);
	vec3 Specular = vec3(0.0, 0.0, 0.0);

	// ��������ͼ
	int diffuseIndex = int(v_DiffuseIndex);
	vec4 materialDiffuse = Input.Color;
	materialDiffuse *= texture(u_Textures[diffuseIndex], Input.TexCoords);

	if (materialDiffuse.a == 0.0)
		discard;
		
	// �������ͼ
	int specularIndex = int(v_SpecularIndex);
	vec4 materialSpecular = vec4(1.0, 1.0, 1.0, 1.0);
	materialSpecular *= texture(u_Textures[specularIndex], Input.TexCoords);
	
	vec3 normal = normalize(Input.Normal);
	vec3 viewDirection = normalize(u_CameraPosition - Input.Position);
	

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
		
        vec3 projCoords = Input.PositionLightSpace.xyz / Input.PositionLightSpace.w;
        projCoords = projCoords * 0.5 + 0.5;
        float closestDepth = texture(u_DepthMap, projCoords.xy).r; 
        float currentDepth = projCoords.z;
	    float bias = max(0.005 * (1.0 - dot(normal, -u_DirectionalLightDirection)), 0.0005);
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	    if(projCoords.z > 1.0)
            shadow = 0.0;
		
		Ambient  += ambient;
		Diffuse  += (1.0 - shadow) * diffuse;
		Specular += (1.0 - shadow) * specular;
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
	
	    /*
	    // TODO�����Դ����������ͼ��ȡ�޷�˳�����У�����Ҫ�Ż�
	    vec3 fragToLight = Input.Position - u_PointLightPosition;
        float currentDepth = length(fragToLight);
        float closestDepth = texture(u_DepthCubeMap, fragToLight).r;
        closestDepth *= u_PointFarPlane;
        float bias = 0.05;
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
		*/
		float shadow = 0;

		Ambient  += ambient;
		Diffuse  += (1.0 - shadow) * diffuse;
		Specular += (1.0 - shadow) * specular;
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
		
	    vec4 positionLightSpace = u_SpotLightSpace * vec4(Input.Position, 1.0);
        vec3 projCoords = positionLightSpace.xyz / positionLightSpace.w;
        projCoords = projCoords * 0.5 + 0.5;
        float closestDepth = texture(u_SpotDepthMap, projCoords.xy).r; 
        float currentDepth = projCoords.z;
	    float bias = max(0.005 * (1.0 - dot(normal, -u_SpotLightDirection)), 0.0005);
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

		Ambient  += ambient;
		Diffuse  += (1.0 - shadow) * diffuse;
		Specular += (1.0 - shadow) * specular;

	}

	/*
	// �ữ���Դ��Ӱ
	vec3 fragToLight = Input.Position - u_PointLightPosition;
    float currentDepth = length(fragToLight);
	float shadow = 0.0;
    float bias = 0.05; 
    float samples = 4.0;
    float offset = 0.1;
    for(float x = -offset; x < offset; x += offset / (samples * 0.5))
    {
        for(float y = -offset; y < offset; y += offset / (samples * 0.5))
        {
            for(float z = -offset; z < offset; z += offset / (samples * 0.5))
            {
                float closestDepth = texture(u_DepthCubeMap, fragToLight + vec3(x, y, z)).r; 
                closestDepth *= u_PointFarPlane;
                if(currentDepth - bias > closestDepth)
                    shadow += 1.0;
            }
        }
    }
    shadow /= (samples * samples * samples);
    vec3 lighting = (Ambient + (1.0 - shadow) * (Diffuse + Specular));
	*/

	vec3 lighting = Ambient + Diffuse + Specular;

    o_Color = vec4(lighting, materialDiffuse.a);

	o_EntityID = v_EntityID;
}
