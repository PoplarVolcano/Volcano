#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;

layout (location = 0) out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core
layout(location = 0) out vec2 o_FragColor;

layout (location = 0) in vec2 v_TexCoord;

const float PI = 3.14159265359;

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
    float k = (roughness * roughness) / 2.0;

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
vec2 IntegrateBRDF(float NdotV, float roughness)
{
    vec3 V;
    V.x = sqrt(1.0 - NdotV*NdotV);
    V.y = 0.0;
    V.z = NdotV;

    float A = 0.0;
    float B = 0.0; 

    vec3 N = vec3(0.0, 0.0, 1.0);
    
    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // 生成一个偏向于首选对齐方向的样本向量（重要性采样）。
        // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if(NdotL > 0.0)
        {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return vec2(A, B);
}
// ----------------------------------------------------------------------------------------------

void main() 
{
    vec2 integratedBRDF = IntegrateBRDF(v_TexCoord.x, v_TexCoord.y); // x坐标为角度cosθ(NdotV)，y坐标为粗糙度
    o_FragColor = integratedBRDF;
}