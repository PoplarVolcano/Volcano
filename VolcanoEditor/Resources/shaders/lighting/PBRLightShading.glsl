#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;

struct VertexOutput
{
	vec2 texCoord;
};

layout (location = 0) out VertexOutput Output;

void main()
{
    Output.texCoord = a_TexCoord;

    gl_Position = vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout (location = 0) out vec4 o_L0;

struct VertexOutput
{
	vec2 texCoord;
};

layout (location = 0) in VertexOutput Input;

layout (binding = 0) uniform sampler2D  g_PositionDepth;
layout (binding = 1) uniform sampler2D  g_Albedo;
layout (binding = 2) uniform sampler2D  g_Normal;
layout (binding = 3) uniform sampler2D  g_RoughnessAO;
layout (binding = 4) uniform isampler2D g_LightingMode;

layout (binding = 5) uniform samplerCube u_PointLightDepthMap;
layout (binding = 6) uniform sampler2D   u_SpotLightDepthMap;

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

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------------------------
// 法线分布函数（NDF）——DistributionGGX
// 公式：Trowbridge-Reitz GGX
// 作用：从统计学上近似地表示与半程向量H取向一致的微平面的比率，决定高光的“分布”形状（粗糙度越大，高光越模糊）
// 实现：a = roughness²，计算 a² / (π * (NdotH² * (a² - 1) + 1)²)。
// ----------------------------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float numerator = a2;
    float denominator = (NdotH2 * (a2 - 1.0) + 1.0);
    denominator = PI * denominator * denominator;

    return numerator / denominator;
}
// ----------------------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------------------
// 几何函数（Geometry）—— GeometrySchlickGGX 和 GeometrySmith
// 公式：使用 GeometrySchlickGGX 近似计算几何结果，史密斯法(Smith’s method)混合两个方向的几何结果（就是直接相乘）。
// 作用：从统计学上近似的求得了微平面间相互遮蔽的比率，这种相互遮蔽会损耗光线的能量。
//       粗糙度较高的表面，其微平面间相互遮蔽的概率也较高。
//       k是粗糙度的重映射，取决于要用的是针对直接光照还是针对IBL光照的几何函数。
//       为了有效的估算几何部分，需要将观察方向（几何遮蔽(Geometry Obstruction)）和光线方向向量（几何阴影(Geometry Shadowing)）都考虑进去。
//       影响高光的强度。
// 实现：kdirect = (roughness + 1)² / 8，kIBL = roughness² / 2
//       G = NdotV / (NdotV*(1-k) + k)
//       再将视线和光线这两个方向的近似结果相乘。
// ----------------------------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float numerator   = NdotV;
    float denominator = NdotV * (1.0 - k) + k;

    return numerator / denominator;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------------------
// 菲涅尔效应（Fresnel）—— FresnelSchlick
// 公式：FresnelSchlick 近似法。
// 作用：计算反射率随视角变化（掠射角时反射增强）。
// 实现：F0 + (1 - F0) * pow(1 - cosTheta, 5)
// ----------------------------------------------------------------------------------------------
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------------------------


