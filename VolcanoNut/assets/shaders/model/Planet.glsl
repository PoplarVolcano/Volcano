#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 2) in vec2 a_TexCoords;

layout (location = 0) out vec2 TexCoords;

layout(std140, binding = 0) uniform Camera
{
	// projection * view
	mat4 u_ViewProjection;
};

//layout(std140, binding = 0)uniform mat4 model;

void main()
{
    TexCoords = a_TexCoords;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0f); 
}

#type fragment
#version 450 core
layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 TexCoords;

layout (binding = 0) uniform sampler2D texture_diffuse1;

void main()
{
    FragColor = texture(texture_diffuse1, TexCoords);
}