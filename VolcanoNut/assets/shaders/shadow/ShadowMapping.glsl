#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoords;

layout(std140, binding = 0) uniform Camera
{
	// projection * view
	mat4 u_ViewProjection;
};

layout(std140, binding = 8) uniform LightSpaceMatrix
{
	mat4 u_LightSpaceMatrix;
};

struct VertexOutput {
    vec3 FragPosition;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
};

layout (location = 0) out VertexOutput Output;


void main()
{
    Output.FragPosition = a_Position;
    Output.Normal = a_Normal;
    Output.TexCoords = a_TexCoords;
    Output.FragPosLightSpace = u_LightSpaceMatrix * vec4(Output.FragPosition, 1.0);
    gl_Position = u_ViewProjection *  vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout (location = 0) out vec4 FragColor;

struct VertexOutput {
    vec3 FragPosition;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
};

layout (location = 0) in VertexOutput Input;

layout (binding = 0) uniform sampler2D diffuseTexture;
layout (binding = 1) uniform sampler2D shadowMap;

layout(std140, binding = 1) uniform CameraPosition
{
	vec3 u_CameraPosition;
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

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projectionCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projectionCoords = projectionCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projectionCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projectionCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

void main()
{           
    vec3 color = texture(diffuseTexture, Input.TexCoords).rgb;
    vec3 normal = normalize(Input.Normal);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = u_PointLightAmbient * lightColor;

    // diffuse
    vec3 lightDirection = normalize(u_PointLightPosition - Input.FragPosition);
    float diff = max(dot(lightDirection, normal), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular
    vec3 viewDirection = normalize(u_CameraPosition - Input.FragPosition);
    vec3 reflectDir = reflect(-lightDirection, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDirection + viewDirection);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    

    // calculate shadow
    float shadow = ShadowCalculation(Input.FragPosLightSpace);                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);
}