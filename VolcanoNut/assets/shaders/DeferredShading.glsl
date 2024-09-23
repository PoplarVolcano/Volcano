#type vertex
#version 450 core
layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoords;

struct VertexOutput
{
	vec2 TexCoords;
};

layout (location = 0) out VertexOutput Output;

void main()
{
	Output.TexCoords = a_TexCoords;

	gl_Position = vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out int  o_EntityID;
layout (location = 2) out vec4 BrightColor;

struct VertexOutput
{
	vec2 TexCoords;
};

layout (location = 0) in VertexOutput Input;

layout (binding = 0)  uniform sampler2D g_Position;
layout (binding = 1)  uniform sampler2D g_Normal;
layout (binding = 2)  uniform sampler2D g_Diffuse;
layout (binding = 3)  uniform sampler2D g_Specular;
layout (binding = 4)  uniform sampler2D ssao;

layout (binding = 30) uniform sampler2D u_SpotDepthMap;
layout (binding = 31) uniform sampler2D u_DirectionalDepthMap;
layout (binding = 0)  uniform samplerCube u_PointDepthMap;

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

layout(std140, binding = 8) uniform DirectionalLightShadow
{
	mat4 u_DirectionalLightSpace;
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
	
	vec3 fragPosition      = texture(g_Position, Input.TexCoords).rgb;
	float depth            = texture(g_Position, Input.TexCoords).a;
	vec4 materialNormal    = texture(g_Normal,   Input.TexCoords);
	vec4 materialDiffuse   = texture(g_Diffuse,  Input.TexCoords);
	vec4 materialSpecular  = texture(g_Specular, Input.TexCoords);
	float AmbientOcclusion = texture(ssao,       Input.TexCoords).r;

	if(materialDiffuse.a + materialNormal.a + materialSpecular.a == 0.0)
	    discard;

	vec3 viewDirection = normalize(u_CameraPosition - fragPosition);
	vec3 normal = materialNormal.rgb;

	// DirectionalLight
	if (u_DirectionalLightDirection != vec3(0.0))
	{
		vec3 lightDirection = normalize(-u_DirectionalLightDirection);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		//vec3 reflectDirection = reflect(-lightDirection, normal);
		//float spec = pow(max(dot(normal, reflectDirection), 0.0), u_MaterialShininess);
		vec3 halfwayDirection = normalize(lightDirection + viewDirection);
		float spec = pow(max(dot(normal, halfwayDirection), 0.0), u_MaterialShininess);
		
		vec3 ambient  = u_DirectionalLightAmbient;
		vec3 diffuse  = u_DirectionalLightDiffuse  * diff;
		vec3 specular = u_DirectionalLightSpecular * spec;
		
	    vec4 lightSpacePosition = u_DirectionalLightSpace * vec4(fragPosition, 1.0);
        vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;
        projCoords = projCoords * 0.5 + 0.5;
        float closestDepth = texture(u_DirectionalDepthMap, projCoords.xy).r; 
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
	if (u_PointLightPosition != vec3(0.0))
	{
		vec3 lightDirection = normalize(u_PointLightPosition - fragPosition);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		vec3 halfwayDirection = normalize(lightDirection + viewDirection);
		float spec = pow(max(dot(normal, halfwayDirection), 0.0), u_MaterialShininess);
		
		float distance = length(u_PointLightPosition - fragPosition);
		float attenuation = 1.0 / (u_PointLightConstant + u_PointLightLinear * distance +  u_PointLightQuadratic * (distance * distance));
		
	    vec3 ambient  = u_PointLightAmbient;
		vec3 diffuse  = u_PointLightDiffuse  * diff;
		vec3 specular = u_PointLightSpecular * spec;
	
	    ambient  *= attenuation;
		diffuse  *= attenuation;
		specular *= attenuation;
	
	    /*
	    // TODO：点光源的立方体贴图读取无法顺利进行，还需要优化
	    vec3 fragToLight = fragPosition - u_PointLightPosition;
        float currentDepth = length(fragToLight);
        float closestDepth = texture(u_PointDepthMap, fragToLight).r;
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
	if (u_SpotLightDirection != vec3(0.0))
	{
		vec3 lightDirection = normalize(u_SpotLightPosition - fragPosition);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		//vec3 reflectDirection = reflect(-lightDirection, normal);
		//float spec = pow(max(dot(normal, reflectDirection), 0.0), u_MaterialShininess);
		vec3 halfwayDirection = normalize(lightDirection + viewDirection);
		float spec = pow(max(dot(normal, halfwayDirection), 0.0), u_MaterialShininess);
		
		float distance    = length(u_SpotLightPosition - fragPosition);
		float attenuation = 1.0 / (u_SpotLightConstant + u_SpotLightLinear * distance + u_SpotLightQuadratic * (distance * distance));    
		
		// spotlight (soft edges)
		float theta = dot(-lightDirection, normalize(u_SpotLightDirection)); 
        float epsilon = (u_SpotLightCutOff - u_SpotLightOuterCutOff);
        float intensity = clamp((theta - u_SpotLightOuterCutOff) / epsilon, 0.0, 1.0);
		
		vec3 ambient  = u_SpotLightAmbient;
		vec3 diffuse  = u_SpotLightDiffuse  * diff;  
		vec3 specular = u_SpotLightSpecular * spec;

		ambient  *= intensity * attenuation;
        diffuse  *= intensity * attenuation;
        specular *= intensity * attenuation;
		
	    vec4 spotLightSpacePosition = u_SpotLightSpace * vec4(fragPosition, 1.0);
        vec3 projCoords = spotLightSpacePosition.xyz / spotLightSpacePosition.w;
        projCoords = projCoords * 0.5 + 0.5;
        float closestDepth = texture(u_SpotDepthMap, projCoords.xy).r; 
        float currentDepth = projCoords.z;
	    float bias = max(0.005 * (1.0 - dot(normal, -u_SpotLightDirection)), 0.0005);
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

		Ambient  += ambient;
		Diffuse  += (1.0 - shadow) * diffuse;
		Specular += (1.0 - shadow) * specular;

	}
	
	Ambient  *= materialDiffuse.rgb;
	Diffuse  *= materialDiffuse.rgb;  
	Specular *= materialSpecular.rgb;

	Ambient *= AmbientOcclusion;

	vec3 lighting = Ambient + Diffuse + Specular;

    FragColor = vec4(lighting, materialDiffuse.a);
	//FragColor = vec4(AmbientOcclusion,AmbientOcclusion,AmbientOcclusion,1.0);
	
	o_EntityID = int(materialNormal.a);

	 // Check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));//转换为灰度来计算片段的亮度
	if(brightness > 1.0)
        BrightColor = FragColor;
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}  