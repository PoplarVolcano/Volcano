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
    //����ʸ���䵱��ԭ�㿪ʼ����������ķ��ߣ���v_FragPosition���롣
    //�����˷����������㻷��������������䡣
    // �˷���Ľ�������Է��߷���Ĺ�ķ��䣬����������PBR��ɫ�������ڲ������նȵķ�����
	// The world vector acts as the normal of a tangent surface from the origin, aligned to v_FragPosition. 
    // The result of this radiance is the radiance of light coming from -Normal direction, which is what we use in the PBR shader to sample irradiance.
    // ����������ڰ�����(������)
    vec3 N = normalize(v_FragPosition);

    vec3 irradiance = vec3(0.0);   
    
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));
       
    float sampleDelta = 0.025; // ��С�������ӣ���������������ӣ�����٣���ȷ��
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // ���浽�ѿ��������߿ռ䣩spherical to cartesian (tangent space)
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