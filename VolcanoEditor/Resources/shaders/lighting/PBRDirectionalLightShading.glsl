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

layout (binding = 5) uniform sampler2D u_DirectionalLightDepthMap;

layout(std140, binding = 0) uniform CameraData
{
	mat4  u_CameraView;
	mat4  u_CameraProjection;
	vec3  u_CameraPosition;
	float u_CameraNearClip;
	float u_CameraFarClip;
};

struct DirectionalLightData
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	int  shadowEnabled;
};

layout (std430, binding = 2) readonly buffer DirectionalLight
{
    DirectionalLightData u_DirectionalLight;
};

layout(std140, binding = 8) uniform DirectionalLightShadowData
{
	mat4 u_DirectionalLightSpaceMatrix;
};

layout (std140, binding = 19) uniform IrradianceEnabled
{
    bool u_IrradianceEnabled;
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
    
    // F0：基础反射率，用于菲涅尔方程计算反射率
    // 法向入射(0°入射角)时的反射率，或者说是直接(垂直)观察表面时有多少光线会被反射
	// 如果是介电质（如塑料）使用F0为0.04，如果它是金属，则使用反射率颜色作为F0（金属工作流）
	// 因为金属表面会吸收所有折射光线而没有漫反射，所以我们可以直接使用表面颜色纹理来作为它们的基础反射率。lbedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, materialAlbedo, materialMetallic); // mix(x,y,a) => x * (1 - a) + y * a

    vec3 lightColor = u_DirectionalLight.ambient + u_DirectionalLight.diffuse + u_DirectionalLight.specular;
    
    // 计算每束光的辐射度 radiance
    vec3 fragToLightDirection = normalize(-u_DirectionalLight.direction);
    vec3 halfwayDirection = normalize(fragToCameraDirection + fragToLightDirection);
    vec3 radiance = lightColor;
    
    // BRDF：双向反射分布函数
    // Cook-Torrance BRDF（fr）模型兼有漫反射和镜面反射两个部分
    // fr = kd * flambert + ks * fcook-torrance
    // kd：入射光线中被折射部分的能量所占的比率
    // ks：入射光线中被反射部分的能量所占的比率，也就是反射率，同fcook-torrance的F（菲涅尔方程）
    // flambert：漫反射部分，它被称为Lambertian漫反射，这和我们之前在漫反射着色中使用的常数因子类似
    //     flambert = c / π
    //     c表示表面颜色（albedo）。除以π是为了对漫反射光进行标准化，因为含有BRDF的反射率L0的积分方程是受π影响的
    // 
    // fcook-torrance：镜面反射部分。包含三个函数，此外分母部分还有一个标准化因子。
    //     fcook-torrance = DFG / 4 * NdotV * NdotL
    //     D：法线分布函数NDF(Normal Distribution Function)
    //     F：菲涅尔方程(Fresnel Rquation)
    //     G：几何函数(Geometry Function)
    //
    // fr = kd * (c / π) + ks * (DFG / 4 * NdotV * NdotL)
    // 注：ks用于控制漫反射衰减的系数。因为能量守恒 kd = 1 - ks。
    //     但因为反射率由F（菲涅尔方程）结算了，ks这个系数没用上，kd跟着F走，ks直接按1算，
    //     所以在实践中 kd = 1 - F， ks = 1，
    //     公式简化为 fr = (1 - F) * (c / π) + 1 * (DFG / 4 * NdotV * NdotL)

    float NDF = DistributionGGX(normal, halfwayDirection, materialRoughness);   
    float G   = GeometrySmith(normal, fragToCameraDirection, fragToLightDirection, materialRoughness);

    // FresnelSchlickRoughness(NdotV, F0, materialRoughness);会出现奇怪的黑斑，暂时不用      
    vec3  F   = FresnelSchlick(max(dot(halfwayDirection, fragToCameraDirection), 0.0), F0);

    float NdotL = max(dot(normal, fragToLightDirection), 0.0);
    
    vec3  numerator   = NDF * G * F; 
    float denominator = 4.0 * NdotV * NdotL + 0.0001; // + 0.0001 避免除零错误
    vec3  specular    = numerator / denominator;
    
    // vec3 kS = vec3(1.0);
    vec3 kD = vec3(1.0) - F;
    // 纯金属没有漫射光，将kD乘以逆金属度，使得只有非金属具有漫射光，部分金属应用线性混合光。
    kD *= 1.0 - materialMetallic;

    float shadow = 0.0f;
    if (u_DirectionalLight.shadowEnabled != 0)
    {
	    vec4 lightSpaceFragPosition = u_DirectionalLightSpaceMatrix * vec4(fragPosition, 1.0f);
		
		vec3 projCoords = lightSpaceFragPosition.xyz / lightSpaceFragPosition.w;
        projCoords = projCoords * 0.5 + 0.5;
		
        //float closestDepth = texture(u_DirectionalLightDepthMap, projCoords.xy).r; 
        float currentDepth = projCoords.z;
	    float bias = max(0.005f * (1.0f - dot(normal, -u_DirectionalLight.direction)), 0.0005f);
        //float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
		
        float shadow = 0.0f;
        vec2 texelSize = 1.0f / textureSize(u_DirectionalLightDepthMap, 0);
        for(int x = -1; x != 2; x++)
        {
            for(int y = -1; y != 2; y++)
            {
                float pcfDepth = texture(u_DirectionalLightDepthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;        
            }    
        }
        shadow /= 9.0f;
		
	    if(projCoords.z > 1.0f)
            shadow = 0.0f;
		
    }

    // BlinnPhong的diff(NdotL)转变成了L0公式最后的n·widwi
    // 公式：L0 = ( kD * (c / π) + (DFG / 4 * NdotV * NdotL) ) * radiance * NdotL;
    vec3 L0 = (1.0 - shadow) * (kD * materialAlbedo / PI + specular) * radiance * NdotL;
    
    o_L0 = vec4(L0, 1.0);

}