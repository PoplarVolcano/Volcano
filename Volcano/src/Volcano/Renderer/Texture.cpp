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

}