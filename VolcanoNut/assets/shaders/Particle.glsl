#type vertex
#version 450 core
layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoords;

layout(std140, binding = 0) uniform Camera
{
	// projection * view
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec2 TexCoords;
};

layout (location = 0) out VertexOutput Output;

void main()
{
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
	Output.TexCoords = a_TexCoords;
}


#type fragment
#version 450 core

layout (location = 0) out vec4 FragColor;

struct VertexOutput
{
	vec2 TexCoords;
};

layout (location = 0) in VertexOutput Input;

layout (binding = 0) uniform sampler2D u_Diffuse;

void main()
{
	vec4 diffuse = texture(u_Diffuse, Input.TexCoords);
	if (diffuse.w == 0.0f)
		discard;
    FragColor = diffuse;//vec4(1.0f, 1.0f, 1.0f, 1.0f);
}  