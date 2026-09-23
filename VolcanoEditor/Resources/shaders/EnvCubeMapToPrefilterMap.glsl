#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

layout (location = 0) out vec3 v_FragPosition;

layout (std140, binding = 40) uniform TemporaryMat4
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

layout (binding = 0) uniform samplerCube u_EnvCubeMap;

layout (std140, binding = 41) uniform TemporaryFloat
{
    float u_Roughness;
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
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// Van Der Corput 序列，把十进制数字的二进制表示镜像翻转到小数点右边，例123=>0.321，用于生成均匀分布的伪随机点
// ----------------------------------------------------------------------------------------------
float RadicalInverse_VanDerCorput(uint bits)
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // return (x / 0x100000000);
}

// 生成低差异序列，大小为 N 的样本集中的低差异样本 i，i∈[0,N)
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VanDerCorput(i));
}

/*
// 不依赖位运算符的替代版本 Van Der Corput 序列
float VanDerCorput(uint n, uint base)
{
    float invBase = 1.0 / float(base);
    float denom   = 1.0;
    float result  = 0.0;

    for(uint i = 0u; i < 32u; ++i)
    {
        if(n > 0u)
        {
            denom   = mod(float(n), 2.0);
            result += denom * invBase;
            invBase = invBase / 2.0;
            n       = uint(float(n) / 2.0);
        }
    }

    return result;
}

vec2 HammersleyNoBitOps(uint i, uint N)
{
    return vec2(float(i)/float(N), VanDerCorpus(i, 2u));
}
*/
// ----------------------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------------------
// 重要性采样，低差异序列值 Xi，定向和偏移采样向量，以使其朝向特定粗糙度的镜面波瓣方向。
// ----------------------------------------------------------------------------------------------
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) 
{
	float a = roughness * roughness;
	
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
    // 从球坐标到笛卡尔（切线空间）坐标的样本向量
	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;
	
	// 样本向量从笛卡尔（切线空间）到世界空间
	vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}
// ----------------------------------------------------------------------------------------------


void main()
{		
    // 假设视角方向V(像素到摄像头)等于镜面反射方向R等于法线
    vec3 N = normalize(v_FragPosition);
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0;
    vec3 prefilteredColor = vec3(0.0);
    
    for(uint i = 0u; i != SAMPLE_COUNT; i++)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H  = ImportanceSampleGGX(Xi, N, u_Roughness);// 样本向量
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);     // 光线方向L(像素到光源)，dot(V, H)：V在H上的投影的大小

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {   
            //----------------------------------------------------------------------------------------
            // 镜面反射中光强度的变化大，高频细节多，
            // 对镜面反射进行卷积需要大量采样，才能正确反映 HDR 环境反射的混乱变化。
            // 我们已经进行了大量的采样，但是在某些环境下，在某些较粗糙的 mip 级别上可能仍然不够，导致明亮区域周围出现点状图案
            // 基于积分的 PDF 和粗糙度 采样环境贴图的 mipmap ，以减少伪像
            // pdf: 概率密度函数 (probability density function)
            //----------------------------------------------------------------------------------------
            float D   = DistributionGGX(N, H, u_Roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

            float resolution = 512.0; // 源立方体贴图的分辨率（每个面）
            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = u_Roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
            
            // 对一个立方体贴图（Cube Map）进行采样，并允许指定使用哪个Mipmap层级（Level of Detail, LOD）
            prefilteredColor += textureLod(u_EnvCubeMap, L, mipLevel).rgb * NdotL;
            
            totalWeight      += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / totalWeight;

    FragColor = vec4(prefilteredColor, 1.0);
}