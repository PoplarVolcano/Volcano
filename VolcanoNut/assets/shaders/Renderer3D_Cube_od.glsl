#type vertex
#version 450 core

// 传入的position和normal已经在C++中完成了model变换
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec2 a_TexCoord;
layout(location = 4) in float a_DiffuseIndex;
layout(location = 5) in float a_SpecularIndex;
layout(location = 6) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	// projection * view
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec3 Position;
	vec3 Normal;
	vec4 Color;
	vec2 TexCoord;
};

layout (location = 0) out flat float v_DiffuseIndex;
layout (location = 1) out flat float v_SpecularIndex;
layout (location = 2) out flat int v_EntityID;
layout (location = 3) out VertexOutput Output;

void main()
{
	v_DiffuseIndex = a_DiffuseIndex;
	v_SpecularIndex = a_SpecularIndex;
	v_EntityID = a_EntityID;
	Output.Position = a_Position;
	Output.Normal = a_Normal;
	Output.Color = a_Color;
	Output.TexCoord = a_TexCoord;

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
	vec2 TexCoord;
};

layout (location = 0) in flat float v_DiffuseIndex;
layout (location = 1) in flat float v_SpecularIndex;
layout (location = 2) in flat int v_EntityID;
layout (location = 3) in VertexOutput Input;

layout (binding = 0) uniform sampler2D u_Textures[32];

layout(std140, binding = 1) uniform CameraPosition
{
	vec3 u_CameraPosition;
};

layout (std140, binding = 2) uniform DirectionalLightData
{
	vec3 u_DirectionalLightDirection;
	vec3 u_DirectionalLightAmbient;
	vec3 u_DirectionalLightDiffuse;
	vec3 u_DirectionalLightSpecular;
};

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

layout (std140, binding = 3) uniform PointLightData
{
	vec3 u_PointLightPosition;
	vec3 u_PointLightAmbient;
	vec3 u_PointLightDiffuse;
	vec3 u_PointLightSpecular;
	float u_PointLightConstant;
    float u_PointLightLinear;
    float u_PointLightQuadratic;
};

struct PointLight {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant;
    float linear;
    float quadratic;
};  

layout (std140, binding = 4) uniform SpotLightData
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

struct SpotLight {
	vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant;
    float linear;
    float quadratic;
	float cutOff;
	float outerCutOff;
};  

layout (std140, binding = 5) uniform Material
{
	float u_MaterialShininess;
};

//vec3 CalculateDirectionalLight(DirectionalLight light, vec4 materialDiffuse, vec4 materialSpecular, vec3 normal, vec3 viewDirection);
//vec3 CalculatePointLight(PointLight light);//, vec4 materialDiffuse, vec4 materialSpecular, vec3 fragmentPosition, vec3 normal, vec3 viewDirection);

