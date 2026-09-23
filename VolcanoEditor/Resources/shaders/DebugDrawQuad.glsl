#type vertex
#version 450 core
layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;

struct VertexOutput
{
	vec2 texCoord;
};

layout (location = 0) out VertexOutput Output;

void main()
{
	Output.texCoord = a_TexCoord;

	gl_Position = vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout (location = 0) out vec4 o_FragColor;

struct VertexOutput
{
	vec2 texCoord;
};

layout (location = 0) in VertexOutput Input;

void main()
{
    o_FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0);
}