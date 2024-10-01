#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoords;
layout (location = 3) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	// projection * view
	mat4 u_ViewProjection;
};

struct VertexOutput
{
    vec3 FragPosition;
	vec2 TexCoords;
    vec3 Normal;
};

layout (location = 0) out flat int v_EntityID;
layout (location = 1) out VertexOutput Output;

void main()
{
	v_EntityID          = a_EntityID;
    Output.FragPosition = a_Position;
    Output.TexCoords    = a_TexCoords;
    Output.Normal       = a_Normal;   

    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out int o_EntityID;
layout (location = 2) out vec4 BrightColor;

struct VertexOutput
{
    vec3 FragPosition;
	vec2 TexCoords;
    vec3 Normal;
};

layout (location = 0) in flat int v_EntityID;
layout (location = 1) in VertexOutput Input;

void main()
{             
    FragColor = vec4(Input.FragPosition, 1.0);
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
	else
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

	o_EntityID = v_EntityID;
}