void main()
{
	// 漫反射贴图
	int diffuseIndex = int(v_DiffuseIndex);
	vec4 materialDiffuse = Input.Color;
	materialDiffuse *= texture(u_Textures[diffuseIndex], Input.TexCoord);

	if (materialDiffuse.a == 0.0)
		discard;
		
	// 镜面光贴图
	int specularIndex = int(v_SpecularIndex);
	vec4 materialSpecular = vec4(1.0, 1.0, 1.0, 1.0);
	materialSpecular *= texture(u_Textures[specularIndex], Input.TexCoord);

	vec3 normal = normalize(Input.Normal);
	vec3 viewDirection = normalize(u_CameraPosition - Input.Position);
	
	vec3 result = vec3(0.0, 0.0, 0.0);

	DirectionalLight directionalLight = DirectionalLight(u_DirectionalLightDirection, u_DirectionalLightAmbient, u_DirectionalLightDiffuse, u_DirectionalLightSpecular);
	PointLight pointLight = PointLight(u_PointLightPosition, u_PointLightAmbient, u_PointLightDiffuse, u_PointLightSpecular, u_PointLightConstant, u_PointLightLinear, u_PointLightQuadratic);
	SpotLight spotLight = SpotLight(u_SpotLightPosition, u_SpotLightDirection, u_SpotLightAmbient, u_SpotLightDiffuse, u_SpotLightSpecular, u_SpotLightConstant, u_SpotLightLinear, u_SpotLightQuadratic, u_SpotLightCutOff, u_SpotLightOuterCutOff);

	//vec3 calculateDirectionalLight = CalculateDirectionalLight(directionalLight, materialDiffuse, materialSpecular, normal, viewDirection);
	//vec3 calculatePointLight = CalculatePointLight(pointLight);//, materialDiffuse, materialSpecular, Input.Position, normal, viewDirection);
	//result += calculateDirectionalLight;

	
	// DirectionalLight
	{
		// 片段位置指向光源的向量
		vec3 lightDirection = normalize(-directionalLight.direction);
		// 向量间角度大于90度，点乘结果为负数，导致漫反射分量变为负数。
		float diff = max(dot(normal, lightDirection), 0.0);
	
		// 第一个参数为从光源指向片段位置的向量
		vec3 reflectDirection = reflect(-lightDirection, normal);
		// pow方法接受两个参数,第一个参数是底数,第二个参数是指数。它返回底数的指定次幂的值。
		// 高光的反光度(Shininess)。一个物体的反光度越高，反射光的能力越强，散射得越少，高光点就会越小。
		float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), u_MaterialShininess);
	
		vec3 ambient  = directionalLight.ambient  * materialDiffuse.rgb;
		vec3 diffuse  = directionalLight.diffuse  * diff * materialDiffuse.rgb;
		vec3 specular = directionalLight.specular * spec * materialSpecular.rgb;

		result += (ambient + diffuse + specular);
	}
	
	// PointLight
	{
		vec3 lightDirection = normalize(pointLight.position - Input.Position);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		vec3 reflectDirection = reflect(-lightDirection, normal);
		float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), u_MaterialShininess);
		
		// 衰减
		float distance    = length(pointLight.position - Input.Position);
		float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distance +  pointLight.quadratic * (distance * distance));
		
	    vec3 ambient  = pointLight.ambient  * materialDiffuse.rgb;
		vec3 diffuse  = pointLight.diffuse  * diff * materialDiffuse.rgb;
		vec3 specular = pointLight.specular * spec * materialSpecular.rgb;
	
	    ambient  *= attenuation;
		diffuse  *= attenuation;
		specular *= attenuation;
	
	    result += (ambient + diffuse + specular);
	}

	
	// SpotLight
	{
		vec3 lightDirection = normalize(spotLight.position - Input.Position);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		vec3 reflectDirection = reflect(-lightDirection, normal);  
		float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), u_MaterialShininess);
		
		// 衰减attenuation
		float distance    = length(spotLight.position - Input.Position);
		float attenuation = 1.0 / (spotLight.constant + spotLight.linear * distance + spotLight.quadratic * (distance * distance));    
		
		// spotlight (soft edges)
		float theta = dot(-lightDirection, normalize(spotLight.direction)); 
        float epsilon = (spotLight.cutOff - spotLight.outerCutOff);
        float intensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);
		
	    vec3 ambient  = spotLight.ambient  * materialDiffuse.rgb;
		vec3 diffuse  = spotLight.diffuse  * diff * materialDiffuse.rgb;  
		vec3 specular = spotLight.specular * spec * materialSpecular.rgb;  
		
		ambient  *= intensity * attenuation;
        diffuse  *= intensity * attenuation;
        specular *= intensity * attenuation;
		
	    result += (ambient + diffuse + specular);
	}
	

    o_Color = vec4(result, materialDiffuse.a);
	o_EntityID = v_EntityID;
}

/*
vec3 CalculateDirectionalLight(DirectionalLight light, vec4 materialDiffuse, vec4 materialSpecular, vec3 normal, vec3 viewDirection)
{
	// 片段位置指向光源的向量
	vec3 lightDirection = normalize(-light.direction);
	// 向量间角度大于90度，点乘结果为负数，导致漫反射分量变为负数。
	float diff = max(dot(normal, lightDirection), 0.0);
	
	// 第一个参数为从光源指向片段位置的向量
	vec3 reflectDirection = reflect(-lightDirection, normal);
	// pow方法接受两个参数,第一个参数是底数,第二个参数是指数。它返回底数的指定次幂的值。
	// 高光的反光度(Shininess)。一个物体的反光度越高，反射光的能力越强，散射得越少，高光点就会越小。
	float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), u_MaterialShininess);
	
    vec3 ambient  = light.ambient  * materialDiffuse.rgb;
	vec3 diffuse  = light.diffuse  * diff * materialDiffuse.rgb;
	vec3 specular = light.specular * spec * materialSpecular.rgb;

	return (ambient + diffuse + specular);
}
*/
//vec3 CalculatePointLight(PointLight light)//, vec4 materialDiffuse, vec4 materialSpecular, vec3 fragmentPosition, vec3 normal, vec3 viewDirection)





	