#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;

layout (location = 0) out vec2 v_TexCoord;

void main()
{
    gl_Position = vec4(a_Position, 1.0); 
    v_TexCoord = a_TexCoord;
}

#type fragment
#version 450 core
layout(location = 0) out vec4 o_FragColor;

layout (location = 0) in vec2 v_TexCoord;

layout (binding = 0) uniform sampler2D u_ScreenTexture;
layout (binding = 1) uniform sampler2D u_GaussianBlur;

layout (binding = 14) uniform Exposure
{
    float u_Exposure;
};

layout (binding = 16) uniform BloomEnabled
{
    bool u_BloomEnabled;
};

void main()
{ 
    vec3 hdrColor = texture(u_ScreenTexture, v_TexCoord).rgb;
    vec3 bloomColor = texture(u_GaussianBlur, v_TexCoord).rgb;
    if(u_BloomEnabled)
        hdrColor += bloomColor; // additive blending

    // 曝光色调映射，exp(x)返回e的x次幂
    vec3 result = vec3(1.0) - exp(-hdrColor * u_Exposure);

    o_FragColor = vec4(result, 1.0);

}