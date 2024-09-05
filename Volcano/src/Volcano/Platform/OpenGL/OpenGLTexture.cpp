#include "volpch.h"
#include "OpenGLTexture.h"

#include "Volcano/Renderer/Renderer.h"

#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Volcano {

	//////////////////////////////////////////////////////////////////////////////////
	// Texture2D
	//////////////////////////////////////////////////////////////////////////////////

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		:m_Width(width), m_Height(height)
	{
		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		///告诉OpenGLm_RendererID的纹理存储的是rbg8位，宽高的缓冲区
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);
		//配置参数:纹理放大时用周围颜色的平均值过滤
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		

	}

	// 图像文件相对路径
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool filp)
		: m_FilePath(path)
	{
		int width, height, channels;
		if(filp)
			stbi_set_flip_vertically_on_load(1);//翻转
		// 输出参数:x,y图像的尺寸, channels_in_file文件中的通道数, desired_channels把图像转换为所需格式
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		VOL_CORE_ASSERT(data, "图片加载错误");
		m_Width = width;
		m_Height = height;

		if (channels == 4)
		{
			m_InternalFormat = GL_RGBA8;
			m_DataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			m_InternalFormat = GL_RGB8;
			m_DataFormat = GL_RGB;
		}

		VOL_CORE_ASSERT(m_InternalFormat && m_DataFormat, "Format not supported!");

		// 获取数据缓冲区并上传到OpenGL，然后上传到GPU
		// 创建2D纹理
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		// 在GPU上分配内存，便于存储图像数据
		// internalformat OpenGL如何存储（GL_RGB8处理一个纹理，每个通道有八位）
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);
		// 更新纹理数据
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
		// 设置纹理参数, GL_TEXTURE_MIN_FILTER用什么样的过滤来缩小, 
		// GL_LINEAR linear filtering 线性滤波（用线性插值计算想要什么颜色）
		glGenerateMipmap(GL_TEXTURE_2D);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// 释放data存储在CPU上的内存
		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}
	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		uint32_t bpc = m_DataFormat == GL_RGBA ? 4 : 3;
		VOL_CORE_ASSERT(size == m_Width * m_Height * bpc, "OpenGLTexture2D：数据必须是完整的！");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}
}