#pragma once
#include "Volcano/Renderer/Texture.h"

namespace Volcano {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height);
		OpenGLTexture2D(const std::string& path, bool srgb);
		virtual ~OpenGLTexture2D();

		virtual void Bind(uint32_t slot = 0) const override;

		virtual void SetData(void* data, uint32_t size) override;
		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }

		virtual uint32_t GetRendererID() const override { return m_RendererID; }

		virtual bool operator==(const Texture& other) const override 
		{ 
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID; 
		};
	private:
		uint32_t m_RendererID;
		uint32_t m_Width, m_Height;
		uint32_t m_InternalFormat = 0, m_DataFormat = 0;

		std::string m_FilePath;
	};

}