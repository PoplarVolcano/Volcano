#type vertex
#version 450 core

// 传入的position和normal已经在C++中完成了model变换
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec4 a_Color;
layout(location = 5) in vec2 a_TexCoords;
layout(location = 6) in float a_DiffuseIndex;
layout(location = 7) in float a_SpecularIndex;
layout(location = 8) in float a_NormalIndex;
layout(location = 9) in float a_ParallaxIndex;
layout(location = 10) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	// projection * view
	mat4 u_ViewProjection;
};

layout(std140, binding = 8) uniform LightSpaceMatrix
{
	mat4 u_LightSpaceMatrix;
};

struct VertexOutput
{
	vec3 Position;
	vec3 Normal;
	vec4 Color;
	vec2 TexCoords;
	mat3 TBN;
    vec4 PositionLightSpace;
};

layout (location = 0) out flat float v_DiffuseIndex;
layout (location = 1) out flat float v_SpecularIndex;
layout (location = 2) out flat float v_NormalIndex;
layout (location = 3) out flat float v_ParallaxIndex;
layout (location = 4) out flat int v_EntityID;
layout (location = 5) out VertexOutput Output;

void main()
{
	vec3 T = normalize(a_Tangent);
    vec3 B = normalize(a_Bitangent);
    vec3 N = normalize(a_Normal);
    mat3 TBN = mat3(T, B, N);

	v_DiffuseIndex  = a_DiffuseIndex;
	v_SpecularIndex = a_SpecularIndex;
	v_NormalIndex   = a_NormalIndex;
	v_ParallaxIndex = a_ParallaxIndex;
	v_EntityID      = a_EntityID;

	Output.Position = a_Position;
	Output.Normal = a_Normal;
	Output.Color = a_Color;
	Output.TexCoords = a_TexCoords;
	Output.TBN = TBN;
	Output.PositionLightSpace = u_LightSpaceMatrix * vec4(Output.Position, 1.0);


	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

struct VertexOutput
{
	vec3 Position;
	vec3 Normal;
	vec4 Color;
	vec2 TexCoords;
	mat3 TBN;
    vec4 PositionLightSpace;
};

layout (location = 0) in flat float v_DiffuseIndex;
layout (location = 1) in flat float v_SpecularIndex;
layout (location = 2) in flat float v_NormalIndex;
layout (location = 3) in flat float v_ParallaxIndex;
layout (location = 4) in flat int v_EntityID;
layout (location = 5) in VertexOutput Input;

layout (binding = 0)  uniform sampler2D u_Textures[30];
layout (binding = 30) uniform sampler2D u_SpotDepthMap;
layout (binding = 31) uniform sampler2D u_DepthMap;
layout (binding = 0)  uniform samplerCube u_DepthCubeMap;

layout(std140, binding = 1) uniform CameraPosition
{
	vec3 u_CameraPosition;
};


layout (std140, binding = 2) uniform DirectionalLight
{
	vec3 u_DirectionalLightDirection;
	vec3 u_DirectionalLightAmbient;
	vec3 u_DirectionalLightDiffuse;
	vec3 u_DirectionalLightSpecular;
};

layout (std140, binding = 3) uniform PointLight
{
	vec3 u_PointLightPosition;
	vec3 u_PointLightAmbient;
	vec3 u_PointLightDiffuse;
	vec3 u_PointLightSpecular;
	float u_PointLightConstant;
    float u_PointLightLinear;
    float u_PointLightQuadratic;
};


layout (std140, binding = 4) uniform SpotLight
{
	vec3 u_SpotLightPosition;
	vec3 u_SpotLightDirection;
	vec3 u_SpotLightAmbient;
	vec3 u_SpotLightDiffuse;
	vec3 u_SpotLightSpecular;
	float u_SpotLightConstant;
    float u_SpotLightLinear;
    float u_SpotLightQuadratic;
	float u_SpotLightCutOff;
	float u_SpotLightOuterCutOff;
};

layout (std140, binding = 5) uniform Material
{
	float u_MaterialShininess;
};

layout(std140, binding = 9) uniform PointLightShadow
{
    mat4 u_ShadowMatrices[6];
    float u_PointFarPlane;
};

layout(std140, binding = 10) uniform SpotLightShadow
{
    mat4 u_SpotLightSpace;
    float u_SpotFarPlane;
};

void main()
{
	vec3 Ambient  = vec3(0.0, 0.0, 0.0);
	vec3 Diffuse  = vec3(0.0, 0.0, 0.0);
	vec3 Specular = vec3(0.0, 0.0, 0.0);
	
	vec3 viewDirection = normalize(u_CameraPosition - Input.Position);
	vec2 texCoords = Input.TexCoords;
	
	// 计算深度贴图
	if(v_ParallaxIndex != 0)
	{

	    // 将视角转换为切线空间（TBN是正交矩阵，转置等于求逆）
	    vec3 viewDir = normalize(transpose(Input.TBN) * viewDirection);

		const float minLayers = 8;
        const float maxLayers = 32;
		// viewDir和正z方向的点乘，使用它的结果根据我们看向表面的角度调整样本数量（正z方向等于切线空间中的表面的法线）
        float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  //abs求绝对值
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

	vec4 materialDiffuse  = texture(u_Textures[int(v_DiffuseIndex)],  texCoords) * Input.Color;
	vec4 materialSpecular = texture(u_Textures[int(v_SpecularIndex)], texCoords);
	vec4 materialNormal   = texture(u_Textures[int(v_NormalIndex)],   texCoords);
	
	vec3 normal = vec3(0.0);
	if(v_NormalIndex == 0.0)
	    normal = normalize(Input.Normal);
	else
	{
	    // 将法线贴图从切线空间转换为世界空间
	    normal = normalize(materialNormal.rgb * 2.0 - 1.0);
	    normal = normalize(Input.TBN * normal);
	}
	

	// DirectionalLight
	{
		vec3 lightDirection = normalize(-u_DirectionalLightDirection);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		//vec3 reflectDirection = reflect(-lightDirection, normal);
		//float spec = pow(max(dot(normal, reflectDirection), 0.0), u_MaterialShininess);
		vec3 halfwayDirection = normalize(lightDirection + viewDirection);
		float spec = pow(max(dot(normal, halfwayDirection), 0.0), u_MaterialShininess);
		
		vec3 ambient  = u_DirectionalLightAmbient  * materialDiffuse.rgb;
		vec3 diffuse  = u_DirectionalLightDiffuse  * diff * materialDiffuse.rgb;
		vec3 specular = u_DirectionalLightSpecular * spec * materialSpecular.rgb;
		
        vec3 projCoords = Input.PositionLightSpace.xyz / Input.PositionLightSpace.w;
        projCoords = projCoords * 0.5 + 0.5;
        float closestDepth = texture(u_DepthMap, projCoords.xy).r; 
        float currentDepth = projCoords.z;
	    float bias = max(0.005 * (1.0 - dot(normal, -u_DirectionalLightDirection)), 0.0005);
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	    if(projCoords.z > 1.0)
            shadow = 0.0;
		
		Ambient  += ambient;
		Diffuse  += (1.0 - shadow) * diffuse;
		Specular += (1.0 - shadow) * specular;
	}

	// PointLight
	{
		vec3 lightDirection = normalize(u_PointLightPosition - Input.Position);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		//vec3 reflectDirection = reflect(-lightDirection, normal);
		//float spec = pow(max(dot(normal, reflectDirection), 0.0), u_MaterialShininess);
		vec3 halfwayDirection = normalize(lightDirection + viewDirection);
		float spec = pow(max(dot(normal, halfwayDirection), 0.0), u_MaterialShininess);
		
		float distance = length(u_PointLightPosition - Input.Position);
		float attenuation = 1.0 / (u_PointLightConstant + u_PointLightLinear * distance +  u_PointLightQuadratic * (distance * distance));
		
	    vec3 ambient  = u_PointLightAmbient  * materialDiffuse.rgb;
		vec3 diffuse  = u_PointLightDiffuse  * diff * materialDiffuse.rgb;
		vec3 specular = u_PointLightSpecular * spec * materialSpecular.rgb;
	
	    ambient  *= attenuation;
		diffuse  *= attenuation;
		specular *= attenuation;
	
	    /*
	    // TODO：点光源的立方体贴图读取无法顺利进行，还需要优化
	    vec3 fragToLight = Input.Position - u_PointLightPosition;
        float currentDepth = length(fragToLight);
        float closestDepth = texture(u_DepthCubeMap, fragToLight).r;
        closestDepth *= u_PointFarPlane;
        float bias = 0.05;
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
		*/

		float shadow = 0;

		Ambient  += ambient;
		Diffuse  += (1.0 - shadow) * diffuse;
		Specular += (1.0 - shadow) * specular;
	}

	// SpotLight
	{
		vec3 lightDirection = normalize(u_SpotLightPosition - Input.Position);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		//vec3 reflectDirection = reflect(-lightDirection, normal);
		//float spec = pow(max(dot(normal, reflectDirection), 0.0), u_MaterialShininess);
		vec3 halfwayDirection = normalize(lightDirection + viewDirection);
		float spec = pow(max(dot(normal, halfwayDirection), 0.0), u_MaterialShininess);
		
		float distance    = length(u_SpotLightPosition - Input.Position);
		float attenuation = 1.0 / (u_SpotLightConstant + u_SpotLightLinear * distance + u_SpotLightQuadratic * (distance * distance));    
		
		// spotlight (soft edges)
		float theta = dot(-lightDirection, normalize(u_SpotLightDirection)); 
        float epsilon = (u_SpotLightCutOff - u_SpotLightOuterCutOff);
        float intensity = clamp((theta - u_SpotLightOuterCutOff) / epsilon, 0.0, 1.0);
		
		vec3 ambient  = u_SpotLightAmbient  * materialDiffuse.rgb;
		vec3 diffuse  = u_SpotLightDiffuse  * diff * materialDiffuse.rgb;  
		vec3 specular = u_SpotLightSpecular * spec * materialSpecular.rgb;

		ambient  *= intensity * attenuation;
        diffuse  *= intensity * attenuation;
        specular *= intensity * attenuation;
		
	    vec4 positionLightSpace = u_SpotLightSpace * vec4(Input.Position, 1.0);
        vec3 projCoords = positionLightSpace.xyz / positionLightSpace.w;
        projCoords = projCoords * 0.5 + 0.5;
        float closestDepth = texture(u_SpotDepthMap, projCoords.xy).r; 
        float currentDepth = projCoords.z;
	    float bias = max(0.005 * (1.0 - dot(normal, -u_SpotLightDirection)), 0.0005);
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

		Ambient  += ambient;
		Diffuse  += (1.0 - shadow) * diffuse;
		Specular += (1.0 - shadow) * specular;

	}

	/*
	// 柔化点光源阴影
	vec3 fragToLight = Input.Position - u_PointLightPosition;
    float currentDepth = length(fragToLight);
	float shadow = 0.0;
    float bias = 0.05; 
    float samples = 4.0;
    float offset = 0.1;
    for(float x = -offset; x < offset; x += offset / (samples * 0.5))
    {
        for(float y = -offset; y < offset; y += offset / (samples * 0.5))
        {
            for(float z = -offset; z < offset; z += offset / (samples * 0.5))
            {
                float closestDepth = texture(u_DepthCubeMap, fragToLight + vec3(x, y, z)).r; 
                closestDepth *= u_PointFarPlane;
                if(currentDepth - bias > closestDepth)
                    shadow += 1.0;
            }
        }
    }
    shadow /= (samples * samples * samples);
    vec3 lighting = (Ambient + (1.0 - shadow) * (Diffuse + Specular));
	*/



	vec3 lighting = Ambient + Diffuse + Specular;

    o_Color = vec4(lighting, materialDiffuse.a);

	o_EntityID = v_EntityID;
}
