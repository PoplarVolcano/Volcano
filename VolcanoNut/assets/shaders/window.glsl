#type vertex
#version 450 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoords;

layout (location = 0) out vec2 TexCoords;

void main()
{
    gl_Position = vec4(a_Pos, 1.0); 
    TexCoords = a_TexCoords;
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