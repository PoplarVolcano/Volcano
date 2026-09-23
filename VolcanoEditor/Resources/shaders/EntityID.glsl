#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

struct VertexOutput
{
    vec3  position;
	vec2  texCoord;
	vec4  uvRect;
	float tilingFactor;
    float explosionOffset;
};

struct InstanceData
{            
	mat4 transform;
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

layout (std430, binding = 23) readonly buffer InstanceDataExplosionBuffer
{
    float u_ExplosionOffset[];
};

layout (std430, binding = 29) readonly buffer InstanceDataEntityID
{
    int u_EntityID[];
};

layout (location = 0) out flat int v_MaterialIndex;
layout (location = 1) out flat int v_EntityID;
layout (location = 2) out VertexOutput Output;

void main()
{
    InstanceData instanceData = u_InstanceData[gl_InstanceID];
    InstanceDataMaterial instanceDataMaterial = u_InstanceDataMaterial[gl_InstanceID];

    v_MaterialIndex                  = instanceDataMaterial.materialIndex;
    v_EntityID                       = u_EntityID[gl_InstanceID];
	Output.position                  = vec3(instanceData.transform * vec4(a_Position, 1.0));
	Output.texCoord                  = a_TexCoord;
	Output.uvRect                    = instanceDataMaterial.uvRect;
	Output.tilingFactor              = instanceDataMaterial.tilingFactor;
    Output.explosionOffset           = u_ExplosionOffset[gl_InstanceID];
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

layout (location = 0) in flat int v_MaterialIndex_in[];
layout (location = 1) in flat int v_EntityID_in[];
layout (location = 2) in VertexOutput
{
    vec3  position;
	vec2  texCoord;
	vec4  uvRect;
	float tilingFactor;
    float explosionOffset;
} v_Output_in[];

layout (location = 0) out flat int v_MaterialIndex;
layout (location = 1) out flat int v_EntityID;
layout (location = 2) out VertexOutput
{
    vec3  position;
	vec2  texCoord;
	vec4  uvRect;
	float tilingFactor;
    float explosionOffset;
} v_Output;

void main()
{
    vec3 A = v_Output_in[0].position;
    vec3 B = v_Output_in[1].position;
    vec3 C = v_Output_in[2].position;
    vec3 faceNormal = normalize(cross(B - A, C - A));

    for (int i = 0; i < 3; i++)
    {
        vec3 offsetPosition = v_Output_in[i].position + faceNormal * v_Output_in[i].explosionOffset;

        // 传递所有属性（除位置外保持不变）
        v_MaterialIndex          = v_MaterialIndex_in[i];
        v_EntityID               = v_EntityID_in[i];
        v_Output.position        = offsetPosition;
        v_Output.texCoord        = v_Output_in[i].texCoord;
        v_Output.uvRect          = v_Output_in[i].uvRect;
        v_Output.tilingFactor    = v_Output_in[i].tilingFactor;
        v_Output.explosionOffset = v_Output_in[i].explosionOffset;

        gl_Position = u_CameraProjection * u_CameraView * vec4(offsetPosition, 1.0);
        EmitVertex();
    }
    EndPrimitive();
}

#type fragment
#version 450 core

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

layout(location = 0) out int o_EntityID;

struct VertexOutput
{
    vec3  position;
	vec2  texCoord;
	vec4  uvRect;
	float tilingFactor;
    float explosionOffset;
};

// flat 是 GLSL 中的插值限定符，用于禁用顶点着色器输出到片段着色器之间的插值。
// 对于整数类型（int、uint），必须使用 flat，否则会报错 Integer varying must be flat。
layout (location = 0) in flat int v_MaterialIndex;
layout (location = 1) in flat int v_EntityID;
layout (location = 2) in VertexOutput Input;

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

void main()
{
    MaterialHandle materialHandle = u_MaterialHandles[v_MaterialIndex];
    
	vec2 texCoord = Input.texCoord * Input.tilingFactor * Input.uvRect.zw + Input.uvRect.xy;

	vec4 materialDiffuse = texture(sampler2D(materialHandle.diffuse), texCoord);

	if(materialDiffuse.a < 0.001)
	    discard;

	o_EntityID = v_EntityID;
}  