#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

struct VertexOutput
{
	vec3  position;
	vec4  color;
	vec2  texCoord;
	vec4  uvRect;
    vec3  normal;
    float parallaxScale;
	float tilingFactor;
    float explosionOffset;
	mat3  TBN;
};

struct InstanceData
{            
	mat4  transform;
};

layout (std430, binding = 21) readonly buffer InstanceDataBuffer
{
    InstanceData u_InstanceData[];
};

struct InstanceDataMaterial
{     
	mat4  normalTransform;
	vec4  color;
	vec4  uvRect;
    float parallaxScale;
	float tilingFactor;
    int   materialIndex;
};

layout (std430, binding = 22) readonly buffer InstanceDataMaterialBuffer
{
    InstanceDataMaterial u_InstanceDataMaterial[];
};

// 注意：因为该结构体只有 4 字节，std430 中数组元素之间会连续排列（无填充），
// 所以着色器中读取时需确保使用 float 数组而非结构体数组（或仍使用结构体，但编译器会按 4 字节对齐处理）。
struct InstanceDataExplosion
{            
    float offset;
};

layout (std430, binding = 23) readonly buffer InstanceDataExplosionBuffer
{
    InstanceDataExplosion u_InstanceDataExplosion[];
};


layout (location = 0) out flat int v_MaterialIndex;
layout (location = 1) out VertexOutput Output;

void main()
{
    InstanceData instanceData = u_InstanceData[gl_InstanceID];
    InstanceDataMaterial instanceDataMaterial = u_InstanceDataMaterial[gl_InstanceID];
    InstanceDataExplosion instanceDataExplosion = u_InstanceDataExplosion[gl_InstanceID];

    vec3 T = normalize(mat3(instanceDataMaterial.normalTransform) * a_Tangent);
    vec3 N = normalize(mat3(instanceDataMaterial.normalTransform) * a_Normal);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);
    
    vec4 position = instanceData.transform * vec4(a_Position, 1.0);

    v_MaterialIndex                  = instanceDataMaterial.materialIndex;
	Output.position                  = position.rgb;
	Output.color                     = instanceDataMaterial.color;
	Output.texCoord                  = a_TexCoord;
	Output.uvRect                    = instanceDataMaterial.uvRect;
	Output.normal                    = N;
	Output.parallaxScale             = instanceDataMaterial.parallaxScale;
	Output.tilingFactor              = instanceDataMaterial.tilingFactor;
    Output.explosionOffset           = instanceDataExplosion.offset;
	Output.TBN                       = TBN;
    
}

#type geometry
#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std140, binding = 0) uniform CameraData
{
	mat4  u_CameraView;
	mat4  u_CameraProjection;
	vec3  u_CameraPosition;
	float u_CameraNearClip;
	float u_CameraFarClip;
};

// 输入：接收顶点着色器的变量（需要加 []，因为每个顶点都有值）

layout (location = 0) in flat int v_MaterialIndex_in[];
layout (location = 1) in VertexOutput
{
	vec3  position;
	vec4  color;
	vec2  texCoord;
	vec4  uvRect;
    vec3  normal;
    float parallaxScale;
	float tilingFactor;
    float explosionOffset;
	mat3  TBN;
} v_Output_in[];

// 输出：转发变量到片段着色器
layout (location = 0) out flat int v_MaterialIndex;
layout (location = 1) out VertexOutput
{
	vec3  position;
	vec4  color;
	vec2  texCoord;
	vec4  uvRect;
    vec3  normal;
    float parallaxScale;
	float tilingFactor;
    float explosionOffset;
	mat3  TBN;
} v_Output;

void main()
{
    // 计算三角形在世界空间中的面法线
    vec3 A = v_Output_in[0].position;
    vec3 B = v_Output_in[1].position;
    vec3 C = v_Output_in[2].position;
    vec3 faceNormal = normalize(cross(B - A, C - A));

    // 对每个顶点进行偏移
    for (int i = 0; i < 3; i++)
    {
        vec3 offsetPos = v_Output_in[i].position + faceNormal * v_Output_in[i].explosionOffset;

        // 传递所有属性（除位置外保持不变）
        v_MaterialIndex          = v_MaterialIndex_in[i];
        v_Output.position        = offsetPos;
        v_Output.color           = v_Output_in[i].color;
        v_Output.texCoord        = v_Output_in[i].texCoord;
        v_Output.uvRect          = v_Output_in[i].uvRect;
        v_Output.normal          = v_Output_in[i].normal;
        v_Output.parallaxScale   = v_Output_in[i].parallaxScale;
        v_Output.tilingFactor    = v_Output_in[i].tilingFactor;
        v_Output.explosionOffset = v_Output_in[i].explosionOffset;
        v_Output.TBN             = v_Output_in[i].TBN;

        gl_Position = u_CameraProjection * u_CameraView * vec4(offsetPos, 1.0);
        EmitVertex();
    }
    EndPrimitive();
}

