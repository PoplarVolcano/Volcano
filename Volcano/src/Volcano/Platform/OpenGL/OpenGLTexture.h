#pragma once
#include "Volcano/Renderer/Texture.h"

namespace Volcano {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height, TextureInternalFormat internalFormat, TextureDataFormat dataFormat, TextureWrap wrap = TextureWrap::REPEAT);
		OpenGLTexture2D(const std::string& absolutePath, bool flip = true, TextureInternalFormat internalFormat = TextureInternalFormat::NONE);
		virtual ~OpenGLTexture2D();

		virtual void Bind(uint32_t slot = 0) const override;

		virtual void SetData(void* data, uint32_t size) override;
		virtual void SetDataFormat(TextureDataFormat format) override;
		virtual void SetInternalFormat(TextureInternalFormat format) override;
		virtual uint32_t GetDataFormat() override;
		virtual uint32_t GetInternalFormat() override;
		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetMipLevelCount() const override;

		virtual uint32_t GetRendererID() const override { return m_RendererID; }
		virtual uint64_t GetHandle() const override { return m_Handle; }
		virtual const std::string& GetRelativePath() const override { return m_FileRelativePath; }

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == other.GetRendererID();
		};
	private:
		uint32_t m_RendererID = 0;
		uint64_t m_Handle = 0;
		uint32_t m_Width, m_Height;
		uint32_t m_InternalFormat = 0, m_DataFormat = 0;

		std::string m_FileRelativePath;
	};

	class OpenGLTextureCube : public TextureCube
	{
	public:
		OpenGLTextureCube(
			uint32_t width,
			uint32_t height,
			TextureInternalFormat internalFormat,
			TextureDataFormat dataFormat,
			TextureWrap wrap,
			TextureFilter minFilter,
			TextureFilter maxFilter
		);

		OpenGLTextureCube(const std::string& absolutePath);
		OpenGLTextureCube(const std::array<Ref<Texture2D>, 6>& faces);
		virtual ~OpenGLTextureCube();

		virtual void Bind(uint32_t slot = 0) const;

		virtual void SetData(const glm::vec4& color) override;

		virtual uint32_t GetInternalFormat() override;
		virtual uint32_t GetDataFormat() override;
		virtual uint32_t GetWidth() const { return m_Width; }
		virtual uint32_t GetHeight() const { return m_Height; }
		virtual uint32_t GetMipLevelCount() const override;

		virtual uint32_t GetRendererID() const override { return m_RendererID; }
		virtual uint64_t GetHandle() const override { return m_Handle; }

		virtual const std::string& GetRelativePath() const override { return m_FileRelativePath; }

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == other.GetRendererID();
		}
	private:
		uint32_t m_RendererID;
		uint64_t m_Handle = 0;
		uint32_t m_InternalFormat, m_DataFormat;
		uint32_t m_Width, m_Height;
		std::string m_FileRelativePath = "";

		unsigned char* m_ImageData;
	};
}