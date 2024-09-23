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

layout (location = 0) out float FragColor;

struct VertexOutput
{
	vec2 TexCoords;
};

layout (location = 0) in VertexOutput Input;

layout (binding = 0)  uniform sampler2D g_PositionDepth;
layout (binding = 1)  uniform sampler2D g_Normal;
layout (binding = 2)  uniform sampler2D texNoise;

layout(std140, binding = 0) uniform Camera
{
	// projection * view
	mat4 u_ViewProjection;
};

layout (std140, binding = 13) uniform Samples
{
    vec3 u_Samples[64];
};

layout (std140, binding = 14) uniform SSAO
{
    int u_KernelSize;
    float u_Radius;
    float u_Bias;
    float u_Power;
};

void main()
{     
    // tile noise texture over screen based on screen dimensions divided by noise size
    const vec2 noiseScale = vec2(2560.0/4.0, 1440.0/4.0); 


    vec3 fragPosition = texture(g_PositionDepth, Input.TexCoords).xyz;
    float fragDepth   = texture(g_PositionDepth, Input.TexCoords).a;
    vec3 normal       = normalize(texture(g_Normal, Input.TexCoords).rgb);
    vec3 randomVec    = normalize(texture(texNoise, Input.TexCoords * noiseScale).xyz);
    
    // TBN矩阵，切线空间变换到世界空间
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal)); //Gramm-Schmidt Process
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < u_KernelSize; ++i)
    {
        vec3 samplePos = TBN * u_Samples[i]; // from tangent to view-space
        samplePos = fragPosition + samplePos * u_Radius;  // 世界坐标样本位置
        
        vec4 offset = vec4(samplePos, 1.0);
        offset = u_ViewProjection * offset; //裁剪空间
        offset.xyz /= offset.w; //透视划分 perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // [0.0, 1.0]
        
        float sampleDepth = texture(g_PositionDepth, offset.xy).w; // 获取采样核心的深度
        
        //float sampleDepth = texture(g_PositionDepth, offset.xy).z;

        // 范围测试
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, u_Radius / abs(fragDepth - sampleDepth)); // (fragDepth - sampleDepth)区间[-0.5,0.5] rangeCheck = 1,
        occlusion += (offset.w - u_Bias >= sampleDepth ? 1.0 : 0.0) * rangeCheck; 

        //occlusion += (samplePos.z + u_Bias <= sampleDepth ? 1.0 : 0.0); 
    }
    occlusion = 1.0 - (occlusion / u_KernelSize);
    
    FragColor = pow(occlusion, u_Power);;
    
}  