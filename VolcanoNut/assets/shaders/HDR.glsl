#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoords;

layout (location = 0) out vec2 TexCoords;

void main()
{
    gl_Position = vec4(a_Position, 1.0); 
    TexCoords = a_TexCoords;
}

#type fragment
#version 450 core
layout(location = 0) out vec4 FragColor;

layout (location = 0) in vec2 TexCoords;


layout (binding = 0) uniform sampler2D hdrBuffer;
layout (binding = 1) uniform sampler2D bloomBlur;

layout (binding = 11) uniform Exposure
{
    float u_Exposure;
    bool u_Bloom;
};

void main()
{ 
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    if(u_Bloom)
        hdrColor += bloomColor; // additive blending

    // ÆØ¹âÉ«µ÷Ó³Éä
    vec3 result = vec3(1.0) - exp(-hdrColor * u_Exposure);
    //result = pow(result, vec3(1.0 / gamma));

    FragColor = vec4(result, 1.0);

}