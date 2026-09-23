#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;

layout (location = 0) out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core
layout (location = 0) out vec4 o_FragColor;

layout (location = 0) in vec2 v_TexCoord;

layout (binding = 0) uniform sampler2D image;

layout (binding = 15) uniform GaussianBlur
{
    bool u_Horizontal;
};

void main()
{
    float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);
    vec2 texOffset = 1.0 / textureSize(image, 0); // gets size of single texel
    vec3 result = texture(image, v_TexCoord).rgb * weight[0];
    if(u_Horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
           result += texture(image, v_TexCoord + vec2(texOffset.x * i, 0.0)).rgb * weight[i];
           result += texture(image, v_TexCoord - vec2(texOffset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, v_TexCoord + vec2(0.0, texOffset.y * i)).rgb * weight[i];
            result += texture(image, v_TexCoord - vec2(0.0, texOffset.y * i)).rgb * weight[i];
        }
    }
    o_FragColor = vec4(result, 1.0);
}