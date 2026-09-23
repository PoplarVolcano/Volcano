#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

layout(std140, binding = 0) uniform CameraData
{
	mat4  u_CameraView;
	mat4  u_CameraProjection;
	vec3  u_CameraPosition;
	float u_CameraNearClip;
	float u_CameraFarClip;
};

struct InstanceData
{            
	mat4 transform;
};

layout (std430, binding = 21) readonly buffer InstanceDataBuffer
{
    InstanceData u_InstanceData[];
};

layout (location = 0) out flat int v_LightIndex;

void main()
{
    InstanceData instanceData = u_InstanceData[gl_InstanceIndex];

	v_LightIndex = gl_InstanceIndex;
    gl_Position = u_CameraProjection * u_CameraView * instanceData.transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout (location = 0) out vec4 o_Ambient;  // 环境光
layout (location = 1) out vec4 o_Diffuse;  // 漫反射
layout (location = 2) out vec4 o_Specular; // 镜面反射

layout (binding = 0) uniform sampler2D g_PositionAndDepth;
layout (binding = 1) uniform sampler2D g_Normal;
layout (binding = 2) uniform isampler2D g_LightingMode;

layout (binding = 3) uniform sampler2D u_SpotLightDepthMap;

layout(std140, binding = 0) uniform CameraData
{
	mat4  u_CameraView;
	mat4  u_CameraProjection;
	vec3  u_CameraPosition;
	float u_CameraNearClip;
	float u_CameraFarClip;
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

layout(std140, binding = 10) uniform SpotLightShadowData
{
    mat4 u_SpotLightSpaceMatrix;
};

layout (location = 0) in flat int v_LightIndex;

void main()
{
	ivec2 texCoord = ivec2(gl_FragCoord.xy);
    
	int lightingMode = texelFetch(g_LightingMode, texCoord, 0).r;
	if (lightingMode != 1)
	    discard;

	SpotLightData light = u_SpotLights[v_LightIndex];

	vec3  fragPosition = texelFetch(g_PositionAndDepth, texCoord, 0).rgb;
	vec3  materialNormal = texelFetch(g_Normal, texCoord, 0).rgb;
    
    vec3 normal = normalize(materialNormal * 2.0 - 1.0); // [0, 1] -> [-1, 1]
    
    vec3 fragToLight = light.position - fragPosition;
    vec3 fragToLightDirection = normalize(fragToLight);
    
    float diff = max(dot(normal, fragToLightDirection), 0.0);
    
    vec3 fragToCamera = u_CameraPosition - fragPosition;
    vec3 fragToCameraDirection = normalize(fragToCamera);
	vec3 halfwayDirection = normalize(fragToLightDirection + fragToCameraDirection);
	float spec = pow(max(dot(normal, halfwayDirection), 0.0f), u_MaterialShininess);
		
    float distance = length(fragToLight);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    
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
	    
        float shadow = 0.0f;
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

        diffuse  *= (1.0 - shadow);
        specular *= (1.0 - shadow);
    }
    

    o_Ambient  = vec4(ambient, 1.0);
    o_Diffuse  = vec4(diffuse, 1.0);
    o_Specular = vec4(specular, 1.0);
}