#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

layout(std140, binding = 7) uniform CameraData
{
	mat4 u_View; // 传入的view矩阵经过mat4(mat3(view))，去除了移动
    mat4 u_Projection;
};

layout (location = 0) out vec3 TexCoords;

void main()
{
    TexCoords = a_Position;
    vec4 position = u_Projection * u_View * vec4(a_Position, 1.0);
    gl_Position = position.xyww;
}  

#type fragment
#version 450 core
layout(location = 0) out vec4 FragColor;

layout (location = 0) in vec3 TexCoords;

layout (binding = 0) uniform samplerCube skybox;

void main()
{    
    FragColor = texture(skybox, TexCoords);
}