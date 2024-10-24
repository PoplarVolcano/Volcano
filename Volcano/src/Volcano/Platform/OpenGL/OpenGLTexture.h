#pragma once
#include "Volcano/Renderer/Texture.h"

namespace Volcano {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height, TextureFormat internalFormat, TextureFormat dataFormat, TextureWrap wrap = TextureWrap::REPEAT);
		OpenGLTexture2D(const std::string& path, bool filp = true, TextureFormat internalFormat = TextureFormat::None);
		virtual ~OpenGLTexture2D();

		virtual void Bind(uint32_t slot = 0) const override;

		virtual void SetData(void* data, uint32_t size) override;
		virtual void SetDataFormat(TextureFormat format) override;
		virtual void SetInternalFormat(TextureFormat format) override;
		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetMipLevelCount() const override;

		virtual uint32_t GetRendererID() const override { return m_RendererID; }
		virtual const std::string& GetPath() const override { return m_FilePath; }

		virtual bool operator==(const Texture& other) const override 
		{ 
			return m_RendererID == other.GetRendererID(); 
		};
	private:
		uint32_t m_RendererID;
		uint32_t m_Width, m_Height;
		uint32_t m_InternalFormat = 0, m_DataFormat = 0;

		std::string m_FilePath;
	};

	class OpenGLTextureCube : public TextureCube
	{
	public:
		OpenGLTextureCube(TextureFormat format, uint32_t width, uint32_t height);
		OpenGLTextureCube(const std::string& path);
		OpenGLTextureCube(std::vector<std::string> faces);
		virtual ~OpenGLTextureCube();

		virtual void Bind(uint32_t slot = 0) const;

		virtual TextureFormat GetFormat() const { return m_Format; }
		virtual uint32_t GetWidth() const { return m_Width; }
		virtual uint32_t GetHeight() const { return m_Height; }
		virtual uint32_t GetMipLevelCount() const override;

		virtual uint32_t GetRendererID() const override { return m_RendererID; }
		virtual const std::string& GetPath() const override { return m_FilePath; }

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == other.GetRendererID();
		}
	private:
		uint32_t m_RendererID;
		TextureFormat m_Format;
		uint32_t m_Width, m_Height;
		std::string m_FilePath;

		unsigned char* m_ImageData;
	};
}