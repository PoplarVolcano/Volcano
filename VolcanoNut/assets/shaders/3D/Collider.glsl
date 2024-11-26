#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

layout(std140, binding = 0) uniform Camera
{
	// projection * view
	mat4 u_ViewProjection;
};

layout(std140, binding = 6) uniform ModelTransform
{
	mat4 u_ModelTransform;
    mat4 u_NormalTransform;
};

void main()
{
    gl_Position = u_ViewProjection * u_ModelTransform * vec4(a_Position, 1.0f);
}  
