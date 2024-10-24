#type vertex
#version 450 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoords;

layout (location = 0) out vec2 TexCoords;

// фад╩©М╦ъ
layout (std140, binding = 17) uniform Resolution
{
    vec4 u_Resolution;
};

void main()
{
    float windowAspect = u_Resolution.x / u_Resolution.y;
    float aspect = 16.0 / 9.0;
    float framebufferAspect = u_Resolution.z / u_Resolution.w;
    vec2 uv;
    if(windowAspect > aspect)
        uv = vec2(a_Pos.x * aspect / windowAspect , a_Pos.y);
    else
        uv = vec2(a_Pos.x, a_Pos.y * windowAspect / aspect);
        
    vec2 texcoords = a_TexCoords * 2.0 - 1.0;
    if(framebufferAspect > aspect)
    {
        texcoords = vec2(texcoords.x * aspect / framebufferAspect , texcoords.y);
    }
    else
    {
        texcoords = vec2(texcoords.x, texcoords.y * framebufferAspect / aspect);
    }

    gl_Position = vec4(uv, 0.0, 1.0); 
    TexCoords = (texcoords + 1.0) / 2.0;
}

#type fragment
#version 450 core
layout(location = 0) out vec4 FragColor;

layout (location = 0) in vec2 TexCoords;

layout (binding = 0) uniform sampler2D screenTexture;

void main()
{ 
    FragColor = texture(screenTexture, TexCoords);
}