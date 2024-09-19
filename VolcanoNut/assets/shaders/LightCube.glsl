#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 4) in vec4 a_Color;
layout (location = 10) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	// projection * view
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec3 Position;
	vec4 Color;
};

layout (location = 0) out flat int v_EntityID;
layout (location = 1) out VertexOutput Output;

void main()
{
	v_EntityID      = a_EntityID;
	Output.Position = a_Position;
	Output.Color    = a_Color;

    gl_Position = u_LightSpaceMatrix * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out int o_EntityID;
layout (location = 2) out vec4 BrightColor;

struct VertexOutput
{
	vec3 Position;
	vec4 Color;
};

layout (location = 0) in flat int v_EntityID;
layout (location = 1) in VertexOutput Input;


layout (std140, binding = 3) uniform PointLight
{
	vec3 u_PointLightPosition;
	vec3 u_PointLightAmbient;
	vec3 u_PointLightDiffuse;
	vec3 u_PointLightSpecular;
	float u_PointLightConstant;
    float u_PointLightLinear;
    float u_PointLightQuadratic;
};


void main()
{             
    FragColor = vec4(u_PointLightAmbient, 1.0);
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
	else
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

	o_EntityID = v_EntityID;
}