#type fragment
#version 450 core

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

layout (location = 0) out vec4 g_PositionAndDepth;
layout (location = 1) out vec4 g_Albedo;
layout (location = 2) out vec4 g_Normal;
layout (location = 3) out vec4 g_RoughnessAndAO;
layout (location = 4) out vec4 g_Emission;

struct VertexOutput
{
	vec3  position;
	vec4  color;
	vec2  texCoord;
	vec4  uvRect;
    vec3  normal;
    float parallaxScale;
	float tilingFactor;
    float explosionOffset;
	mat3  TBN;
};

// flat 是 GLSL 中的插值限定符，用于禁用顶点着色器输出到片段着色器之间的插值。
// 对于整数类型（int、uint），必须使用 flat，否则会报错 Integer varying must be flat。
layout (location = 0) in flat int v_MaterialIndex;
layout (location = 1) in VertexOutput Input;

layout(std140, binding = 0) uniform CameraData
{
	mat4  u_CameraView;
	mat4  u_CameraProjection;
	vec3  u_CameraPosition;
	float u_CameraNearClip;
	float u_CameraFarClip;
};

struct MaterialHandle
{
    uint64_t diffuse;
    uint64_t specular;
    uint64_t normal;
    uint64_t parallax;
    uint64_t roughness;
    uint64_t ao;
    uint64_t emission;
};

layout(std430, binding = 20) readonly buffer MaterialBuffer
{
    MaterialHandle u_MaterialHandles[];
};

// ----------------------------------------------------------------------------
mat3 getTBN()
{
    // dFdx、dFdy：变量在屏幕空间中沿水平方向（x）和垂直方向（y）的偏导数，也就是该变量在相邻像素之间的变化率
    vec3 Q1  = dFdx(Input.position);
    vec3 Q2  = dFdy(Input.position);
    vec2 st1 = dFdx(Input.texCoord);
    vec2 st2 = dFdy(Input.texCoord);

    vec3 N  = normalize(Input.normal);
    vec3 T  = normalize(Q1 * st2.t - Q2 * st1.t); // st2.t => st2.y
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return TBN;
}
// ----------------------------------------------------------------------------

