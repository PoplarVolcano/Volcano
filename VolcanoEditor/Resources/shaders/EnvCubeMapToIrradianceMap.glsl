#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

layout (location = 0) out vec3 v_FragPosition;

layout (std140, binding = 40) uniform TemporaryMat4
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
layout (location = 0) out vec4 o_FragColor;

layout (location = 0) in vec3 v_FragPosition;

layout (binding = 0) uniform samplerCube u_EnvCubeMap;

const float PI = 3.14159265359;

void main()
{		
    // 从原点到像素的世界坐标作为法向量的切线空间，计算环境的所有入射辐射。
    // 此辐射的结果是来自法线方向的光的辐射，这是我们在PBR着色器中用于采样辐照度的方法。
    vec3 N = normalize(v_FragPosition);

    vec3 irradiance = vec3(0.0);   
    
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));
       
    float sampleDelta = 0.025; // 减小（或增加）这个增量将会增加（或减少）精确度
    float sampleCount = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // 球面到笛卡尔（切线空间）
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // 切线空间到世界空间
            vec3 worldSample = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            // 将采样的颜色值乘以系数 cos(θ) ，因为较大角度的光较弱
            // 系数 sin(θ) 用于权衡较高半球区域的较小采样区域的贡献度
            irradiance += texture(u_EnvCubeMap, worldSample).rgb * cos(theta) * sin(theta);
            sampleCount += 1.0;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(sampleCount));
    
    o_FragColor = vec4(irradiance, 1.0);
}