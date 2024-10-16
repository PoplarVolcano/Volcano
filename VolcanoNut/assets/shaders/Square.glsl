#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main(){
	v_TexCoord=a_TexCoord;
	gl_Position = vec4(a_Position, 1.0);
}


//绘制颜色  sampler2D：对2D纹理的采样
#type fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
uniform sampler2D u_Texture;

void main(){
	color = texture(u_Texture, v_TexCoord);	
}