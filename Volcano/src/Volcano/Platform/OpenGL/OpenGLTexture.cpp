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
		///����OpenGLm_RendererID������洢����rbg8λ����ߵĻ�����
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);
		//���ò���:����Ŵ�ʱ����Χ��ɫ��ƽ��ֵ����
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		

	}

	// ͼ���ļ����·��
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool filp)
		: m_FilePath(path)
	{
		int width, height, channels;
		if(filp)
			stbi_set_flip_vertically_on_load(1);//��ת
		// �������:x,yͼ��ĳߴ�, channels_in_file�ļ��е�ͨ����, desired_channels��ͼ��ת��Ϊ�����ʽ
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		VOL_CORE_ASSERT(data, "ͼƬ���ش���");
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

		// ��ȡ���ݻ��������ϴ���OpenGL��Ȼ���ϴ���GPU
		// ����2D����
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		// ��GPU�Ϸ����ڴ棬���ڴ洢ͼ������
		// internalformat OpenGL��δ洢��GL_RGB8����һ������ÿ��ͨ���а�λ��
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);
		// ������������
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
		// �����������, GL_TEXTURE_MIN_FILTER��ʲô���Ĺ�������С, 
		// GL_LINEAR linear filtering �����˲��������Բ�ֵ������Ҫʲô��ɫ��
		glGenerateMipmap(GL_TEXTURE_2D);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// �ͷ�data�洢��CPU�ϵ��ڴ�
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
		VOL_CORE_ASSERT(size == m_Width * m_Height * bpc, "OpenGLTexture2D�����ݱ����������ģ�");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}
}