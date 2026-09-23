#type vertex
#version 450 core
layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;

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

layout (location = 0) out float o_FragColor;

struct VertexOutput
{
	vec2 texCoord;
};

layout (location = 0) in VertexOutput Input;

layout (binding = 0)  uniform sampler2D g_PositionDepth;
layout (binding = 1)  uniform sampler2D g_Normal;
layout (binding = 2)  uniform sampler2D u_NoiseTexture;

layout(std140, binding = 0) uniform CameraData
{
	mat4  u_CameraView;
	mat4  u_CameraProjection;
	vec3  u_CameraPosition;
	float u_CameraNearClip;
	float u_CameraFarClip;
};
layout (std140, binding = 17) uniform Samples
{
    vec4 u_Samples[64];
};

layout (std140, binding = 18) uniform SSAO
{
    int   u_KernelSize;
    float u_Radius;
    float u_Bias;
    float u_Power;
    int   u_SSAOEnabled;
};

void main()
{     
    // 噪声纹理的平铺缩放（屏幕尺寸 / 噪声尺寸）
    const vec2 noiseScale = vec2(2560.0/4.0, 1440.0/4.0); 

    // 世界空间位置、视图空间线性深度、法线、随机旋转向量
    vec3  fragPosition = texture(g_PositionDepth, Input.texCoord).xyz;
    float fragDepth    = texture(g_PositionDepth, Input.texCoord).a;
    vec3  normal       = normalize(texture(g_Normal, Input.texCoord).rgb);
    vec3  randomVec    = normalize(texture(u_NoiseTexture, Input.texCoord * noiseScale).xyz);
    
    normal = normal * 2.0f - 1.0f;

    // TBN矩阵，切线空间变换到世界空间，Gramm-Schmidt Process（处理）
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // 遍历样本并计算遮挡因子
    float occlusion = 0.0;
    for(int i = 0; i != u_KernelSize; i++)
    {
        // 采样方向：切线空间->世界空间
        vec3 sampleDirection = TBN * u_Samples[i].xyz;
        // 世界坐标样本位置
        vec3 samplePosition = fragPosition + sampleDirection * u_Radius;  
        
        // 世界空间->屏幕空间
        vec4 offset = u_CameraProjection * u_CameraView * vec4(samplePosition, 1.0);
        // 透视除法 perspective divide，标准化设备坐标NDC，[-1,1]
        offset.xyz /= offset.w;
        // NDC -> [0,1]纹理坐标
        offset.xy = offset.xy * 0.5 + 0.5;
        
        // 采样该点的视图空间深度
        float sampleDepth = texture(g_PositionDepth, offset.xy).w;
        
        // 范围检查（世界空间距离）
        // smoothstep光滑插值
        // 样本的当前深度值是否大于存储的深度值
        //float rangeCheck = smoothstep(0.0, 1.0, u_Radius / abs(fragDepth - sampleDepth));
        float rangeCheck = smoothstep(0.0, 1.0, length(samplePosition - fragPosition) / u_Radius);
        occlusion += (sampleDepth >= fragDepth ? 1.0 : 0.0) * rangeCheck; 

    }
    occlusion = 1.0 - (occlusion / float(u_KernelSize));
    
    o_FragColor = pow(occlusion, u_Power);
    
}  