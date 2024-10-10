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

layout (location = 0) out vec4 FragColor;
layout (location = 1) out int  o_EntityID;
layout (location = 2) out vec4 BrightColor;

struct VertexOutput
{
	vec2 TexCoords;
};

layout (location = 0) in VertexOutput Input;

layout (binding = 0) uniform sampler2D g_Position;
layout (binding = 1) uniform sampler2D g_Normal;
layout (binding = 2) uniform sampler2D g_Albedo;
layout (binding = 3) uniform isampler2D g_EntityID;
layout (binding = 4) uniform sampler2D ssao;

layout (binding = 5) uniform sampler2D u_Ambient;
layout (binding = 6) uniform sampler2D u_Diffuse;
layout (binding = 7) uniform sampler2D u_Specular;


void main()
{
	vec3 Ambient  = texture(u_Ambient,  Input.TexCoords).rgb;
	vec3 Diffuse  = texture(u_Diffuse,  Input.TexCoords).rgb;
	vec3 Specular = texture(u_Specular, Input.TexCoords).rgb;
	
	vec3  fragPosition     = texture(g_Position, Input.TexCoords).rgb;
	float depth            = texture(g_Position, Input.TexCoords).a;
	vec3  materialNormal   = texture(g_Normal,   Input.TexCoords).rgb;
	vec3  materialDiffuse  = texture(g_Albedo,   Input.TexCoords).rgb;
	float materialSpecular = texture(g_Albedo,   Input.TexCoords).a;
	float AmbientOcclusion = texture(ssao,       Input.TexCoords).r;

	if(fragPosition == vec3(0.0))
	    discard;

	Ambient  *= materialDiffuse;
	Diffuse  *= materialDiffuse;  
	Specular *= materialSpecular;

	Ambient *= AmbientOcclusion;

	vec3 lighting = Ambient + Diffuse + Specular;

    FragColor = vec4(lighting, 1.0);

	o_EntityID = texture(g_EntityID, Input.TexCoords).r;

	 // Check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));//转换为灰度来计算片段的亮度
	if(brightness > 1.0)
        BrightColor = FragColor;
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}  