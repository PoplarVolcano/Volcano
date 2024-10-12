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

layout (binding = 0) uniform sampler2D  g_Position;
layout (binding = 1) uniform sampler2D  g_Normal;
layout (binding = 2) uniform sampler2D  g_Albedo;
layout (binding = 3) uniform sampler2D  g_RoughnessAO;
layout (binding = 4) uniform isampler2D g_EntityID;
layout (binding = 5) uniform sampler2D  u_SSAO;
					 
layout (binding = 6) uniform sampler2D  u_Lo;

layout (binding = 7) uniform samplerCube u_IrradianceMap;
layout (binding = 8) uniform samplerCube u_PrefilterMap;
layout (binding = 9) uniform sampler2D   u_BRDFLUT;

layout(std140, binding = 1) uniform CameraPosition
{
	vec3 u_CameraPosition;
};

const float PI = 3.14159265359;
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
    vec3  fragPosition = texture(g_Position, Input.TexCoords).rgb;

	if(fragPosition == vec3(0.0))
	    discard;

    vec3  albedo    = pow(texture(g_Albedo,  Input.TexCoords).rgb, vec3(2.2));
    float metallic  = texture(g_Albedo,      Input.TexCoords).a;
    float roughness = texture(g_RoughnessAO, Input.TexCoords).r;
    float ao        = texture(g_RoughnessAO, Input.TexCoords).g;
	float ambientOcclusion = texture(u_SSAO, Input.TexCoords).r;
    
    vec3 N = texture(g_Normal, Input.TexCoords).rgb * 2.0 - 1.0;// [-1, 1]
    vec3 V = normalize(u_CameraPosition - fragPosition);
    vec3 R = reflect(-V, N); 

    float NdotV = max(dot(N, V), 0.0);

    // 计算法向入射时的反射率；如果是介电质（如塑料）使用F0为0.04，如果它是金属，则使用反射率颜色作为F0（金属工作流）
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic); // x * (1 - a) + y * a
    
    vec3 Lo = texture(u_Lo, Input.TexCoords).rgb;

    // ambient lighting (we now use IBL as the ambient term)
    vec3 Fresnel = FresnelSchlickRoughness(NdotV, F0, roughness);

    vec3 kS = Fresnel;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 diffuse    = irradiance * albedo;

    // sample both the pre-filter map and the BRDFLUT and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(u_PrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(u_BRDFLUT, vec2(NdotV, roughness)).rg;
    vec3 specular = prefilteredColor * (Fresnel * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;

    //ambient *= ambientOcclusion;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 


    FragColor = vec4(color, 1.0);
	o_EntityID = texture(g_EntityID, Input.TexCoords).r;

	 // Check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));//转换为灰度来计算片段的亮度
	if(brightness > 1.0)
        BrightColor = FragColor;
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}  