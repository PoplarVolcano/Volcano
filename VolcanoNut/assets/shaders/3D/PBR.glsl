#type vertex
#version 420 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;
layout (location = 2) in vec3 a_Normal;
layout (location = 3) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	// projection * view
	mat4 u_ViewProjection;
};

struct VertexOutput
{
    vec3 FragPosition;
	vec2 TexCoords;
    vec3 Normal;
};

layout (location = 0) out flat int v_EntityID;
layout (location = 1) out VertexOutput Output;

void main()
{
	v_EntityID          = a_EntityID;
    Output.FragPosition = a_Position;
    Output.TexCoords    = a_TexCoords;
    Output.Normal       = a_Normal;   

    gl_Position =  u_ViewProjection * vec4(a_Position, 1.0);
}


#type fragment
#version 420 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out int o_EntityID;
layout (location = 2) out vec4 BrightColor;

struct VertexOutput
{
    vec3 FragPosition;
	vec2 TexCoords;
    vec3 Normal;
};

layout (location = 0) in flat int v_EntityID;
layout (location = 1) in VertexOutput Input;

// material parameters
layout (binding = 1) uniform sampler2D u_Albedo;
layout (binding = 2) uniform sampler2D u_Normal;
layout (binding = 3) uniform sampler2D u_Metallic;
layout (binding = 4) uniform sampler2D u_Roughness;
layout (binding = 5) uniform sampler2D u_AO;

layout (binding = 6) uniform samplerCube u_IrradianceMap;
layout (binding = 7) uniform samplerCube u_PrefilterMap;
layout (binding = 25) uniform sampler2D u_BRDFLUT;

layout(std140, binding = 1) uniform CameraPosition
{
	vec3 u_CameraPosition;
};

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// 获取切线法线转换世界空间以简化PBR代码的简单技巧。 
// 您通常希望以通常的方式进行法线映射以提高性能
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// you generally want to do normal mapping the usual way;
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(u_Normal, Input.TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(Input.FragPosition);
    vec3 Q2  = dFdy(Input.FragPosition);
    vec2 st1 = dFdx(Input.TexCoords);
    vec2 st2 = dFdy(Input.TexCoords);

    vec3 N   = normalize(Input.Normal);
    vec3 T  = normalize(Q1 * st2.t - Q2 * st1.t); // st2.t => st2.y
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
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
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   
// ----------------------------------------------------------------------------
void main()
{		
    vec3 albedo     = pow(texture(u_Albedo, Input.TexCoords).rgb, vec3(2.2));
    float metallic  = texture(u_Metallic,  Input.TexCoords).r;
    float roughness = texture(u_Roughness, Input.TexCoords).r;
    float ao        = texture(u_AO,        Input.TexCoords).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(u_CameraPosition - Input.FragPosition);
    vec3 R = reflect(-V, N); 

    float NdotV = max(dot(N, V), 0.0);

    // 计算法向入射时的反射率；如果是介电质（如塑料）使用F0为0.04，如果它是金属，则使用反射率颜色作为F0（金属工作流）
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic); // x * (1 - a) + y * a

    // 反射率方程 reflectance equation
    vec3 Lo = vec3(0.0);
    const vec3 lightPositions[4] = vec3[4](
        vec3(-1.0,-1.0,  1.0), 
        vec3( 1.0,-1.0,  1.0), 
        vec3( 1.0, 1.0,  1.0), 
        vec3(-1.0, 1.0,  1.0)
        );
    const vec3 lightColors[4] = vec3[4](
        vec3(15.0, 15.0, 15.0), 
        vec3(15.0, 15.0, 15.0), 
        vec3(15.0, 15.0, 15.0), 
        vec3(15.0, 15.0, 15.0)
        );
    for(int i = 0; i < 4; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - Input.FragPosition);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - Input.FragPosition);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

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
   

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    
    // ambient lighting (we now use IBL as the ambient term)
    vec3 Fresnel = FresnelSchlickRoughness(NdotV, F0, roughness);

    vec3 kS = Fresnel;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 diffuse    = irradiance * albedo;

    // sample both the pre-filter map and the BRDFLUT and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(u_PrefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(u_BRDFLUT, vec2(NdotV, roughness)).rg;
    vec3 specular = prefilteredColor * (Fresnel * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
	o_EntityID = v_EntityID;
}