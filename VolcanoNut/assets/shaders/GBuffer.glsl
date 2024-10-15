#type vertex
#version 450 core
layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoords;
layout(location = 2) in vec3  a_Normal;
layout(location = 3) in vec3  a_Tangent;
layout(location = 4) in vec3  a_Bitangent;
layout(location = 5) in ivec4 a_BoneIds; 
layout(location = 6) in vec4  a_Weights;
layout(location = 7) in int   a_EntityID;

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

layout (location = 0) out flat int v_EntityID;
layout (location = 1) out VertexOutput Output;

void main()
{
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);

	vec3 T = normalize(a_Tangent);
    vec3 B = normalize(a_Bitangent);
    vec3 N = normalize(a_Normal);
    mat3 TBN = mat3(T, B, N);
    
	v_EntityID       = a_EntityID;
	Output.Position  = a_Position;
	Output.TexCoords = a_TexCoords;
	Output.Normal    = N;
	Output.TBN       = TBN;

}


#type fragment
#version 450 core

layout (location = 0) out vec4 g_PositionDepth;
layout (location = 1) out vec4 g_Normal;
layout (location = 2) out vec4 g_Albedo;
layout (location = 3) out vec4 g_RoughnessAO;
layout (location = 4) out int  g_EntityID;

struct VertexOutput
{
	vec3 Position;
	vec2 TexCoords;
    vec3 Normal;
	mat3 TBN;
};

layout (location = 0) in flat int v_EntityID;
layout (location = 1) in VertexOutput Input;

layout (binding = 0) uniform sampler2D u_Diffuse;
layout (binding = 1) uniform sampler2D u_Specular;
layout (binding = 2) uniform sampler2D u_Normal;
layout (binding = 3) uniform sampler2D u_Parallax;
layout (binding = 4) uniform sampler2D u_Roughness;
layout (binding = 5) uniform sampler2D u_AO;

layout(std140, binding = 1) uniform CameraPosition
{
	vec3 u_CameraPosition;
};

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

layout(std140, binding = 15) uniform BonesMatrices
{
    mat4 u_FinalBonesMatrices[100];
};

// ----------------------------------------------------------------------------
mat3 getTBN()
{
    vec3 Q1  = dFdx(Input.Position);
    vec3 Q2  = dFdy(Input.Position);
    vec2 st1 = dFdx(Input.TexCoords);
    vec2 st2 = dFdy(Input.TexCoords);

    vec3 N  = normalize(Input.Normal);
    vec3 T  = normalize(Q1 * st2.t - Q2 * st1.t); // st2.t => st2.y
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return TBN;
}
// ----------------------------------------------------------------------------

void main()
{
    mat3 TBN;
    if(Input.TBN[1] == vec3(0.0))
        TBN = getTBN();
    else
        TBN = Input.TBN;
	// ���������ͼ
	vec3 viewDirection = normalize(u_CameraPosition - Input.Position);
	vec2 texCoords = Input.TexCoords;
	{
	    // ���ӽ�ת��Ϊ���߿ռ䣨TBN����������ת�õ������棩
	    vec3 viewDir = normalize(transpose(TBN) * viewDirection);

		const float minLayers = 8;
        const float maxLayers = 32;
		// viewDir����z����ĵ�ˣ�ʹ�����Ľ���������ǿ������ĽǶȵ���������������z����������߿ռ��еı���ķ��ߣ�
        float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  //abs�����ֵ
        float layerDepth = 1.0 / numLayers;
        float currentLayerDepth = 0.0;
        vec2 P = viewDir.xy / viewDir.z * 0.1; 
        vec2 deltaTexCoords = P / numLayers;

        vec2  currentTexCoords     = texCoords;
        float currentDepthMapValue = texture(u_Parallax, currentTexCoords).r;
          
        while(currentLayerDepth < currentDepthMapValue)
        {
            currentTexCoords -= deltaTexCoords;
            currentDepthMapValue = texture(u_Parallax, currentTexCoords).r;  
            currentLayerDepth += layerDepth;  
        }

        // get texture coordinates before collision (reverse operations)
        vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
	    
        // get depth after and before collision for linear interpolation
        float afterDepth  = currentDepthMapValue - currentLayerDepth;
        float beforeDepth = texture(u_Parallax, prevTexCoords).r - currentLayerDepth + layerDepth;
 	    
        // interpolation of texture coordinates
        float weight = afterDepth / (afterDepth - beforeDepth);
        vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

        texCoords = finalTexCoords;

	    //if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        //    discard;
	}

	
	vec3  materialNormal    = texture(u_Normal,    texCoords).rgb;
	vec4  materialDiffuse   = texture(u_Diffuse,   texCoords);
	float materialSpecular  = texture(u_Specular,  texCoords).r;
    float materialRoughness = texture(u_Roughness, texCoords).r;
    float materialAO        = texture(u_AO,        texCoords).r;

	if(materialNormal == vec3(0.0) && materialDiffuse.a == 0.0 && materialSpecular == 0.0)
	    discard;

	const float near = 0.1; // ͶӰ����Ľ�ƽ��
    const float far = 1000.0; // ͶӰ�����Զƽ��
	// Note that gl_FragCoord.z ranges from [0,1] instead of up to 'far plane distance' since we divide by 'far'
	float z = gl_FragCoord.z * 2.0 - 1.0;  // Back to NDC [-1,1]
	z = (2.0 * near * far) / (far + near - z * (far - near));// ת����[near, far]
    g_PositionDepth = vec4(Input.Position, z);

    // ͬ���洢��ÿ����Ƭ�η��ߵ�G������
	vec3 normal = vec3(0.0);
	if(materialNormal == vec3(0.0))
	    normal = Input.Normal;
    else
	{
	    // ��������ͼ�����߿ռ�ת��Ϊ����ռ�
	    normal = normalize(materialNormal * 2.0 - 1.0); // [-1, 1]
	    normal = normalize(TBN * normal);
	}
    normal = (normal + 1.0) / 2.0;// [0, 1]
    g_Normal = vec4(normal, 1.0);
    g_Albedo = vec4(materialDiffuse.rgb, materialSpecular);
	g_RoughnessAO = vec4(materialRoughness, materialAO, 1.0, 1.0);
    g_EntityID = v_EntityID;

}  