void main()
{		
	int lightingMode = texture(g_LightingMode, Input.texCoord).r;
	if (lightingMode != 2)
	    discard;

    vec3  fragPosition      = texture(g_PositionDepth, Input.texCoord).rgb; // 世界空间位置
    vec3  materialAlbedo    = texture(g_Albedo,        Input.texCoord).rgb; // 漫反射颜色（Albedo）
    float materialMetallic  = texture(g_Albedo,        Input.texCoord).a;   // 金属度（Metallic）
    vec3  materialNormal    = texture(g_Normal,        Input.texCoord).rgb; // 世界空间法线（编码为 [0,1]，需解码）
    float materialRoughness = texture(g_RoughnessAO,   Input.texCoord).r;   // 粗糙度（Roughness）
    float materialAO        = texture(g_RoughnessAO,   Input.texCoord).g;   // 环境遮蔽（AO）
    
    vec3 normal = normalize(materialNormal * 2.0 - 1.0); // [0, 1] -> [-1, 1]
    vec3 fragToCameraDirection = normalize(u_CameraPosition - fragPosition);
    vec3 reflect = reflect(-fragToCameraDirection, normal); 
    
    float NdotV = max(dot(normal, fragToCameraDirection), 0.0);
    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, materialAlbedo, materialMetallic);
    
    vec3 L0 = vec3(0.0f);

	int i = 0;
	while (i < u_PointLightCount)
    {
	    PointLightData light = u_PointLights[i];
		
        vec3 lightColor = light.ambient + light.diffuse + light.specular;
    
        vec3 fragToLight = light.position - fragPosition;
        vec3 fragToLightDirection = normalize(fragToLight);
        vec3 halfwayDirection = normalize(fragToCameraDirection + fragToLightDirection);
        
        float distance = length(fragToLight);
        float attenuation = 1.0 / (distance * distance);
        vec3  radiance = lightColor * attenuation;
        
        float NDF = DistributionGGX(normal, halfwayDirection, materialRoughness);   
        float G   = GeometrySmith(normal, fragToCameraDirection, fragToLightDirection, materialRoughness);      
        vec3  F   = FresnelSchlick(max(dot(halfwayDirection, fragToCameraDirection), 0.0), F0);

        float NdotL = max(dot(normal, fragToLightDirection), 0.0);
        
        vec3  numerator    = NDF * G * F; 
        float denominator = 4.0 * NdotV * NdotL + 0.0001;
        vec3  specular = numerator / denominator;
        
        vec3 kD = vec3(1.0) - F;
        kD *= 1.0 - materialMetallic;
        
        float shadow = 0.0f;
        if (light.shadowEnabled != 0)
        {
            float closestDepth = texture(u_PointLightDepthMap, fragToLightDirection).r * light.radius;
            float currentDepth = distance;
            float bias = max(0.5f * (1.0f - dot(normal, -fragToLightDirection)), 0.01f);
            shadow = (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
        }

        L0 += (1.0 - shadow) * (kD * materialAlbedo / PI + specular) * radiance * NdotL;
        
        i++;
    }
    
	i = 0;
	while (i < u_SpotLightCount)
    {
	    SpotLightData light = u_SpotLights[i];

        vec3 lightColor = light.ambient + light.diffuse + light.specular;
        
        vec3 fragToLight = light.position - fragPosition;
        vec3 fragToLightDirection = normalize(fragToLight);
        vec3 halfwayDirection = normalize(fragToCameraDirection + fragToLightDirection);
        
        float distance = length(fragToLight);
        float attenuation = 1.0 / (distance * distance);
        vec3  radiance = lightColor * attenuation;
        
        float NDF = DistributionGGX(normal, halfwayDirection, materialRoughness);   
        float G   = GeometrySmith(normal, fragToCameraDirection, fragToLightDirection, materialRoughness);      
        vec3  F   = FresnelSchlick(max(dot(halfwayDirection, fragToCameraDirection), 0.0), F0);
 
        float NdotL = max(dot(normal, fragToLightDirection), 0.0);
        
        vec3  numerator    = NDF * G * F; 
        float denominator = 4.0 * NdotV * NdotL + 0.0001;
        vec3  specular = numerator / denominator;
        
        vec3 kD = vec3(1.0) - F;
        kD *= 1.0 - materialMetallic;
        
	    // spotlight (soft edges)
	    float theta = dot(-fragToLightDirection, normalize(light.direction)); 
        float epsilon = (light.cutoff - light.outerCutoff);
        float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
        
        float shadow = 0.0f;
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
        }

        L0 += intensity * (1.0 - shadow) * (kD * materialAlbedo / PI + specular) * radiance * NdotL;    
        
        i++;
    }
    
    o_L0 = vec4(L0, 1.0);
}