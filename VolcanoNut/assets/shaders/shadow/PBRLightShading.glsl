#type vertex
#version 420 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;

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
#version 420 core

layout (location = 0) out vec4 o_Lo;

struct VertexOutput
{
	vec2 TexCoords;
};

layout (location = 0) in VertexOutput Input;

layout (binding = 0) uniform sampler2D g_Position;
layout (binding = 1) uniform sampler2D g_Normal;
layout (binding = 2) uniform sampler2D g_Albedo;
layout (binding = 3) uniform sampler2D g_RoughnessAO;
layout (binding = 4) uniform sampler2D u_Lo;
layout (binding = 5) uniform sampler2D u_DirectionalDepthMap;
layout (binding = 6) uniform samplerCube u_PointDepthMap;
layout (binding = 7) uniform sampler2D u_SpotDepthMap;

layout(std140, binding = 1) uniform CameraPosition
{
	vec3 u_CameraPosition;
};

struct DirectionalLightData
{
    vec3 Position;
	vec3 Direction;
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};

layout (std140, binding = 2) uniform DirectionalLight
{
    DirectionalLightData u_DirectionalLight;
};

struct PointLightData
{
    vec3  Position;
	vec3  Ambient;
	vec3  Diffuse;
	vec3  Specular;
	float Constant;
    float Linear;
    float Quadratic;
};

layout (std140, binding = 3) uniform PointLight
{
    PointLightData u_PointLight;
};

struct SpotLightData
{
    vec3  Position;
	vec3  Direction;
	vec3  Ambient;
	vec3  Diffuse;
	vec3  Specular;
	float Constant;
    float Linear;
    float Quadratic;
	float CutOff;
	float OuterCutOff;
};

layout (std140, binding = 4) uniform SpotLight
{
	SpotLightData u_SpotLight;
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

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)  //Trowbridge-Reitz GGX, 法向量N，半向量H
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0; // 直接光照

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
void main()
{		
    vec3  fragPosition = texture(g_Position, Input.TexCoords).rgb;
    vec3  albedo    = pow(texture(g_Albedo,  Input.TexCoords).rgb, vec3(2.2));
    float metallic  = texture(g_Albedo,      Input.TexCoords).a;
    float roughness = texture(g_RoughnessAO, Input.TexCoords).r;
    float ao        = texture(g_RoughnessAO, Input.TexCoords).g;

    vec3 N = texture(g_Normal, Input.TexCoords).rgb * 2.0 - 1.0;
    vec3 V = normalize(u_CameraPosition - fragPosition);
    vec3 R = reflect(-V, N); 

    float NdotV = max(dot(N, V), 0.0);

    // 计算法向入射时的反射率；如果是介电质（如塑料）使用F0为0.04，如果它是金属，则使用反射率颜色作为F0（金属工作流）
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic); // x * (1 - a) + y * a


    // 反射率方程 reflectance equation
    vec3 Lo = texture(u_Lo, Input.TexCoords).rgb;
    
    vec3 directionalLightPosition = u_DirectionalLight.Position;
    vec3 directionalLightColor = u_DirectionalLight.Ambient + u_DirectionalLight.Diffuse + u_DirectionalLight.Specular;

    vec3 pointLightPosition = u_PointLight.Position;
    vec3 pointLightColor = u_PointLight.Ambient + u_PointLight.Diffuse + u_PointLight.Specular;
    
    vec3 spotLightPosition = u_SpotLight.Position;
    vec3 spotLightColor = u_SpotLight.Ambient + u_SpotLight.Diffuse + u_SpotLight.Specular;
    
	if (u_DirectionalLight.Direction != vec3(0.0))
    {
        // calculate per-light radiance
        vec3 L = normalize(directionalLightPosition - fragPosition);
        vec3 H = normalize(V + L);
        //float distance = length(directionalLightPosition - fragPosition);
        //float attenuation = 1.0 / (distance * distance);
        vec3 radiance = directionalLightColor;// * attenuation;
        
        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);
           
        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);
        
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * NdotV * NdotL + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        // 将kD乘以逆金属度，使得只有非金属具有漫射光，如果为部分金属应用线性混合光（纯金属没有漫射光）。
        kD *= 1.0 - metallic;
        
	    vec4 lightSpacePosition = u_DirectionalLightSpace * vec4(fragPosition, 1.0);
        vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;
        projCoords = projCoords * 0.5 + 0.5;
        float closestDepth = texture(u_DirectionalDepthMap, projCoords.xy).r; 
        float currentDepth = projCoords.z;
	    float bias = max(0.005 * (1.0 - dot(N, -u_DirectionalLight.Direction)), 0.0005);
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	    if(projCoords.z > 1.0)
            shadow = 0.0;

        // add to outgoing radiance Lo
        Lo += (1.0 - shadow) * (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    
	if (u_PointLight.Position != vec3(0.0))
    {
        // calculate per-light radiance
        vec3 L = normalize(pointLightPosition - fragPosition);
        vec3 H = normalize(V + L);
        float distance = length(pointLightPosition - fragPosition);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = pointLightColor * attenuation;
        
        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);
           
        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);
        
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * NdotV * NdotL + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        // 将kD乘以逆金属度，使得只有非金属具有漫射光，如果为部分金属应用线性混合光（纯金属没有漫射光）。
        kD *= 1.0 - metallic;
        
	    vec3 fragToLight = fragPosition - u_PointLight.Position;
        float currentDepth = length(fragToLight);
        float closestDepth = texture(u_PointDepthMap, fragToLight).r;
        closestDepth *= u_PointFarPlane;
        float bias = 0.05;
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

        // add to outgoing radiance Lo
        Lo += (1.0 - shadow) * (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    
	if (u_SpotLight.Direction != vec3(0.0))
    {
        // calculate per-light radiance
        vec3 L = normalize(spotLightPosition - fragPosition);
        vec3 H = normalize(V + L);
        float distance = length(spotLightPosition - fragPosition);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = spotLightColor * attenuation;
        
        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);
           
        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);
        
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * NdotV * NdotL + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        // 将kD乘以逆金属度，使得只有非金属具有漫射光，如果为部分金属应用线性混合光（纯金属没有漫射光）。
        kD *= 1.0 - metallic;
        
		// spotlight (soft edges)
		float theta = dot(-L, normalize(u_SpotLight.Direction)); 
        float epsilon = (u_SpotLight.CutOff - u_SpotLight.OuterCutOff);
        float intensity = clamp((theta - u_SpotLight.OuterCutOff) / epsilon, 0.0, 1.0);

	    vec4 spotLightSpacePosition = u_SpotLightSpace * vec4(fragPosition, 1.0);
        vec3 projCoords = spotLightSpacePosition.xyz / spotLightSpacePosition.w;
        projCoords = projCoords * 0.5 + 0.5;
        float closestDepth = texture(u_SpotDepthMap, projCoords.xy).r; 
        float currentDepth = projCoords.z;
	    float bias = max(0.005 * (1.0 - dot(N, -u_SpotLight.Direction)), 0.0005);
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
        
        // add to outgoing radiance Lo
        Lo += intensity * (1.0 - shadow) * (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    o_Lo = vec4(Lo, 1.0);


}