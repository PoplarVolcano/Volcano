#include "volpch.h"
#include "OpenGLTexture.h"

#include "Volcano/Renderer/Renderer.h"

#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Volcano {

	static GLenum VolcanoToOpenGLTextureFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::RG:      return GL_RG;
		case TextureFormat::RGB:     return GL_RGB;
		case TextureFormat::RGBA:    return GL_RGBA;
		case TextureFormat::RGBA8:   return GL_RGBA8;
		case TextureFormat::RG16F:   return GL_RG16F;
		case TextureFormat::RGB16F:  return GL_RGB16F;
		case TextureFormat::RGBA16F: return GL_RGBA16F;
		}
		VOL_CORE_ASSERT(false, "Unknown texture format!");
		return 0;
	}

	static GLenum VolcanoToOpenGLTextureWrap(TextureWrap wrap)
	{
		switch (wrap)
		{
		case TextureWrap::REPEAT:        return GL_REPEAT;
		case TextureWrap::CLAMP_TO_EDGE: return GL_CLAMP_TO_EDGE;
		}
		VOL_CORE_ASSERT(false, "Unknown texture wrap!");
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Texture2D
	//////////////////////////////////////////////////////////////////////////////////

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, TextureFormat internalFormat, TextureFormat dataFormat, TextureWrap wrap)
		:m_Width(width), m_Height(height)
	{
		m_InternalFormat = VolcanoToOpenGLTextureFormat(internalFormat);
		m_DataFormat = VolcanoToOpenGLTextureFormat(dataFormat);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);
		//���ò���:����Ŵ�ʱ����Χ��ɫ��ƽ��ֵ����
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, VolcanoToOpenGLTextureWrap(wrap));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, VolcanoToOpenGLTextureWrap(wrap));
		

	}

	// ͼ���ļ����·��
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool filp, TextureFormat internalFormat)
		: m_FilePath(path)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(filp);//��ת
		// �������:x,yͼ��ĳߴ�, channels_in_file�ļ��е�ͨ����, desired_channels��ͼ��ת��Ϊ�����ʽ
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		VOL_CORE_ASSERT(data, "ͼƬ���ش���");
		m_Width = width;
		m_Height = height;

		switch (channels)
		{
		case 1:
			m_InternalFormat = GL_RED;
			m_DataFormat = GL_RED;
			break;
		case 2:
			if (internalFormat == TextureFormat::RG16F)
				m_InternalFormat = GL_RG16F;
			else
				m_InternalFormat = GL_RG8;
			m_DataFormat = GL_RG;
			break;
		case 3:
			if (internalFormat == TextureFormat::RGB16F)
				m_InternalFormat = GL_RGB16F;
			else
				m_InternalFormat = GL_RGB8;
			m_DataFormat = GL_RGB;
			break;
		case 4:
			if (internalFormat == TextureFormat::RGBA16F)
				m_InternalFormat = GL_RGBA16F;
			else
				m_InternalFormat = GL_RGBA8;
			m_DataFormat = GL_RGBA;
			break;
		}

		VOL_CORE_ASSERT(m_InternalFormat && m_DataFormat, "Format not supported!");

		// ��ȡ���ݻ��������ϴ���OpenGL��Ȼ���ϴ���GPU
		// ����2D����
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		//glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		// ��GPU�Ϸ����ڴ棬���ڴ洢ͼ������
		// internalformat OpenGL��δ洢��GL_RGB8����һ������ÿ��ͨ���а�λ��
		//glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);
		// ������������
		//glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
		glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, width, height, 0, m_DataFormat, GL_UNSIGNED_BYTE, data);
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
	void Texture::Bind(uint32_t m_RendererID, uint32_t slot)
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	void Texture::ClearTextureSlot(uint32_t i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		uint32_t bpc = m_DataFormat == GL_RGBA ? 4 : 3;
		VOL_CORE_ASSERT(size == m_Width * m_Height * bpc, "OpenGLTexture2D�����ݱ����������ģ�");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::SetDataFormat(TextureFormat format)
	{
		m_DataFormat = VolcanoToOpenGLTextureFormat(format);
	}

	void OpenGLTexture2D::SetInternalFormat(TextureFormat format)
	{
		m_InternalFormat = VolcanoToOpenGLTextureFormat(format);
	}

	uint32_t OpenGLTexture2D::GetMipLevelCount() const
	{
		return Texture::CalculateMipMapCount(m_Width, m_Height);
	}

	//===============================TextureCube=========================================
	
	OpenGLTextureCube::OpenGLTextureCube(TextureFormat format, uint32_t width, uint32_t height)
	{
		m_Width = width;
		m_Height = height;
		m_Format = format;

		uint32_t levels = Texture::CalculateMipMapCount(width, height);

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		//glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);

		for (uint32_t i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, VolcanoToOpenGLTextureFormat(m_Format), m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

		//glTextureStorage2D(m_RendererID, levels, VolcanoToOpenGLTextureFormat(m_Format), width, height);
		//glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		//glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// glTextureParameterf(m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, 16);
	}

	OpenGLTextureCube::OpenGLTextureCube(const std::string& path)
		: m_FilePath(path)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(false);
		m_ImageData = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb);

		m_Width = width;
		m_Height = height;
		m_Format = TextureFormat::RGB;

		// �ָ���������ͼ
		uint32_t faceWidth = m_Width / 4;
		uint32_t faceHeight = m_Height / 3;
		VOL_CORE_ASSERT(faceWidth == faceHeight, "Non-square faces!");

		std::array<unsigned char*, 6> faces;
		for (size_t i = 0; i < faces.size(); i++)
			faces[i] = new unsigned char[faceWidth * faceHeight * 3]; // BPP��RGB = 3 

		int faceIndex = 0;

		// ��ȡ��ǰ�Һ���
		for (size_t i = 0; i < 4; i++)
		{
			for (size_t y = 0; y < faceHeight; y++)
			{
				size_t yOffset = y + faceHeight;
				for (size_t x = 0; x < faceWidth; x++)
				{
					size_t xOffset = x + i * faceWidth;
					faces[faceIndex][(x + y * faceWidth) * 3 + 0] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 0];
					faces[faceIndex][(x + y * faceWidth) * 3 + 1] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 1];
					faces[faceIndex][(x + y * faceWidth) * 3 + 2] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 2];
				}
			}
			faceIndex++;
		}

		// ��ȡ�����棬����ǰ��
		for (size_t i = 0; i < 3; i++)
		{
			// Skip the middle one
			if (i == 1)
				continue;

			for (size_t y = 0; y < faceHeight; y++)
			{
				size_t yOffset = y + i * faceHeight;
				for (size_t x = 0; x < faceWidth; x++)
				{
					size_t xOffset = x + faceWidth;
					faces[faceIndex][(x + y * faceWidth) * 3 + 0] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 0];
					faces[faceIndex][(x + y * faceWidth) * 3 + 1] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 1];
					faces[faceIndex][(x + y * faceWidth) * 3 + 2] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 2];
				}
			}
			faceIndex++;
		}
		
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameterf(m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, RendererAPI::GetCapabilities().MaxAnisotropy);

		auto format = VolcanoToOpenGLTextureFormat(m_Format);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[2]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[0]);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[4]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[5]);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[1]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[3]);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glBindTexture(GL_TEXTURE_2D, 0);

		for (size_t i = 0; i < faces.size(); i++)
			delete[] faces[i];

		stbi_image_free(m_ImageData);
	}

	OpenGLTextureCube::OpenGLTextureCube(std::vector<std::string> faces)
		: m_FilePath(faces[0])
	{
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

		int width, height, channels;
		stbi_set_flip_vertically_on_load(false);
		m_Format = TextureFormat::RGB;
		auto format = VolcanoToOpenGLTextureFormat(m_Format);
		for (unsigned int i = 0; i < faces.size(); i++)
		{
			m_ImageData = stbi_load(faces[i].c_str(), &width, &height, &channels, STBI_rgb);
			if (m_ImageData)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, m_ImageData);
				stbi_image_free(m_ImageData);
			}
			else
			{
				std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
				stbi_image_free(m_ImageData);
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameterf(m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, RendererAPI::GetCapabilities().MaxAnisotropy);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	OpenGLTextureCube::~OpenGLTextureCube()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTextureCube::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	uint32_t OpenGLTextureCube::GetMipLevelCount() const
	{
		return Texture::CalculateMipMapCount(m_Width, m_Height);
	}
}