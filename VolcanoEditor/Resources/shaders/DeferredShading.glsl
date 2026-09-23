#type vertex
#version 450 core
layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;

struct VertexOutput
{
	vec2 TexCoord;
};

layout (location = 0) out VertexOutput Output;

void main()
{
	Output.TexCoord = a_TexCoord;

	gl_Position = vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout (location = 0) out vec4 o_FragColor;
layout (location = 1) out vec4 o_BrightColor;

struct VertexOutput
{
	vec2 TexCoord;
};

layout (location = 0) in VertexOutput Input;

layout (binding = 0) uniform sampler2D  g_PositionAndDepth;
layout (binding = 1) uniform sampler2D  g_Albedo;
layout (binding = 2) uniform sampler2D  g_Normal;
layout (binding = 3) uniform sampler2D  g_RoughnessAO;
layout (binding = 4) uniform sampler2D  g_Emission;
layout (binding = 5) uniform sampler2D  u_SSAO;
layout (binding = 6) uniform isampler2D g_LightingMode;

layout (binding = 10) uniform sampler2D u_Ambient;
layout (binding = 11) uniform sampler2D u_Diffuse;
layout (binding = 12) uniform sampler2D u_Specular;
layout (binding = 13) uniform sampler2D u_L0;

layout (binding = 14) uniform samplerCube u_IrradianceMap;
layout (binding = 15) uniform samplerCube u_PrefilterMap;
layout (binding = 16) uniform sampler2D   u_BRDFLUT;

layout (binding = 20) uniform samplerCube u_SkyboxMap;

layout(std140, binding = 0) uniform CameraData
{
	mat4  u_CameraView;
	mat4  u_CameraProjection;
	vec3  u_CameraPosition;
	float u_CameraNearClip;
	float u_CameraFarClip;
};

layout (std140, binding = 18) uniform SSAO
{
    int   u_KernelSize;
    float u_Radius;
    float u_Bias;
    float u_Power;
    int   u_SSAOEnabled;
};

layout (std140, binding = 19) uniform IrradianceEnabled
{
    bool u_IrradianceEnabled;
};

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


// ----------------------------------------------------------------------------------------------
// 环境光来自半球内围绕法线 N 的所有方向，因此没有一个确定的半向量来计算菲涅耳效应。
// 为了模拟菲涅耳效应，我们用法线和视线之间的夹角计算菲涅耳系数。
// 我们以受粗糙度影响的微表面半向量作为菲涅耳公式的输入，
// 但我们目前没有考虑任何粗糙度，表面的反射率总是会相对较高。
// 间接光和直射光遵循相同的属性，因此我们期望较粗糙的表面在边缘反射较弱。
// 通过在 Sébastien Lagarde(https://seblagarde.wordpress.com/2011/08/17/hello-world/) 
// 提出的 Fresnel-Schlick 方程中加入粗糙度项来缓解这个问题
// ----------------------------------------------------------------------------------------------
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------------------------

void main()
{
	
	vec3  fragPosition = texture(g_PositionAndDepth, Input.TexCoord).rgb;
	float depth        = texture(g_PositionAndDepth, Input.TexCoord).a;
	
	if(fragPosition == vec3(0.0) && depth == 0)
	    discard;
	
	int lightingMode = texture(g_LightingMode, Input.TexCoord).r; 

	vec3 finalColor;
	
	if (lightingMode == 0)
	{
	    vec3 materialDiffuse = texture(g_Albedo, Input.TexCoord).rgb;
	    finalColor = materialDiffuse;
	}
	else if (lightingMode == 2) // PBR
	{
	
	    vec3  materialAlbedo    = texture(g_Albedo,      Input.TexCoord).rgb;
	    float materialMetallic  = texture(g_Albedo,      Input.TexCoord).a;
	    vec3  materialNormal    = texture(g_Normal,      Input.TexCoord).rgb;  // [0, 1]
	    float materialRoughness = texture(g_RoughnessAO, Input.TexCoord).r;
	    float materialAO        = texture(g_RoughnessAO, Input.TexCoord).g;
        vec3  L0                = texture(u_L0,          Input.TexCoord).rgb;

		vec3 ambient = vec3(0.0);
		if (u_IrradianceEnabled)
		{
            vec3 normal = normalize(materialNormal * 2.0 - 1.0); // [0, 1] -> [-1, 1]
            vec3 fragToCameraDirection = normalize(u_CameraPosition - fragPosition);
            vec3 R = reflect(-fragToCameraDirection, normal); 
	        //o_FragColor = vec4(normal, 0);
			//return;
            float NdotV = max(dot(normal, fragToCameraDirection), 0.0);
		    
            vec3 F0 = vec3(0.04); 
            F0 = mix(F0, materialAlbedo, materialMetallic); // x * (1 - a) + y * a
    	    
            vec3 Fresnel = FresnelSchlickRoughness(NdotV, F0, materialRoughness);
		    
            vec3 kD = vec3(1.0) - Fresnel;
            kD = kD * (1.0 - materialMetallic);

            vec3 irradiance = texture(u_IrradianceMap, normal).rgb;
            vec3 diffuse    = irradiance * materialAlbedo;
		    
		    //对预滤波贴图和BRDFLUT进行采样，并根据Split-Sum近似法将它们结合在一起，以获得IBL镜面反射部分
            const float MAX_REFLECTION_LOD = 4.0;
			vec3 prefilteredColor = textureLod(u_PrefilterMap, R, materialRoughness * MAX_REFLECTION_LOD).rgb;    
			vec2 BRDF  = texture(u_BRDFLUT, vec2(NdotV, materialRoughness)).rg;
            vec3 specular = prefilteredColor * (Fresnel * BRDF.x + BRDF.y);
		    
            ambient = (kD * diffuse + specular) * materialAO;
		}

	    finalColor = ambient + L0;
	}
	else if (lightingMode == 1) // 传统光照
	{

	    vec3  materialDiffuse   = texture(g_Albedo,   Input.TexCoord).rgb;
	    float materialSpecular  = texture(g_Albedo,   Input.TexCoord).a;
	    vec3  materialNormal    = texture(g_Normal,   Input.TexCoord).rgb;  // [0, 1]
	    vec3  materialEmission  = texture(g_Emission, Input.TexCoord).rgb;
	    float ambientOcclusion  = texture(u_SSAO,     Input.TexCoord).r;

	    vec3 ambient   = texture(u_Ambient,  Input.TexCoord).rgb;
	    vec3 diffuse   = texture(u_Diffuse,  Input.TexCoord).rgb;
	    vec3 specular  = texture(u_Specular, Input.TexCoord).rgb;

        vec3 I = normalize(fragPosition - u_CameraPosition);
        vec3 normal = normalize(materialNormal * 2.0 - 1.0); // [0, 1] -> [-1, 1]
	    
	    // reflect(vec3 I, vec3 N)：
	    // 计算镜面反射方向，根据入射方向和表面法线，计算出光线的反射方向（遵循反射定律：入射角等于反射角）。
	    // 参数	类型	说明
        // I	vec3	入射方向（光线射入的方向）。必须归一化。
        //              注意：GLSL 中 I 通常定义为指向入射光线来源的方向，即从表面指向光源/观察者的反方向。
	    //              在计算折射时，通常传入 -viewDir（即从表面指向眼睛的方向取反）。
        // N	vec3	表面法线方向。必须归一化。
	    // 
	    // 计算 R = I - 2.0 * dot(N, I) * N;
	    //
	    vec3 Reflect = reflect(I, normal);
	    
	    // 材质     折射率
        // 空气     1.00
        // 水       1.33
        // 冰       1.309
        // 玻璃     1.52
        // 钻石     2.42
	    //
	    // refract(vec3 I, vec3 N, float eta)：
	    // 计算光线折射方向，基于斯涅尔定律（Snell's Law），模拟光穿过不同介质（如空气进入水或玻璃）时发生的“弯折”现象
	    //
	    // 参数	类型	说明
        // I	vec3	入射方向（光线射入的方向）。必须归一化。
        //              注意：GLSL 中 I 通常定义为指向入射光线来源的方向，即从表面指向光源/观察者的反方向。
	    //              在计算折射时，通常传入 -viewDir（即从表面指向眼睛的方向取反）。
        // N	vec3	表面法线方向。必须归一化。
        // eta	float	折射率比率（eta = n1 / n2）。
        //              - n1：入射介质（光线来自的一侧）的折射率（如空气 ≈ 1.0）。
        //              - n2：折射介质（光线进入的一侧）的折射率（如水 ≈ 1.33，玻璃 ≈ 1.5）。
        //              例如：空气→玻璃时 eta = 1.0 / 1.5 ≈ 0.666；玻璃→空气时 eta = 1.5 / 1.0 = 1.5。
        //
	    // 计算 k = 1.0 - eta² * (1.0 - dot(N, I)²)。
	    // 如果 k < 0，发生全内反射（光无法穿透，全部反射回原介质），函数返回 (0,0,0)（或可配合 reflect 做回退）。
	    // 否则，折射方向为：
	    // refracted = eta * I - (eta * dot(N, I) + sqrt(k)) * N
	    
	    // 注：折射率没有引入金属折射率的消光部分，所以目前金属也会发生折射。
        float ratio = 1.00 / 1.00;
        vec3 Refract = refract(I, normal, ratio);
	    
	    ambient  *= materialDiffuse;
	    diffuse  *= materialDiffuse;
	    specular = (specular + texture(u_SkyboxMap, Reflect).rgb + texture(u_SkyboxMap, Refract).rgb) * materialSpecular;
	    
	    if (u_SSAOEnabled != 0)
	        ambient *= ambientOcclusion;
	    
	    finalColor = ambient + diffuse + specular + materialEmission;
	}
	else
	{
	    discard;
	}

    o_FragColor = vec4(finalColor, 1.0);

	// 检查片段输出是否高于阈值，如果是，则作为亮度颜色o_BrightColor输出
    float brightness = dot(o_FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));//转换为灰度来计算片段的亮度
	if(brightness > 1.0)
        o_BrightColor = o_FragColor;
    else
        o_BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}