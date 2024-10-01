#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

layout (location = 0) out vec3 v_FragPosition;

layout (std140, binding = 16) uniform ViewProjection
{
    mat4 u_ViewProjection;
};

void main()
{
    v_FragPosition = a_Position;
    gl_Position =  u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core
layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec3 v_FragPosition;

layout (binding = 0) uniform sampler2D u_EquirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
// ²ÉÑù
vec2 SampleSphericalMap(vec3 fragPosition)
{
    vec2 uv = vec2(atan(fragPosition.z, fragPosition.x), asin(fragPosition.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(v_FragPosition));
    vec3 color = texture(u_EquirectangularMap, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}