#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

struct VertexOutput
{
	vec2 texCoord;
};

layout (location = 0) out VertexOutput Output;

void main()
{
	Output.texCoord = a_TexCoord;

	gl_Position = vec4(a_Position, 1.0f);
}

#type fragment
#version 450 core

layout (location = 0) out vec4 o_Ambient;  // 环境光
layout (location = 1) out vec4 o_Diffuse;  // 漫反射
layout (location = 2) out vec4 o_Specular; // 镜面反射

struct VertexOutput
{
	vec2 texCoord;
};

layout (location = 0) in VertexOutput Input;

layout (binding = 0) uniform sampler2D  g_PositionAndDepth;
layout (binding = 1) uniform sampler2D  g_Albedo;
layout (binding = 2) uniform sampler2D  g_Normal;
layout (binding = 3) uniform isampler2D g_LightingMode;

layout (binding = 4) uniform samplerCube u_PointLightDepthMap;
layout (binding = 5) uniform sampler2D   u_SpotLightDepthMap;

layout(std140, binding = 0) uniform CameraData
{
	mat4  u_CameraView;
	mat4  u_CameraProjection;
	vec3  u_CameraPosition;
	float u_CameraNearClip;
	float u_CameraFarClip;
};

struct PointLightData
{
    vec3  position;
	vec3  ambient;
	vec3  diffuse;
	vec3  specular;
	float constant;
    float linear;
    float quadratic;
	float radius;
	int   shadowEnabled;
};

layout (std430, binding = 3) readonly buffer PointLight
{
    PointLightData u_PointLights[];
};

struct SpotLightData
{
    vec3  position;
	vec3  direction;
	vec3  ambient;
	vec3  diffuse;
	vec3  specular;
	float constant;
    float linear;
    float quadratic;
	float cutoff;
	float outerCutoff;
	float radius;
	int   shadowEnabled;
};

layout (std430, binding = 4) readonly buffer SpotLight
{
	SpotLightData u_SpotLights[];
};

layout (std140, binding = 5) uniform Material
{
	float u_MaterialShininess;
};

layout (std140, binding = 6) uniform LightCount
{
	int u_DirectionalLightCount;
	int u_PointLightCount;
	int u_SpotLightCount;
};

layout(std140, binding = 10) uniform SpotLightShadowData
{
	mat4 u_SpotLightSpaceMatrix;
};

void main()
{
	int lightingMode = texture(g_LightingMode, Input.texCoord).r;
	if (lightingMode != 1)
	    discard;

	vec3  fragPosition     = texture(g_PositionAndDepth, Input.texCoord).rgb;
	vec3  materialNormal   = texture(g_Normal,           Input.texCoord).rgb;
	
	vec3 Ambient  = vec3(0.0f);
	vec3 Diffuse  = vec3(0.0f);
	vec3 Specular = vec3(0.0f);

	if(fragPosition == vec3(0.0f))
	    discard;

    vec3 fragToCameraDirection = normalize(u_CameraPosition - fragPosition);
    vec3 normal = normalize(materialNormal * 2.0 - 1.0); // [0, 1] -> [-1, 1]

    float shadow = 0.0f;

	// PointLight
	int i = 0;
	while (i < u_PointLightCount)
	{
	    PointLightData light = u_PointLights[i];
		
        vec3 fragToLight = light.position - fragPosition;
        vec3 fragToLightDirection = normalize(fragToLight);
		float diff = max(dot(normal, fragToLightDirection), 0.0f);
		
		vec3 halfwayDirection = normalize(fragToLightDirection + fragToCameraDirection);
		float spec = pow(max(dot(normal, halfwayDirection), 0.0f), u_MaterialShininess);
		
		float distance = length(fragToLight);
		float attenuation = 1.0f / (light.constant + light.linear * distance +  light.quadratic * (distance * distance));
		
	    vec3 ambient  = light.ambient;
		vec3 diffuse  = light.diffuse  * diff;
		vec3 specular = light.specular * spec;
	    ambient  *= attenuation;
		diffuse  *= attenuation;
		specular *= attenuation;

		if (light.shadowEnabled != 0)
		{
            float closestDepth = texture(u_PointLightDepthMap, -fragToLightDirection).r * light.radius;
            float currentDepth = distance;
			// 光照方向和法线夹角越大，偏移越大
            float bias = max(0.1f * (1.0f - dot(normal, fragToLightDirection)), 0.001f);
            shadow = (currentDepth - bias) > closestDepth ? 1.0f : 0.0f;
		}
		else
		{
		    shadow = 0.0f;
		}
		
		Ambient  += ambient;
		Diffuse  += (1.0f - shadow) * diffuse;
		Specular += (1.0f - shadow) * specular;

		i++;
	}
	
	// SpotLight
	i = 0;
	while (i < u_SpotLightCount)
	{
	    SpotLightData light = u_SpotLights[i];

		vec3 fragToLight = light.position - fragPosition;
        vec3 fragToLightDirection = normalize(fragToLight);
		float diff = max(dot(normal, fragToLightDirection), 0.0f);
		
		vec3 halfwayDirection = normalize(fragToLightDirection + fragToCameraDirection);
		float spec = pow(max(dot(normal, halfwayDirection), 0.0f), u_MaterialShininess);
		
		float distance    = length(fragToLight);
		float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
		
		// spotlight (soft edges)
		float theta = dot(-fragToLightDirection, normalize(light.direction)); 
        float epsilon = (light.cutoff - light.outerCutoff);
        float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0f, 1.0f);
		
		vec3 ambient  = light.ambient;
		vec3 diffuse  = light.diffuse  * diff;  
		vec3 specular = light.specular * spec;
        ambient  = ambient  * intensity * attenuation;
        diffuse  = diffuse  * intensity * attenuation;
        specular = specular * intensity * attenuation;
		
        if (light.shadowEnabled != 0)
		{
	        vec4 spotLightSpacePosition = u_SpotLightSpaceMatrix * vec4(fragPosition, 1.0f);
            vec3 projCoords = spotLightSpacePosition.xyz / spotLightSpacePosition.w;
            projCoords = projCoords * 0.5 + 0.5;
            //float closestDepth = texture(u_SpotLightDepthMap, projCoords.xy).r; 
            float currentDepth = projCoords.z;
	        float bias = max(0.0005f * (1.0f - dot(normal, -light.direction)), 0.0001f);
            //float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
		    
            shadow = 0.0f;
            vec2 texelSize = 1.0f / textureSize(u_SpotLightDepthMap, 0);
            for(int x = -1; x != 2; x++)
            {
                for(int y = -1; y != 2; y++)
                {
                    float pcfDepth = texture(u_SpotLightDepthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                    shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;        
                }    
            }
            shadow /= 9.0f;

		}
		else
		{
		    shadow = 0.0f;
		}

		Ambient  += ambient;
		Diffuse  += (1.0f - shadow) * diffuse;
		Specular += (1.0f - shadow) * specular;

		i++;
	}

	o_Ambient  = vec4(Ambient, 1.0f);
	o_Diffuse  = vec4(Diffuse, 1.0f);
	o_Specular = vec4(Specular, 1.0f);
}  