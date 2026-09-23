
#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(std140, binding = 0) uniform CameraData
{
	mat4  u_CameraView;
	mat4  u_CameraProjection;
	vec3  u_CameraPosition;
	float u_CameraNearClip;
	float u_CameraFarClip;
};

struct VertexOutput
{
	vec3  position;
	vec4  color;
	vec2  texCoord;
	vec4  uvRect;
    vec3  normal;
	float tilingFactor;
	mat3  TBN;
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

layout (location = 0) out flat int v_MaterialIndex;
layout (location = 1) out VertexOutput Output;


void main()
{
    InstanceData instanceData = u_InstanceData[gl_InstanceID];
    InstanceDataMaterial instanceDataMaterial = u_InstanceDataMaterial[gl_InstanceID];

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
	Output.tilingFactor              = instanceDataMaterial.tilingFactor;
	Output.TBN                       = TBN;
    
    gl_Position = u_CameraProjection * u_CameraView * position;
}

#type fragment
#version 450 core
#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_bindless_texture : enable

struct VertexOutput {
    vec3  position;
    vec4  color;
    vec2  texCoord;
    vec4  uvRect;
    vec3  normal;
    float tilingFactor;
    mat3  TBN;
};

layout (location = 0) in flat int v_MaterialIndex;
layout (location = 1) in VertexOutput Input;

struct MaterialHandle {
    uint64_t diffuse;
    uint64_t specular;
    uint64_t normal;
    uint64_t parallax;
    uint64_t roughness;
    uint64_t ao;
    uint64_t emission;
};

layout(std430, binding = 20) readonly buffer MaterialBuffer {
    MaterialHandle u_MaterialHandles[];
};

layout (location = 0) out vec4 o_FragColor;

void main()
{
    MaterialHandle materialHandle = u_MaterialHandles[v_MaterialIndex];

    vec2 finalUV = Input.texCoord * Input.tilingFactor * Input.uvRect.zw + Input.uvRect.xy;
    vec4 texColor = texture(sampler2D(materialHandle.diffuse), finalUV);
    vec4 finalColor = texColor * Input.color;

    if (finalColor.a < 0.01)
        discard;

    o_FragColor = finalColor;
}