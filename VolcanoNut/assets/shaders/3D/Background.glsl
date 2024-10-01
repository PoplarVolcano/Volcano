#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

layout (location = 0) out vec3 v_TexCoords;

layout(std140, binding = 7) uniform CameraData
{
	mat4 u_View; // �����view���󾭹�mat4(mat3(view))��ȥ�����ƶ�
    mat4 u_Projection;
};

void main()
{
    v_TexCoords = a_Position;
    vec4 position = u_Projection * u_View * vec4(a_Position, 1.0);
    gl_Position = position.xyww;
}

#type fragment
#version 450 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out int o_EntityID;

layout (location = 0) in vec3 v_TexCoords;

layout (binding = 0) uniform samplerCube environmentMap;

void main()
{		
    vec3 envColor = texture(environmentMap, v_TexCoords).rgb;
    
    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
    
    if(envColor == vec3(0.0))
        FragColor = vec4(vec3(1.0), 1.0);
    else
        FragColor = vec4(envColor, 1.0);
    o_EntityID = -1;
}