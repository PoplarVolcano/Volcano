#type vertex
#version 450 core

layout(location = 0) in vec3 a_WorldPosition;
layout(location = 1) in vec2 a_LocalPosition;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in float a_Thickness;
layout(location = 4) in float a_Fade;
layout(location = 5) in int a_EntityID;

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
	vec2 LocalPosition;
	vec4 Color;
	float Thickness;
	float Fade;
};

layout (location = 0) out VertexOutput Output;
layout (location = 4) out flat int v_EntityID;

void main()
{
	Output.LocalPosition = a_LocalPosition;
	Output.Color = a_Color;
	Output.Thickness = a_Thickness;
	Output.Fade = a_Fade;

	v_EntityID = a_EntityID;

	gl_Position = u_CameraProjection * u_CameraView * vec4(a_WorldPosition, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

struct VertexOutput
{
	vec2 LocalPosition;
	vec4 Color;
	float Thickness;
	float Fade;
};

layout (location = 0) in VertexOutput Input;
layout (location = 4) in flat int v_EntityID;

void main()
{
    // 计算距离并用白色填充圆圈
    float distance = 1.0 - length(Input.LocalPosition);
    float fade = smoothstep(0.0, Input.Fade, distance);
    fade *= smoothstep(Input.Thickness + Input.Fade, Input.Thickness, distance);
	
	// fade为0则抛弃抛弃像素，在点击圆边缘选中背后entity时不会获得错误EntityID
	if (fade == 0.0)
		discard;

    // Set output color
    o_Color = Input.Color;
	o_Color.a *= fade;

	o_EntityID = v_EntityID;
}