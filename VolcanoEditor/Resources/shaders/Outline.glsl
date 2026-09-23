#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

layout(std140, binding = 0) uniform CameraData
{
	mat4  u_CameraView;
	mat4  u_CameraProjection;
	vec3  u_CameraPosition;
	float u_CameraNearClip;
	float u_CameraFarClip;
};

struct InstanceData
{            
	mat4 transform;
};

layout (std430, binding = 21) readonly buffer InstanceDataBuffer
{
    InstanceData u_InstanceData[];
};

struct InstanceDataOutline
{            
	vec4  color;
};

layout (std430, binding = 24) readonly buffer InstanceDataOutlineBuffer
{
    InstanceDataOutline u_InstanceDataOutline[];
};

struct VertexOutput
{
	vec4 outlineColor;
};

layout (location = 0) out VertexOutput Output;

void main()
{
    InstanceData instanceData = u_InstanceData[gl_InstanceIndex];
    InstanceDataOutline instanceDataOutline = u_InstanceDataOutline[gl_InstanceIndex];

	Output.outlineColor = instanceDataOutline.color;
	
    gl_Position = u_CameraProjection * u_CameraView * instanceData.transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout (location = 0) out vec4 o_FragColor;

struct VertexOutput
{
	vec4 outlineColor;
};

layout (location = 0) in VertexOutput Input;

void main()
{
    o_FragColor = Input.outlineColor;
}