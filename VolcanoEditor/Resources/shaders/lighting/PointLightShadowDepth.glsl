#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

struct InstanceData
{            
	mat4 transform;
};

layout (std430, binding = 21) readonly buffer InstanceDataBuffer
{
    InstanceData u_InstanceData[];
};

layout (location = 0) out vec4 v_FragPosition;

void main()
{
    InstanceData instanceData = u_InstanceData[gl_InstanceIndex];

    v_FragPosition = instanceData.transform * vec4(a_Position, 1.0);
}

#type geometry
#version 450 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout(std140, binding = 9) uniform PointLightShadowData
{
    mat4 u_LightSpaceMatrices[6];
    vec3 u_Position;
    float u_Radius;
};

layout (location = 0) in vec4 v_FragPosition_in[];

layout (location = 0) out vec4 v_FragPosition;

void main()
{
    for(int face = 0; face != 6; face++)
    {
        // 几何着色器内建变量 gl_Layer，指定发射出的基本图形送到立方体贴图的哪个面。
        gl_Layer = face;
        for(int i = 0; i != 3; i++) // 对于每个三角形的顶点
        {
            v_FragPosition = v_FragPosition_in[i];
            gl_Position = u_LightSpaceMatrices[face] * v_FragPosition;
            EmitVertex();
        }    
        EndPrimitive();
    }
} 


#type fragment
#version 450 core

layout (location = 0) in vec4 v_FragPosition;

layout(std140, binding = 9) uniform PointLightShadowData
{
    mat4 u_LightSpaceMatrices[6];
    vec3 u_Position;
    float u_Radius;
};

void main()
{
    float lightDistance = length(v_FragPosition.xyz - u_Position);
    
    // 通过除以u_Radius将lightDistance映射到[0, 1]
    gl_FragDepth = lightDistance / u_Radius;
}