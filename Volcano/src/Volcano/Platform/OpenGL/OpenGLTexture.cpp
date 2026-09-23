#include "volpch.h"
#include "OpenGLTexture.h"

#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Project/Project.h"

#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Volcano {

	namespace Utils
	{
		GLenum VolcanoToOpenGLTextureInternalFormat(TextureInternalFormat format)
		{
			switch (format)
			{
			case TextureInternalFormat::RED:      return GL_RED;
			case TextureInternalFormat::GREEN:	  return GL_GREEN;
			case TextureInternalFormat::BLUE:	  return GL_BLUE;
			case TextureInternalFormat::RG:		  return GL_RG;
			case TextureInternalFormat::RGB:	  return GL_RGB;
			case TextureInternalFormat::RGBA:	  return GL_RGBA;
			case TextureInternalFormat::R8:		  return GL_R8;
			case TextureInternalFormat::R16:	  return GL_R16;
			case TextureInternalFormat::R16F:	  return GL_R16F;
			case TextureInternalFormat::R32F:	  return GL_R32F;
			case TextureInternalFormat::R8I:	  return GL_R8I;
			case TextureInternalFormat::RG8:	  return GL_RG8;
			case TextureInternalFormat::RG16F:	  return GL_RG16F;
			case TextureInternalFormat::RG32F:	  return GL_RG32F;
			case TextureInternalFormat::RGB4:	  return GL_RGB4;
			case TextureInternalFormat::RGB8:	  return GL_RGB8;
			case TextureInternalFormat::RGB16F:	  return GL_RGB16F;
			case TextureInternalFormat::RGB32F:	  return GL_RGB32F;
			case TextureInternalFormat::RGBA4:	  return GL_RGBA4;
			case TextureInternalFormat::RGBA8:	  return GL_RGBA8;
			case TextureInternalFormat::RGBA16F:  return GL_RGBA16F;
			case TextureInternalFormat::RGBA32F:  return GL_RGBA32F;
			case TextureInternalFormat::RGB10_A2: return GL_RGB10_A2;
			default:
				return 0;
			}
			//VOL_CORE_ASSERT(false, "Unknown texture format!");
		}

		TextureInternalFormat OpenGLTextureInternalFormatToVolcano(GLenum format)
		{
			switch (format)
			{
			case GL_RED:       return TextureInternalFormat::RED;
			case GL_GREEN:	   return TextureInternalFormat::GREEN;
			case GL_BLUE:	   return TextureInternalFormat::BLUE;
			case GL_RG:		   return TextureInternalFormat::RG;
			case GL_RGB:	   return TextureInternalFormat::RGB;
			case GL_RGBA:	   return TextureInternalFormat::RGBA;
			case GL_R8:		   return TextureInternalFormat::R8;
			case GL_R16:	   return TextureInternalFormat::R16;
			case GL_R16F:	   return TextureInternalFormat::R16F;
			case GL_R32F:	   return TextureInternalFormat::R32F;
			case GL_R8I:	   return TextureInternalFormat::R8I;
			case GL_RG8:	   return TextureInternalFormat::RG8;
			case GL_RG16F:	   return TextureInternalFormat::RG16F;
			case GL_RG32F:	   return TextureInternalFormat::RG32F;
			case GL_RGB4:	   return TextureInternalFormat::RGB4;
			case GL_RGB8:	   return TextureInternalFormat::RGB8;
			case GL_RGB16F:	   return TextureInternalFormat::RGB16F;
			case GL_RGB32F:	   return TextureInternalFormat::RGB32F;
			case GL_RGBA4:	   return TextureInternalFormat::RGBA4;
			case GL_RGBA8:	   return TextureInternalFormat::RGBA8;
			case GL_RGBA16F:   return TextureInternalFormat::RGBA16F;
			case GL_RGBA32F:   return TextureInternalFormat::RGBA32F;
			case GL_RGB10_A2:  return TextureInternalFormat::RGB10_A2;
			default:
				return TextureInternalFormat::NONE;
			}
			//VOL_CORE_ASSERT(false, "Unknown texture format!");
		}
		

		GLenum VolcanoToOpenGLTextureDataFormat(TextureDataFormat format)
		{
			switch (format)
			{
			case TextureDataFormat::NONE:            return GL_NONE;
			case TextureDataFormat::RED:			 return GL_RED;
			case TextureDataFormat::GREEN:			 return GL_GREEN;
			case TextureDataFormat::BLUE:	         return GL_BLUE;
			case TextureDataFormat::RG:	             return GL_RG;
			case TextureDataFormat::RGB:	         return GL_RGB;
			case TextureDataFormat::BGR:	         return GL_BGR;
			case TextureDataFormat::RGBA:	         return GL_RGBA;
			case TextureDataFormat::BGRA:	         return GL_BGRA;
			case TextureDataFormat::RED_INTEGER:	 return GL_RED_INTEGER;
			case TextureDataFormat::RG_INTEGER:	     return GL_RG_INTEGER;
			case TextureDataFormat::RGB_INTEGER:	 return GL_RGB_INTEGER;
			case TextureDataFormat::RGBA_INTEGER:	 return GL_RGBA_INTEGER;
			case TextureDataFormat::DEPTH_COMPONENT: return GL_DEPTH_COMPONENT;
			case TextureDataFormat::STENCIL_INDEX:	 return GL_STENCIL_INDEX;
			case TextureDataFormat::DEPTH_STENCIL:	 return GL_DEPTH_STENCIL;
			default:
				return GL_RED;
			}
			//VOL_CORE_ASSERT(false, "Unknown texture format!");
		}

		TextureDataFormat OpenGLTextureDataFormatToVolcano(GLenum format)
		{
			switch (format)
			{
			case GL_NONE:            return TextureDataFormat::NONE;
			case GL_RED:             return TextureDataFormat::RED;
			case GL_GREEN:           return TextureDataFormat::GREEN;
			case GL_BLUE:            return TextureDataFormat::BLUE;
			case GL_RG:              return TextureDataFormat::RG;
			case GL_RGB:             return TextureDataFormat::RGB;
			case GL_BGR:             return TextureDataFormat::BGR;
			case GL_RGBA:            return TextureDataFormat::RGBA;
			case GL_BGRA:            return TextureDataFormat::BGRA;
			case GL_RED_INTEGER:     return TextureDataFormat::RED_INTEGER;
			case GL_RG_INTEGER:      return TextureDataFormat::RG_INTEGER;
			case GL_RGB_INTEGER:     return TextureDataFormat::RGB_INTEGER;
			case GL_RGBA_INTEGER:    return TextureDataFormat::RGBA_INTEGER;
			case GL_DEPTH_COMPONENT: return TextureDataFormat::DEPTH_COMPONENT;
			case GL_STENCIL_INDEX:   return TextureDataFormat::STENCIL_INDEX;
			case GL_DEPTH_STENCIL:   return TextureDataFormat::DEPTH_STENCIL;
			default:
				return TextureDataFormat::NONE;
			}
			//VOL_CORE_ASSERT(false, "Unknown texture format!");
		}

		GLenum VolcanoToOpenGLTextureWrap(TextureWrap wrap)
		{
			switch (wrap)
			{
			case TextureWrap::REPEAT:        return GL_REPEAT;
			case TextureWrap::CLAMP_TO_EDGE: return GL_CLAMP_TO_EDGE;
			}
			VOL_CORE_ASSERT(false, "Unknown texture wrap!");
			return 0;
		}

		GLenum VolcanoToOpenGLTextureFilter(TextureFilter filter)
		{
			switch (filter)
			{
			case TextureFilter::LINEAR:               return GL_LINEAR;
			case TextureFilter::LINEAR_MIPMAP_LINEAR: return GL_LINEAR_MIPMAP_LINEAR;
			}
			VOL_CORE_ASSERT(false, "Unknown texture filter!");
			return 0;
		}

		uint8_t OpenGLTextureDataFormatToChannels(GLenum dataFormat)
		{
			switch (dataFormat)
			{
			case GL_RED:
			case GL_GREEN:
			case GL_BLUE:
			case GL_RED_INTEGER:
				return 1;
			case GL_RG:
			case GL_RG_INTEGER:
				return 2;
			case GL_RGB:
			case GL_BGR:
			case GL_RGB_INTEGER:
				return 3;
			case GL_RGBA:
			case GL_BGRA:
			case GL_RGBA_INTEGER:
				return 4;
			default:
				return 0;
			}
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, TextureInternalFormat internalFormat, TextureDataFormat dataFormat, TextureWrap wrap)
		:m_Width(width), m_Height(height)
	{
		m_InternalFormat = Utils::VolcanoToOpenGLTextureInternalFormat(internalFormat);
		m_DataFormat = Utils::VolcanoToOpenGLTextureDataFormat(dataFormat);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, Utils::VolcanoToOpenGLTextureWrap(wrap));
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, Utils::VolcanoToOpenGLTextureWrap(wrap));

		if (!m_Handle)
		{
			m_Handle = glGetTextureHandleARB(m_RendererID);
			glMakeTextureHandleResidentARB(m_Handle);
		}
	}

	// 图像文件相对路径
	OpenGLTexture2D::OpenGLTexture2D(const std::string& absolutePath, bool flip, TextureInternalFormat internalFormat)
	{
		m_FileRelativePath = std::filesystem::relative(std::filesystem::path(absolutePath), Project::GetAssetsRelativePath()).string();

		int width, height, channels;
		m_Flip = flip;
		stbi_set_flip_vertically_on_load(flip);
		stbi_uc* data = stbi_load(absolutePath.c_str(), &width, &height, &channels, 0);

		std::vector<stbi_uc> fallback;

		if (!data)
		{
			VOL_CORE_WARN("Failed to load image '{0}', using default black texture.", absolutePath);

			// 兜底：1x1 黑色，带 alpha = 255
			width = 1;
			height = 1;
			channels = 4;
			fallback = { 0, 0, 0, 255 };
			data = fallback.data();
		}

		m_Width = width;
		m_Height = height;

		switch (channels)
		{
		case 1:
		{
			switch (internalFormat)
			{
			case TextureInternalFormat::R8:
				m_InternalFormat = GL_R8;
				break;
			case TextureInternalFormat::R16:
				m_InternalFormat = GL_R16;
				break;
			case TextureInternalFormat::R16F:
				m_InternalFormat = GL_R16F;
				break;
			case TextureInternalFormat::R32F:
				m_InternalFormat = GL_R32F;
				break;
			case TextureInternalFormat::R8I:
				m_InternalFormat = GL_R8I;
				break;
			default:
				m_InternalFormat = GL_R8;
				break;
			}
			m_DataFormat = GL_RED;
			break;
		}
		case 2:
		{
			switch (internalFormat)
			{
			case TextureInternalFormat::RG8:
				m_InternalFormat = GL_RG8;
				break;
			case TextureInternalFormat::RG16F:
				m_InternalFormat = GL_RG16F;
				break;
			case TextureInternalFormat::RG32F:
				m_InternalFormat = GL_RG32F;
				break;
			default:
				m_InternalFormat = GL_RG8;
				break;
			}
			m_DataFormat = GL_RG;
			break;
		}
		case 3:
		{
			switch (internalFormat)
			{
			case TextureInternalFormat::RGB4:
				m_InternalFormat = GL_RGB4;
				break;
			case TextureInternalFormat::RGB8:
				m_InternalFormat = GL_RGB8;
				break;
			case TextureInternalFormat::RGB16F:
				m_InternalFormat = GL_RGB16F;
				break;
			case TextureInternalFormat::RGB32F:
				m_InternalFormat = GL_RGB32F;
				break;
			default:
				m_InternalFormat = GL_RGB8;
				break;
			}
			m_DataFormat = GL_RGB;
			break;
		}
		case 4:
		{
			switch (internalFormat)
			{
			case TextureInternalFormat::RGBA4:
				m_InternalFormat = GL_RGBA4;
				break;
			case TextureInternalFormat::RGBA8:
				m_InternalFormat = GL_RGBA8;
				break;
			case TextureInternalFormat::RGBA16F:
				m_InternalFormat = GL_RGBA16F;
				break;
			case TextureInternalFormat::RGBA32F:
				m_InternalFormat = GL_RGBA32F;
				break;
			case TextureInternalFormat::RGB10_A2:
				m_InternalFormat = GL_RGB10_A2;
				break;
			default:
				m_InternalFormat = GL_RGBA8;
				break;
			}
			m_DataFormat = GL_RGBA;
			break;
		}
		}

		VOL_CORE_ASSERT(m_InternalFormat && m_DataFormat, "Format not supported!");


		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		int mipLevels = 1 + (int)std::floor(std::log2(std::max(m_Width, m_Height)));
		glTextureStorage2D(m_RendererID, mipLevels, m_InternalFormat, m_Width, m_Height);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateTextureMipmap(m_RendererID);

		if (!m_Handle)
		{
			m_Handle = glGetTextureHandleARB(m_RendererID);
			glMakeTextureHandleResidentARB(m_Handle);
		}

		// 释放data存储在CPU上的内存
		// data 指向 fallback 时不需要 stbi_image_free
		if (fallback.empty())
		{
			stbi_image_free(data);
		}
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
		VOL_CORE_ASSERT(size == m_Width * m_Height * bpc, "OpenGLTexture2D：数据必须是完整的！");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::SetDataFormat(TextureDataFormat format)
	{
		m_DataFormat = Utils::VolcanoToOpenGLTextureDataFormat(format);
	}

	void OpenGLTexture2D::SetInternalFormat(TextureInternalFormat format)
	{
		m_InternalFormat = Utils::VolcanoToOpenGLTextureInternalFormat(format);
	}

	uint32_t OpenGLTexture2D::GetDataFormat()
	{
		return m_DataFormat;
	}

	uint32_t OpenGLTexture2D::GetInternalFormat()
	{
		return m_InternalFormat;
	}

	uint32_t OpenGLTexture2D::GetMipLevelCount() const
	{
		return Texture::CalculateMipMapCount(m_Width, m_Height);
	}


	// ==================================TextureCube=================================================

	OpenGLTextureCube::OpenGLTextureCube(
		uint32_t width,
		uint32_t height,
		TextureInternalFormat internalFormat,
		TextureDataFormat dataFormat,
		TextureWrap wrap,
		TextureFilter minFilter,
		TextureFilter maxFilter
	)
	{
		m_Width = width;
		m_Height = height;
		m_InternalFormat = Utils::VolcanoToOpenGLTextureInternalFormat(internalFormat);
		m_DataFormat = Utils::VolcanoToOpenGLTextureDataFormat(dataFormat);

		auto wrapTemp = Utils::VolcanoToOpenGLTextureWrap(wrap);
		auto minFilterTemp = Utils::VolcanoToOpenGLTextureFilter(minFilter);
		auto maxFilterTemp = Utils::VolcanoToOpenGLTextureFilter(maxFilter);


		uint32_t levels = Texture::CalculateMipMapCount(width, height);

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

		for (uint32_t i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, 0);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilterTemp);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, maxFilterTemp);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapTemp);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapTemp);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapTemp);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	OpenGLTextureCube::OpenGLTextureCube(const std::string& absolutePath)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(false);
		m_ImageData = stbi_load(absolutePath.c_str(), &width, &height, &channels, STBI_rgb);

		m_FileRelativePath = std::filesystem::relative(std::filesystem::path(absolutePath), Project::GetAssetsRelativePath()).string();;


		m_Width = width;
		m_Height = height;
		m_InternalFormat = Utils::VolcanoToOpenGLTextureInternalFormat(TextureInternalFormat::RGB);

		switch (channels)
		{
		case 1:
			m_DataFormat = GL_RED;
			break;
		case 2:
			m_DataFormat = GL_RG;
			break;
		case 3:
			m_DataFormat = GL_RGB;
			break;
		case 4:
			m_DataFormat = GL_RGBA;
			break;
		default:
			VOL_CORE_ASSERT("OpenGLTextureCube(const std::string& path)");
			break;
		}

		// 分割立方体贴图
		uint32_t faceWidth = m_Width / 4;
		uint32_t faceHeight = m_Height / 3;
		VOL_CORE_ASSERT(faceWidth == faceHeight, "Non-square faces!");

		std::array<unsigned char*, 6> faces;
		for (size_t i = 0; i < faces.size(); i++)
			faces[i] = new unsigned char[faceWidth * faceHeight * 3]; // BPP：RGB = 3 

		int faceIndex = 0;

		// 读取左前右后面
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

		// 读取上下面，跳过前面
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

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, m_InternalFormat, faceWidth, faceHeight, 0, m_DataFormat, GL_UNSIGNED_BYTE, faces[2]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, m_InternalFormat, faceWidth, faceHeight, 0, m_DataFormat, GL_UNSIGNED_BYTE, faces[0]);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, m_InternalFormat, faceWidth, faceHeight, 0, m_DataFormat, GL_UNSIGNED_BYTE, faces[4]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, m_InternalFormat, faceWidth, faceHeight, 0, m_DataFormat, GL_UNSIGNED_BYTE, faces[5]);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, m_InternalFormat, faceWidth, faceHeight, 0, m_DataFormat, GL_UNSIGNED_BYTE, faces[1]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, m_InternalFormat, faceWidth, faceHeight, 0, m_DataFormat, GL_UNSIGNED_BYTE, faces[3]);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glBindTexture(GL_TEXTURE_2D, 0);

		for (size_t i = 0; i < faces.size(); i++)
			delete[] faces[i];

		stbi_image_free(m_ImageData);
	}

	static int GetChannelCount(GLenum internalFormat)
	{
		switch (internalFormat)
		{
			// 1 通道
		case GL_RED:
		case GL_R8:
		case GL_R16F:
		case GL_R32F:
		case GL_DEPTH_COMPONENT:
		case GL_DEPTH_COMPONENT16:
		case GL_DEPTH_COMPONENT24:
		case GL_DEPTH_COMPONENT32F:
			return 1;

			// 2 通道
		case GL_RG:
		case GL_RG8:
		case GL_RG16F:
		case GL_RG32F:
			return 2;

			// 3 通道
		case GL_RGB:
		case GL_RGB8:
		case GL_RGB16F:
		case GL_RGB32F:
		case GL_SRGB:
		case GL_SRGB8:
			return 3;

			// 4 通道
		case GL_RGBA:
		case GL_RGBA8:
		case GL_RGBA16F:
		case GL_RGBA32F:
		case GL_SRGB_ALPHA:
		case GL_SRGB8_ALPHA8:
		case GL_RGBA16:
		case GL_RGBA32UI:
		case GL_RGBA32I:
		case GL_RGBA16UI:
		case GL_RGBA16I:
		case GL_RGBA8UI:
		case GL_RGBA8I:
			return 4;

			// 深度 + 模板（特殊，通道数按2计，但常见忽略）
		case GL_DEPTH_STENCIL:
		case GL_DEPTH24_STENCIL8:
		case GL_DEPTH32F_STENCIL8:
			return 2; // 深度 + 模板

		default:
			// 未知格式，记录警告并返回 0
			VOL_CORE_WARN("Unknown internal format in GetChannelCount: 0x{:x}", internalFormat);
			return 0;
		}
	}

	OpenGLTextureCube::OpenGLTextureCube(const std::array<Ref<Texture2D>, 6>& faces)
	{
		// 找到一个非空的面来确定尺寸和格式（假设所有有效面格式一致）
		Ref<Texture2D> refFace = nullptr;
		for (const auto& face : faces)
		{
			if (face != nullptr)
			{
				refFace = face;
				break;
			}
		}
		if (!refFace)
		{
			VOL_CORE_ASSERT("All faces are null, cannot create cubemap.");
			return;
		}

		m_Width = refFace->GetWidth();
		m_Height = refFace->GetHeight();
		m_InternalFormat = refFace->GetInternalFormat();
		m_DataFormat = refFace->GetDataFormat();

		// 创建立方体贴图并分配存储（传统方式，避免 DSA 与 Cube 的兼容性问题）
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, m_InternalFormat, m_Width, m_Height);

		// 循环 6 个面
		for (uint32_t i = 0; i != 6; i++)
		{
			uint32_t srcID = 0;

			if (faces[i])
			{
				// 检查尺寸是否匹配
				if (faces[i]->GetWidth() == m_Width && faces[i]->GetHeight() == m_Height)
				{
					srcID = faces[i]->GetRendererID();
				}
				else
				{
					// 尺寸不匹配，需要创建临时纹理（实际上你应该保证传入纹理尺寸一致）
					VOL_CORE_WARN("Face {} size mismatch, creating temporary black texture.", i);
				}
			}

			// 源纹理无效（空指针或尺寸不匹配），使用 glTexSubImage2D 填充黑色
			if (srcID == 0)
			{
				glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

				GLenum cubeTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
				std::vector<uint8_t> zeroData(m_Width * m_Height * GetChannelCount(m_InternalFormat), 0);
				glTexSubImage2D(cubeTarget, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, zeroData.data());

				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			}
			else
			{
				// 读取源纹理像素到 CPU
				size_t pixelSize = m_Width * m_Height * GetChannelCount(m_InternalFormat);
				std::vector<uint8_t> pixels(pixelSize);

				glBindTexture(GL_TEXTURE_2D, srcID);
				glGetTexImage(GL_TEXTURE_2D, 0, m_DataFormat, GL_UNSIGNED_BYTE, pixels.data());
				glBindTexture(GL_TEXTURE_2D, 0);

				// 上传到立方体面
				glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
				GLenum cubeTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
				glTexSubImage2D(cubeTarget, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, pixels.data());
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

				/*
				// glCopyImageSubData拷贝直接在GPU内部执行，不经过CPU
				// glTexSubImage2D需要从GPU读取到CPU再写入GPU
				// glCopyImageSubData能正常执行，但是有不知原因Error，虽然Error没有影响，无视就好
				// 这里在注释中保留glCopyImageSubData的代码，有需要的时候可以直接切换
				// 执行拷贝
				glCopyImageSubData
				(
					srcID, GL_TEXTURE_2D, 0, 0, 0, 0,               // 源：纹理ID，目标类型，mip级别，x,y,z
					m_RendererID, GL_TEXTURE_CUBE_MAP, 0, 0, 0, i,  // 目标：纹理ID，目标类型（指定面），mip级别，x,y,z
					width, height, 1                                // 拷贝尺寸（宽度，高度，深度=1）
				);
				GLenum err = glGetError();
				if (err != GL_NO_ERROR) {
					VOL_CORE_ERROR("glCopyImageSubData failed for face {}! Error: 0x{:x}", i, err);
				}
				*/
			}
		}

		// 设置采样参数
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameterf(m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, RendererAPI::GetCapabilities().MaxAnisotropy);

		// 生成 mipmap（如果你需要）
		glGenerateTextureMipmap(m_RendererID);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	OpenGLTextureCube::~OpenGLTextureCube()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTextureCube::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTextureCube::SetData(const glm::vec4& color)
	{
		float clearColor[4] = { color.r, color.g, color.b, color.a };
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		glClearTexImage(m_RendererID, 0, m_InternalFormat, m_DataFormat, clearColor);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	uint32_t OpenGLTextureCube::GetInternalFormat()
	{
		return m_InternalFormat;
	}

	uint32_t OpenGLTextureCube::GetDataFormat()
	{
		return m_DataFormat;
	}

	uint32_t OpenGLTextureCube::GetMipLevelCount() const
	{
		return Texture::CalculateMipMapCount(m_Width, m_Height);
	}
}