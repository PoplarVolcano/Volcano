#type vertex
#version 450 core
layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoords;

struct VertexOutput
{
	vec2 TexCoords;
};

layout (location = 0) out VertexOutput Output;

void main()
{
	Output.TexCoords = a_TexCoords;
	gl_Position = vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout (location = 0) out float FragColor;

struct VertexOutput
{
	vec2 TexCoords;
};

layout (location = 0) in VertexOutput Input;

layout (binding = 0)  uniform sampler2D ssaoInput;

void main()
{    
    // 返回vec2纹理维度的textureSize,根据纹理单元的真实大小偏移每一个纹理坐标
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoInput, Input.TexCoords + offset).r;
        }
    }
    FragColor = result / (4.0 * 4.0);
}