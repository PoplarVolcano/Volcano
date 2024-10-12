#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

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

layout (location = 0) out vec4 o_Ambient;
layout (location = 1) out vec4 o_Diffuse;
layout (location = 2) out vec4 o_Specular;

struct VertexOutput
{
	vec2 TexCoords;
};

layout (location = 0) in VertexOutput Input;

layout (binding = 0) uniform sampler2D g_Position;
layout (binding = 1) uniform sampler2D g_Normal;
layout (binding = 2) uniform sampler2D g_Albedo;

layout (binding = 3) uniform sampler2D u_Ambient;
layout (binding = 4) uniform sampler2D u_Diffuse;
layout (binding = 5) uniform sampler2D u_Specular;

layout (binding = 6) uniform sampler2D u_DirectionalDepthMap;
layout (binding = 7) uniform samplerCube u_PointDepthMap;
layout (binding = 8) uniform sampler2D u_SpotDepthMap;

layout(std140, binding = 1) uniform CameraPosition
{
	vec3 u_CameraPosition;
};

struct DirectionalLightData
{
    vec3 Position;
	vec3 Direction;
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};

layout (std140, binding = 2) uniform DirectionalLight
{
    DirectionalLightData u_DirectionalLight;
};

struct PointLightData
{
    vec3  Position;
	vec3  Ambient;
	vec3  Diffuse;
	vec3  Specular;
	float Constant;
    float Linear;
    float Quadratic;
};

layout (std140, binding = 3) uniform PointLight
{
    PointLightData u_PointLight;
};

struct SpotLightData
{
    vec3  Position;
	vec3  Direction;
	vec3  Ambient;
	vec3  Diffuse;
	vec3  Specular;
	float Constant;
    float Linear;
    float Quadratic;
	float CutOff;
	float OuterCutOff;
};

layout (std140, binding = 4) uniform SpotLight
{
	SpotLightData u_SpotLight;
};

layout (std140, binding = 5) uniform Material
{
	float u_MaterialShininess;
};

