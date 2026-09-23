#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

layout(std140, binding = 0) uniform CameraData
{
	mat4  u_CameraView;
	mat4  u_CameraProjection;
	vec3  u_CameraPosition;
	float u_CameraNearClip;
	float u_CameraFarClip;
};

layout (location = 0) out vec3 v_TexCoords;

void main()
{
    v_TexCoords = a_Position;
    
    mat4 view = mat4(mat3(u_CameraView)); // 去除移动
    vec4 position = u_CameraProjection * view * vec4(a_Position, 1.0);
    gl_Position = position.xyww;
}  

#type fragment
#version 450 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out int o_EntityID;

layout (location = 0) in vec3 v_TexCoords;

layout (binding = 0) uniform samplerCube skybox;

void main()
{    
    vec3 envColor = texture(skybox, v_TexCoords).rgb;
    
    // HDR tonemap and gamma correct
    //envColor = envColor / (envColor + vec3(1.0));
    //envColor = pow(envColor, vec3(1.0/2.2)); 

    FragColor = vec4(envColor, 1.0);
    o_EntityID = -1;
}