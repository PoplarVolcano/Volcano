#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 3) in vec3 a_Normal;

struct VertexOutput
{
    vec3  position;
    vec3  normal;
    int   vertexIndex;
    vec4  normalVisualizationColor;
    float normalVisualizationLength;
    int   normalVisualizationIndex1;
    int   normalVisualizationIndex2;
};

layout (location = 0) out VertexOutput Output;

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
};

layout (std430, binding = 22) readonly buffer InstanceDataMaterialBuffer
{
    InstanceDataMaterial u_InstanceDataMaterial[];
};

struct InstanceDataNormalVisualization
{            
    vec4  color;
    float normalVisualizationLength;
    int   index1;
    int   index2;
};

layout (std430, binding = 25) readonly buffer InstanceDataNormalVisualizationBuffer
{
    InstanceDataNormalVisualization u_InstanceDataNormalVisualization[];
};

void main()
{
    InstanceData instanceData = u_InstanceData[gl_InstanceIndex];
    InstanceDataMaterial instanceDataMaterial = u_InstanceDataMaterial[gl_InstanceIndex];
    InstanceDataNormalVisualization instanceDataNormalVisualization = u_InstanceDataNormalVisualization[gl_InstanceIndex];
    
    vec4 worldPosition = instanceData.transform * vec4(a_Position, 1.0);
    vec3 worldNormal = normalize(mat3(instanceDataMaterial.normalTransform) * a_Normal);

    Output.position                  = worldPosition.rgb;
    Output.normal                    = worldNormal;
    Output.vertexIndex               = gl_VertexIndex;
    Output.normalVisualizationLength = instanceDataNormalVisualization.normalVisualizationLength;
    Output.normalVisualizationColor  = instanceDataNormalVisualization.color;
    Output.normalVisualizationIndex1 = instanceDataNormalVisualization.index1;
    Output.normalVisualizationIndex2 = instanceDataNormalVisualization.index2;
}



#type geometry
#version 450 core
layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

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
    vec3  normal;
    int   vertexIndex;
    vec4  normalVisualizationColor;
    float normalVisualizationLength;
    int   normalVisualizationIndex1;
    int   normalVisualizationIndex2;
};

layout (location = 0) in VertexOutput Input[];

layout (location = 0) out vec4 v_Color;

void GenerateLine(int index)
{

    if (Input[index].normalVisualizationIndex1 < 0 || Input[index].normalVisualizationIndex2 < 0)
        return;
    int vertexIndex = Input[index].vertexIndex;
    if (vertexIndex < Input[index].normalVisualizationIndex1 || vertexIndex > Input[index].normalVisualizationIndex2)
        return;
         
    vec3 start = Input[index].position;
    vec3 end = start + Input[index].normal * Input[index].normalVisualizationLength;

    gl_Position = u_CameraProjection * u_CameraView * vec4(start, 1.0);
    v_Color = Input[index].normalVisualizationColor;
    EmitVertex();
    gl_Position = u_CameraProjection * u_CameraView  * vec4(end, 1.0);
    v_Color = Input[index].normalVisualizationColor;
    EmitVertex();
    EndPrimitive();
}

void main()
{ 
    // 对三个顶点分别尝试生成法线
    for (int i = 0; i < 3; i++)
    {
        GenerateLine(i);
    }
}



#type fragment
#version 450 core
layout (location = 0) out vec4 o_FragColor;

layout (location = 0) in vec4 v_Color;

void main()
{
    o_FragColor = v_Color;
}