layout(std140, binding = 8) uniform DirectionalLightShadow
{
	mat4 u_DirectionalLightSpace;
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
	vec3  fragPosition     = texture(g_Position, Input.TexCoords).rgb;
	float depth            = texture(g_Position, Input.TexCoords).a;
	vec3  materialNormal   = texture(g_Normal,   Input.TexCoords).rgb;
	vec3  materialDiffuse  = texture(g_Albedo,   Input.TexCoords).rgb;
	float materialSpecular = texture(g_Albedo,   Input.TexCoords).a;

	vec3 Ambient  = texture(u_Ambient,  Input.TexCoords).rgb;
	vec3 Diffuse  = texture(u_Diffuse,  Input.TexCoords).rgb;
	vec3 Specular = texture(u_Specular, Input.TexCoords).rgb;

	if(fragPosition == vec3(0.0))
	    discard;

	vec3 viewDirection = normalize(u_CameraPosition - fragPosition);
	vec3 normal = materialNormal * 2.0 - 1.0;

	// DirectionalLight
	if (u_DirectionalLight.Direction != vec3(0.0))
	{
		vec3 lightDirection = normalize(-u_DirectionalLight.Direction);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		//vec3 reflectDirection = reflect(-lightDirection, normal);
		//float spec = pow(max(dot(normal, reflectDirection), 0.0), u_MaterialShininess);
		vec3 halfwayDirection = normalize(lightDirection + viewDirection);
		float spec = pow(max(dot(normal, halfwayDirection), 0.0), u_MaterialShininess);
		
		vec3 ambient  = u_DirectionalLight.Ambient;
		vec3 diffuse  = u_DirectionalLight.Diffuse  * diff;
		vec3 specular = u_DirectionalLight.Specular * spec;
		
	    vec4 lightSpacePosition = u_DirectionalLightSpace * vec4(fragPosition, 1.0);
        vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;
        projCoords = projCoords * 0.5 + 0.5;
        float closestDepth = texture(u_DirectionalDepthMap, projCoords.xy).r; 
        float currentDepth = projCoords.z;
	    float bias = max(0.005 * (1.0 - dot(normal, -u_DirectionalLight.Direction)), 0.0005);
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	    if(projCoords.z > 1.0)
            shadow = 0.0;
		
		Ambient  += ambient;
		Diffuse  += (1.0 - shadow) * diffuse;
		Specular += (1.0 - shadow) * specular;
	}

	// PointLight
	if (u_PointLight.Position != vec3(0.0))
	{
		vec3 lightDirection = normalize(u_PointLight.Position - fragPosition);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		vec3 halfwayDirection = normalize(lightDirection + viewDirection);
		float spec = pow(max(dot(normal, halfwayDirection), 0.0), u_MaterialShininess);
		
		float distance = length(u_PointLight.Position - fragPosition);
		float attenuation = 1.0 / (u_PointLight.Constant + u_PointLight.Linear * distance +  u_PointLight.Quadratic * (distance * distance));
		
	    vec3 ambient  = u_PointLight.Ambient;
		vec3 diffuse  = u_PointLight.Diffuse  * diff;
		vec3 specular = u_PointLight.Specular * spec;
	
	    ambient  *= attenuation;
		diffuse  *= attenuation;
		specular *= attenuation;
	
	    
	    vec3 fragToLight = fragPosition - u_PointLight.Position;
        float currentDepth = length(fragToLight);
        float closestDepth = texture(u_PointDepthMap, fragToLight).r;
        closestDepth *= u_PointFarPlane;
        float bias = 0.05;
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
		
	
		//float shadow = 0;
	
		Ambient  += ambient;
		Diffuse  += (1.0 - shadow) * diffuse;
		Specular += (1.0 - shadow) * specular;
	}

	// SpotLight
	if (u_SpotLight.Direction != vec3(0.0))
	{
		vec3 lightDirection = normalize(u_SpotLight.Position - fragPosition);
		float diff = max(dot(normal, lightDirection), 0.0);
		
		//vec3 reflectDirection = reflect(-lightDirection, normal);
		//float spec = pow(max(dot(normal, reflectDirection), 0.0), u_MaterialShininess);
		vec3 halfwayDirection = normalize(lightDirection + viewDirection);
		float spec = pow(max(dot(normal, halfwayDirection), 0.0), u_MaterialShininess);
		
		float distance    = length(u_SpotLight.Position - fragPosition);
		float attenuation = 1.0 / (u_SpotLight.Constant + u_SpotLight.Linear * distance + u_SpotLight.Quadratic * (distance * distance));    
		
		// spotlight (soft edges)
		float theta = dot(-lightDirection, normalize(u_SpotLight.Direction)); 
        float epsilon = (u_SpotLight.CutOff - u_SpotLight.OuterCutOff);
        float intensity = clamp((theta - u_SpotLight.OuterCutOff) / epsilon, 0.0, 1.0);
		
		vec3 ambient  = u_SpotLight.Ambient;
		vec3 diffuse  = u_SpotLight.Diffuse  * diff;  
		vec3 specular = u_SpotLight.Specular * spec;
	
		ambient  *= intensity * attenuation;
        diffuse  *= intensity * attenuation;
        specular *= intensity * attenuation;
		
	    vec4 spotLightSpacePosition = u_SpotLightSpace * vec4(fragPosition, 1.0);
        vec3 projCoords = spotLightSpacePosition.xyz / spotLightSpacePosition.w;
        projCoords = projCoords * 0.5 + 0.5;
        float closestDepth = texture(u_SpotDepthMap, projCoords.xy).r; 
        float currentDepth = projCoords.z;
	    float bias = max(0.005 * (1.0 - dot(normal, -u_SpotLight.Direction)), 0.0005);
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	
		Ambient  += ambient;
		Diffuse  += (1.0 - shadow) * diffuse;
		Specular += (1.0 - shadow) * specular;
	}
	
	o_Ambient  = vec4(Ambient, 1.0);
	o_Diffuse  = vec4(Diffuse, 1.0);
	o_Specular = vec4(Specular, 1.0);
}  