void main()
{
    MaterialHandle materialHandle = u_MaterialHandles[v_MaterialIndex];
    
    // 如果 Input.TBN[1] == vec3(0.0)（即未传递有效的 TBN），则通过 dFdx / dFdy 从屏幕空间导数重建 TBN（用于没有切线数据的模型或细面）。
    mat3 TBN;
    if (Input.TBN[1] == vec3(0.0))
    {
        TBN = getTBN();
    }
    else
    {
        TBN = Input.TBN;
    }
    
	vec3 viewDirection = normalize(u_CameraPosition - Input.position);
	vec2 texCoord = Input.texCoord * Input.tilingFactor * Input.uvRect.zw + Input.uvRect.xy;

    // 视差映射（Steep Parallax Mapping）
    if (Input.parallaxScale != 0)
	{
	    // 将视角转换为切线空间（TBN是正交矩阵，转置等于求逆）
	    vec3 viewDir = normalize(transpose(TBN) * viewDirection);

		const float minLayers = 8;
        const float maxLayers = 32;
		// 根据视角与法线夹角动态调整采样层数：
        // viewDir和正z方向的点乘，使用它的结果根据我们看向表面的角度调整样本数量（正z方向等于切线空间中的表面的法线）
        float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  //abs求绝对值
        float layerDepth = 1.0 / numLayers;
        float currentLayerDepth = 0.0;
        // 使用 viewDir.xy / viewDir.z 计算偏移方向
        vec2 P = viewDir.xy / viewDir.z * Input.parallaxScale; 
        vec2 deltaTexCoord = P / numLayers;

        vec2  currentTexCoord     = texCoord;
        float currentDepthMapValue = texture(sampler2D(materialHandle.parallax), currentTexCoord).r;
          
        // 进行深度迭代，寻找最接近真实高度的纹理坐标。
        while(currentLayerDepth < currentDepthMapValue)
        {
            currentTexCoord -= deltaTexCoord;
            currentDepthMapValue = texture(sampler2D(materialHandle.parallax), currentTexCoord).r;  
            currentLayerDepth += layerDepth;  
        }

        // get texture coordinates before collision (reverse operations)
        vec2 prevTexCoord = currentTexCoord + deltaTexCoord;
	    
        // get depth after and before collision for linear interpolation
        float afterDepth  = currentDepthMapValue - currentLayerDepth;
        float beforeDepth = texture(sampler2D(materialHandle.parallax), prevTexCoord).r - currentLayerDepth + layerDepth;
 	    
        // interpolation of texture coordinates
        float weight = afterDepth / (afterDepth - beforeDepth);
        vec2 finalTexCoord = prevTexCoord * weight + currentTexCoord * (1.0 - weight);

        texCoord = finalTexCoord;

        // 如果纹理坐标超出 [0,1] 范围可选择 discard
	    //if(texCoord.x > 1.0 || texCoord.y > 1.0 || texCoord.x < 0.0 || texCoord.y < 0.0)
        //    discard;
	}
	
	vec4  materialDiffuse   = texture(sampler2D(materialHandle.diffuse),   texCoord);
	float materialSpecular  = texture(sampler2D(materialHandle.specular),  texCoord).r;
	vec3  materialNormal    = texture(sampler2D(materialHandle.normal),    texCoord).rgb;
    float materialRoughness = texture(sampler2D(materialHandle.roughness), texCoord).r;
    float materialAO        = texture(sampler2D(materialHandle.ao),        texCoord).r;
    vec4  materialEmission  = texture(sampler2D(materialHandle.emission),  texCoord);
    
    const float gamma = 2.2;
    materialDiffuse.rgb  = pow(materialDiffuse.rgb,  vec3(gamma));
    materialEmission.rgb = pow(materialEmission.rgb, vec3(gamma));

	if(materialDiffuse.a < 0.001)
	    discard;

	const float near = u_CameraNearClip;   // 投影矩阵的近平面
    const float far = u_CameraFarClip;     // 投影矩阵的远平面

    // gl_FragCoord 是 OpenGL/GLSL 片元着色器中的‌内置只读输入变量‌（类型 vec4），
    // 表示当前待处理片元在‌窗口坐标系‌下的位置及深度信息，无需声明即可直接使用 。‌‌
    // ‌x, y‌：片元在屏幕（窗口）上的像素坐标。默认原点在‌左下角‌，像素中心值为 0.5 起始（如第一个像素中心为 0.5, 0.5）；可通过布局限定符 origin_upper_left 改为左上角原点。
    // z‌：片元的‌深度值‌，范围通常为 [0.0, 1.0]，由顶点着色器经模型 - 视图 - 投影变换及透视除法后插值得到，默认用于深度测试。
    // w‌：裁剪空间坐标 clip.w 的倒数（即 1/clip.w）。在透视投影中，其绝对值的倒数与眼空间深度距离相关（eye_z ≈ -1/w）；正交投影下该值恒为 1。

    // 将深度采样值从 [0,1] 映射到 NDC 深度[-1,1]
    // 从 NDC 深度 转换为视图空间线性深度（以摄像机为原点，沿视线方向的物理距离）。
	float z = gl_FragCoord.z * 2.0 - 1.0;  // Back to NDC [-1,1]
	z = (2.0 * near * far) / (far + near - z * (far - near));// 转换到[near, far]
    g_PositionAndDepth = vec4(Input.position, z);

    // 逐片段存储每个法线到G缓冲中
	vec3 normal = vec3(0.0);
	if(materialNormal == vec3(0.0))
    {
	    normal = normalize(Input.normal);
    }
    else
	{
	    // 将法线贴图从切线空间转换为世界空间
	    normal = normalize(materialNormal * 2.0 - 1.0); // [0, 1] -> [-1, 1]
	    normal = normalize(TBN * normal);
	}
    normal = normal * 0.5 + 0.5;  // [-1, 1] -> [0, 1]

    g_Albedo = vec4(materialDiffuse.rgb, materialSpecular);
    g_Normal = vec4(normal, 1.0);
	g_RoughnessAndAO = vec4(materialRoughness, materialAO, 1.0, 1.0);
    g_Emission = materialEmission;

}  