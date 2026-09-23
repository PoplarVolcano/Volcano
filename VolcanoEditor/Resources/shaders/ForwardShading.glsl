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

layout (location = 0) out vec4 o_FragColor;
layout (location = 1) out vec4 o_BrightColor;

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

layout (binding = 20) uniform samplerCube u_SkyboxMap;

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

    // 逐片段存储每个法线到G缓冲中
	vec3 normal = vec3(0.0);
	if(materialNormal == vec3(0.0))
    {
	    normal = normalize(Input.normal);
    }
    else
	{
	    // 将法线贴图从切线空间转换为世界空间
	    normal = normalize(materialNormal * 2.0 - 1.0); // [-1, 1]
	    normal = normalize(TBN * normal);
	}
    
    vec3 fragPosition = Input.position;

    vec3 I = normalize(fragPosition - u_CameraPosition);

	vec3 Reflect = reflect(I, normal);

    float ratio = 1.00 / 1.00;
    vec3 Refract = refract(I, normal, ratio);

	vec3 ambient  = materialDiffuse.rgb;
	vec3 diffuse  = materialDiffuse.rgb;
	vec3 specular = (texture(u_SkyboxMap, Reflect).rgb + texture(u_SkyboxMap, Refract).rgb) * materialSpecular;
	
	vec3 lighting = ambient + diffuse + specular + materialEmission.rgb;
	
    o_FragColor = vec4(lighting, 1.0);

	// 检查片段输出是否高于阈值，如果是，则作为亮度颜色o_BrightColor输出
    float brightness = dot(o_FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));//转换为灰度来计算片段的亮度
	if(brightness > 1.0)
        o_BrightColor = o_FragColor;
    else
        o_BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}  