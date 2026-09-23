#type vertex
#version 450 core
layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;

struct VertexOutput
{
	vec2 texCoord;
};

layout (location = 0) out VertexOutput Output;

void main()
{
	Output.texCoord = a_TexCoord;

	gl_Position = vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout (location = 0) out vec4 o_FragColor;
layout (location = 1) out int  o_EntityID;

struct VertexOutput
{
	vec2 texCoord;
};

layout (location = 0) in VertexOutput Input;

layout (binding = 0)  uniform sampler2D  u_ScreenTexture;
layout (binding = 1)  uniform isampler2D u_EntityID;

layout(std140, binding = 13) uniform PostProcessing
{
    uint u_PostProcessingFlags;
    float u_Kernel0;
    float u_Kernel1;
    float u_Kernel2;
    float u_Kernel3;
    float u_Kernel4;
    float u_Kernel5;
    float u_Kernel6;
    float u_Kernel7;
    float u_Kernel8;
};

void main()
{

	vec3 result = texture(u_ScreenTexture, Input.texCoord).rgb;

	if ((u_PostProcessingFlags & (1 << 2)) != 0)
	{
	    vec2 offset = 1.0 / textureSize(u_ScreenTexture, 0);
	
	    vec2 offsets[9] = vec2[](
            vec2(-offset.x,  offset.y), // 左上
            vec2( 0.0f,      offset.y), // 正上
            vec2( offset.x,  offset.y), // 右上
            vec2(-offset.x,  0.0f),     // 左
            vec2( 0.0f,      0.0f),     // 中
            vec2( offset.x,  0.0f),     // 右
            vec2(-offset.x, -offset.y), // 左下
            vec2( 0.0f,     -offset.y), // 正下
            vec2( offset.x, -offset.y)  // 右下
        );
	    vec3 sampleTex[9];
        for(int i = 0; i < 9; i++)
        {
            sampleTex[i] = vec3(texture(u_ScreenTexture, Input.texCoord.st + offsets[i]));
        }
        vec3 col = vec3(0.0);
        
		col += sampleTex[0] * u_Kernel0;
		col += sampleTex[1] * u_Kernel1;
		col += sampleTex[2] * u_Kernel2;
		col += sampleTex[3] * u_Kernel3;
		col += sampleTex[4] * u_Kernel4;
		col += sampleTex[5] * u_Kernel5;
		col += sampleTex[6] * u_Kernel6;
		col += sampleTex[7] * u_Kernel7;
		col += sampleTex[8] * u_Kernel8;

        result = col;
	}
    
	if ((u_PostProcessingFlags & (1 << 0)) != 0)
	{
        result = 1.0 - result;
	}
	
	if ((u_PostProcessingFlags & (1 << 1)) != 0)
	{
	    float average = 0.2126 * result.r + 0.7152 * result.g + 0.0722 * result.b;
        result = vec3(average);
	}
    
    const float gamma = 2.2;
    result = pow(result, vec3(1.0 / gamma));
    o_FragColor = vec4(result, 1.0f);
    
	o_EntityID = texture(u_EntityID, Input.texCoord).r;

}