#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

struct VertexOutput
{
	vec2 texCoord;
};

layout (location = 0) out VertexOutput Output;

void main()
{
	Output.texCoord = a_TexCoord;

	gl_Position = vec4(a_Position, 1.0f);
}

#type fragment
#version 450 core

layout (location = 0) out vec4 o_Ambient;  // 环境光
layout (location = 1) out vec4 o_Diffuse;  // 漫反射
layout (location = 2) out vec4 o_Specular; // 镜面反射

struct VertexOutput
{
	vec2 texCoord;
};

layout (location = 0) in VertexOutput Input;

layout (binding = 0) uniform sampler2D  g_PositionAndDepth;
layout (binding = 1) uniform sampler2D  g_Normal;
layout (binding = 2) uniform isampler2D g_LightingMode;

layout (binding = 3) uniform sampler2D u_DirectionalLightDepthMap;

layout(std140, binding = 0) uniform CameraData
{
	mat4  u_CameraView;
	mat4  u_CameraProjection;
	vec3  u_CameraPosition;
	float u_CameraNearClip;
	float u_CameraFarClip;
};

struct DirectionalLightData
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	int  shadowEnabled;
};

layout (std430, binding = 2) readonly buffer DirectionalLight
{
    DirectionalLightData u_DirectionalLight;
};

layout (std140, binding = 5) uniform Material
{
	float u_MaterialShininess;
};

layout(std140, binding = 8) uniform DirectionalLightShadowData
{
	mat4 u_LightSpaceMatrix;
};

void main()
{
	int lightingMode = texture(g_LightingMode, Input.texCoord).r;
	if (lightingMode != 1)
	    discard;

	vec3 fragPosition     = texture(g_PositionAndDepth, Input.texCoord).rgb;
	vec3 materialNormal   = texture(g_Normal,           Input.texCoord).rgb;
	
    vec3 normal = normalize(materialNormal * 2.0 - 1.0); // [0, 1] -> [-1, 1]
    
	vec3 fragToLightDirection = normalize(-u_DirectionalLight.direction);
    vec3 fragToCamera = u_CameraPosition - fragPosition;
    vec3 fragToCameraDirection = normalize(fragToCamera);
    
    float diff = max(dot(normal, fragToLightDirection), 0.0);

	vec3 halfwayDirection = normalize(fragToLightDirection + fragToCameraDirection);
	float spec = pow(max(dot(normal, halfwayDirection), 0.0f), u_MaterialShininess);
		
    vec3 ambient  = u_DirectionalLight.ambient;
    vec3 diffuse  = u_DirectionalLight.diffuse  * diff;
    vec3 specular = u_DirectionalLight.specular * spec;

    if (u_DirectionalLight.shadowEnabled != 0)
    {
	    vec4 lightSpaceFragPosition = u_LightSpaceMatrix * vec4(fragPosition, 1.0f);
		
		// 片段在光源空间中的位置，并将范围限定在[-1,1](NDC坐标)。
		vec3 projCoords = lightSpaceFragPosition.xyz / lightSpaceFragPosition.w;
		// 范围变换为[0,1]
		// 为了和深度贴图的深度相比较，z分量需要变换到[0,1]；
		// 为了作为从深度贴图中采样的坐标，xy分量也需要变换到[0,1]。
		// 所以整个projCoords向量都需要变换到[0,1]范围。
        projCoords = projCoords * 0.5 + 0.5;
		
		// 最近点的深度
        //float closestDepth = texture(u_DirectionalLightDepthMap, projCoords.xy).r; 
		// 当前片段在光源视角下的深度
        float currentDepth = projCoords.z;
		// 阴影偏移，偏移量的最大值0.0005，和一个最小值0.00005，它们是基于表面法线和光照方向的。
		// 这样像地板这样的表面几乎与光源垂直，得到的偏移就很小；立方体的侧面这种表面得到的偏移就更大
	    float bias = max(0.0005f * (1.0f - dot(normal, -u_DirectionalLight.direction)), 0.0005f);
        //float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
		
        float shadow = 0.0f;
		// textureSize返回指定采样器纹理在0级mipmap的宽高向量，类型为vec2。
		// 取其倒数，即可得到单一纹理像素的大小
        vec2 texelSize = 1.0f / textureSize(u_DirectionalLightDepthMap, 0);
        for(int x = -1; x != 2; x++)
        {
            for(int y = -1; y != 2; y++)
            {
                float pcfDepth = texture(u_DirectionalLightDepthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;        
            }    
        }
        shadow /= 9.0f;
		
		// 解决过采样
	    if(projCoords.z > 1.0f)
            shadow = 0.0f;
		
        diffuse  *= (1.0 - shadow);
        specular *= (1.0 - shadow);
    }
    
    o_Ambient  = vec4(ambient, 1.0);
    o_Diffuse  = vec4(diffuse, 1.0);
    o_Specular = vec4(specular, 1.0);
}