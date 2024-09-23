#type vertex
#version 450 core
layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec3  a_Normal;
layout(location = 2) in vec2  a_TexCoords;
layout(location = 3) in vec3  a_Tangent;
layout(location = 4) in vec3  a_Bitangent;
layout(location = 5) in float a_DiffuseIndex;
layout(location = 6) in float a_SpecularIndex;
layout(location = 7) in float a_NormalIndex;
layout(location = 8) in float a_ParallaxIndex;
layout(location = 9) in int   a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	// projection * view
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec3 Position;
	vec2 TexCoords;
    vec3 Normal;
	mat3 TBN;
};

layout (location = 0) out flat float v_DiffuseIndex;
layout (location = 1) out flat float v_SpecularIndex;
layout (location = 2) out flat float v_NormalIndex;
layout (location = 3) out flat float v_ParallaxIndex;
layout (location = 4) out flat int   v_EntityID;
layout (location = 5) out VertexOutput Output;

void main()
{
	vec3 T = normalize(a_Tangent);
    vec3 B = normalize(a_Bitangent);
    vec3 N = normalize(a_Normal);
    mat3 TBN = mat3(T, B, N);
    
	v_DiffuseIndex   = a_DiffuseIndex;
	v_SpecularIndex  = a_SpecularIndex;
	v_NormalIndex    = a_NormalIndex;
	v_ParallaxIndex  = a_ParallaxIndex;
	v_EntityID       = a_EntityID;
	Output.Position  = a_Position;
	Output.TexCoords = a_TexCoords;
	Output.Normal    = N;
	Output.TBN       = TBN;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout (location = 0) out vec4 g_PositionDepth;
layout (location = 1) out vec4 g_Normal;
layout (location = 2) out vec4 g_Diffuse;
layout (location = 3) out vec4 g_Specular;

struct VertexOutput
{
	vec3 Position;
	vec2 TexCoords;
    vec3 Normal;
	mat3 TBN;
};

layout (location = 0) in flat float v_DiffuseIndex;
layout (location = 1) in flat float v_SpecularIndex;
layout (location = 2) in flat float v_NormalIndex;
layout (location = 3) in flat float v_ParallaxIndex;
layout (location = 4) in flat int v_EntityID;
layout (location = 5) in VertexOutput Input;

layout (binding = 0)  uniform sampler2D u_Textures[30];

layout(std140, binding = 1) uniform CameraPosition
{
	vec3 u_CameraPosition;
};

void main()
{    
	// ���������ͼ
	vec3 viewDirection = normalize(u_CameraPosition - Input.Position);
	vec2 texCoords = Input.TexCoords;
	if(v_ParallaxIndex != 0)
	{

	    // ���ӽ�ת��Ϊ���߿ռ䣨TBN����������ת�õ������棩
	    vec3 viewDir = normalize(transpose(Input.TBN) * viewDirection);

		const float minLayers = 8;
        const float maxLayers = 32;
		// viewDir����z����ĵ�ˣ�ʹ�����Ľ���������ǿ������ĽǶȵ���������������z����������߿ռ��еı���ķ��ߣ�
        float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  //abs�����ֵ
        float layerDepth = 1.0 / numLayers;
        float currentLayerDepth = 0.0;
        vec2 P = viewDir.xy / viewDir.z * 0.1; 
        vec2 deltaTexCoords = P / numLayers;

        vec2  currentTexCoords     = texCoords;
        float currentDepthMapValue = texture(u_Textures[int(v_ParallaxIndex)], currentTexCoords).r;
          
        while(currentLayerDepth < currentDepthMapValue)
        {
            currentTexCoords -= deltaTexCoords;
            currentDepthMapValue = texture(u_Textures[int(v_ParallaxIndex)], currentTexCoords).r;  
            currentLayerDepth += layerDepth;  
        }

        // get texture coordinates before collision (reverse operations)
        vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
	    
        // get depth after and before collision for linear interpolation
        float afterDepth  = currentDepthMapValue - currentLayerDepth;
        float beforeDepth = texture(u_Textures[int(v_ParallaxIndex)], prevTexCoords).r - currentLayerDepth + layerDepth;
 	    
        // interpolation of texture coordinates
        float weight = afterDepth / (afterDepth - beforeDepth);
        vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

        texCoords = finalTexCoords;

	    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
            discard;
	}

	const float near = 0.1; // ͶӰ����Ľ�ƽ��
    const float far = 1000.0; // ͶӰ�����Զƽ��
	// Note that gl_FragCoord.z ranges from [0,1] instead of up to 'far plane distance' since we divide by 'far'
	float z = gl_FragCoord.z * 2.0 - 1.0;  // Back to NDC [-1,1]
	z = (2.0 * near * far) / (far + near - z * (far - near));// ת����[near, far]
	//z =   (far + near - z * (2.0 * near * far)) / (far - near);
    g_PositionDepth = vec4(Input.Position, z);
    //g_PositionDepth = vec4(vec3(z), 1.0);

    // ͬ���洢��ÿ����Ƭ�η��ߵ�G������
	vec3 normal = vec3(0.0);
	if(v_NormalIndex != 0.0)
	{
	    vec4 materialNormal = texture(u_Textures[int(v_NormalIndex)], texCoords);
	    // ��������ͼ�����߿ռ�ת��Ϊ����ռ�
	    normal = normalize(materialNormal.rgb * 2.0 - 1.0);
	    normal = normalize(Input.TBN * normal);
	}
	else
	{
	    normal = Input.Normal;
	}
    g_Normal = vec4(normal, float(v_EntityID));

    g_Diffuse = texture(u_Textures[int(v_DiffuseIndex)], texCoords);
    g_Specular = texture(u_Textures[int(v_SpecularIndex)], texCoords);

}  