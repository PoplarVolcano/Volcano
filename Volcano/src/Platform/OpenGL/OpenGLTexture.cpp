#include "volpch.h"
#include "OpenGLTexture.h"

#include <glad/glad.h>

#include "stb_image.h"

namespace Volcano {

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		: m_Path(path)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		// 输出参数:x,y图像的尺寸, channels_in_file文件中的通道数, desired_channels把图像转换为所需格式
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		VOL_CORE_ASSERT(data, "Failed to load image!");
		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		VOL_CORE_ASSERT(internalFormat && dataFormat, "Format not supported!");

		//获取数据缓冲区并上传到OpenGL，然后上传到GPU
		//创建2D纹理
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		//在GPU上分配内存，便于存储图像数据
		//internalformat OpenGL如何存储（GL_RGB8处理一个纹理，每个通道有八位）
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

		//设置纹理参数, GL_TEXTURE_MIN_FILTER用什么样的过滤来缩小, 
		// GL_LINEAR linear filtering 线性滤波（用线性插值计算想要什么颜色）
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		//上传纹理
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

		//释放data存储在CPU上的内存
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

}