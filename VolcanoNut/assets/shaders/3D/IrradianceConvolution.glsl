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

layout (binding = 0) uniform samplerCube u_EnvironmentMap;

const float PI = 3.14159265359;

void main()
{		
    //世界矢量充当从原点开始的切线曲面的法线，与v_FragPosition对齐。
    //给定此法向量，计算环境的所有入射辐射。
    // 此辐射的结果是来自法线方向的光的辐射，这是我们在PBR着色器中用于采样辐照度的方法。
	// The world vector acts as the normal of a tangent surface from the origin, aligned to v_FragPosition. 
    // The result of this radiance is the radiance of light coming from -Normal direction, which is what we use in the PBR shader to sample irradiance.
    // 采样方向等于半球方向(法向量)
    vec3 N = normalize(v_FragPosition);

    vec3 irradiance = vec3(0.0);   
    
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));
       
    float sampleDelta = 0.025; // 减小（或增加）这个增量将会增加（或减少）精确度
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // 球面到笛卡尔（切线空间）spherical to cartesian (tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world space
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(u_EnvironmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    FragColor = vec4(irradiance, 1.0);
}