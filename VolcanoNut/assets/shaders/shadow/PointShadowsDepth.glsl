#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;

void main()
{
    gl_Position = vec4(a_Position, 1.0);
}

#type geometry
#version 450 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout(std140, binding = 9) uniform shadowMatrices
{
    mat4 u_ShadowMatrices[6];
    float u_FarPlane;
};

layout (location = 0) out vec4 FragPosition; // FragPos from GS (output per emitvertex)

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            FragPosition = gl_in[i].gl_Position;
            gl_Position = u_ShadowMatrices[face] * FragPosition;
            EmitVertex();
        }    
        EndPrimitive();
    }
} 


#type fragment
#version 450 core

layout (location = 0) in vec4 FragPosition;

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

layout(std140, binding = 9) uniform shadowMatrices
{
    mat4 u_ShadowMatrices[6];
    float u_FarPlane;
};

void main()
{
    float lightDistance = length(FragPosition.xyz - u_PointLightPosition);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / u_FarPlane;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
}