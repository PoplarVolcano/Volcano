#include "volpch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Volcano/Platform/OpenGL/OpenGLTexture.h"

namespace Volcano {
	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:    VOL_CORE_ASSERT(false, "Texture2D：API为None不支持"); return nullptr;
			case RendererAPIType::OpenGL:  return  CreateRef<OpenGLTexture2D>(width, height);
		}
		VOL_CORE_ASSERT(false, "Buffer：未知API");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path, bool filp)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:    VOL_CORE_ASSERT(false, "Texture2D：API为None不支持"); return nullptr;
			case RendererAPIType::OpenGL:  return  std::make_shared<OpenGLTexture2D>(path, filp);
		}
		VOL_CORE_ASSERT(false, "Buffer：未知API");
		return nullptr;
	}

	// ==================================TextureCube=================================================

	Ref<TextureCube> TextureCube::Create(TextureFormat format, uint32_t width, uint32_t height)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None: return nullptr;
		case RendererAPIType::OpenGL: return CreateRef<OpenGLTextureCube>(format, width, height);
		}
		return nullptr;
	}

	Ref<TextureCube> TextureCube::Create(const std::string& path)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None: return nullptr;
		case RendererAPIType::OpenGL: return CreateRef<OpenGLTextureCube>(path);
		}
		return nullptr;
	}

	Ref<TextureCube> TextureCube::Create(std::vector<std::string> faces)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None: return nullptr;
		case RendererAPIType::OpenGL: return CreateRef<OpenGLTextureCube>(faces);
		}
		return nullptr;
	}

	uint32_t Texture::GetBPP(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::RGB:    return 3;
		case TextureFormat::RGBA:   return 4;
		}
		return 0;
	}

	uint32_t Texture::CalculateMipMapCount(uint32_t width, uint32_t height)
	{
		uint32_t levels = 1;
		while ((width | height) >> levels)
			levels++;

		return levels;
	}

}