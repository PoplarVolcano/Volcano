#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

layout (location = 0) out vec3 v_FragPosition;

layout (std140, binding = 16) uniform ViewProjection
{
    mat4 u_ViewProjection;
};

void main()
{
    v_FragPosition = a_Position;
    gl_Position =  u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core
layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec3 v_FragPosition;

layout (binding = 0) uniform samplerCube u_EnvironmentMap;

layout (std140, binding = 17) uniform Roughness
{
    float u_Roughness;
};

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// 法线N，与某些（半程）向量H取向一致的微平面的比率。
float DistributionGGX(vec3 N, vec3 H, float roughness) //Trowbridge-Reitz GGX
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
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// efficient VanDerCorput calculation.
float RadicalInverse_VdC(uint bits)  // Van Der Corput 序列，把十进制数字的二进制表示镜像翻转到小数点右边，例123=>0.321
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // return (x / 0x100000000);
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)  // 低差异序列,i∈[0,N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}
// ----------------------------------------------------------------------------
// 重要性采样，低差异序列值 Xi，定向和偏移采样向量，以使其朝向特定粗糙度的镜面波瓣方向。
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) 
{
	float a = roughness * roughness;
	
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
    // 从球坐标到笛卡尔坐标的半矢量
	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;
	
	// from tangent-space H vector to world-space sample vector
	vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}
// ----------------------------------------------------------------------------
void main()
{		
    vec3 N = normalize(v_FragPosition);
    // 假设视角方向V(像素到摄像头)等于镜面反射方向R等于法线
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;
    
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, u_Roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V); // 光线方向L(像素到光源)，dot(V, H)：V在H上的投影的大小

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
        
            // sample from the environment's mip level based on roughness/pdf
            float D   = DistributionGGX(N, H, u_Roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            // pdf 代表概率密度函数 (probability density function)
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

            float resolution = 512.0; // resolution of source cubemap (per face)
            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = u_Roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
            
            prefilteredColor += textureLod(u_EnvironmentMap, L, mipLevel).rgb * NdotL;
            
            //prefilteredColor += texture(u_EnvironmentMap, L).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / totalWeight;

    FragColor = vec4(prefilteredColor, 1.0);
}