#include "volpch.h"
#include "Texture.h"

#include"Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Volcano {

	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:   VOL_CORE_ASSERT(false, "Buffer：API为None不支持"); return nullptr;
			case RendererAPI::API::OpenGL: return std::make_shared<OpenGLTexture2D>(path);
		}

		VOL_CORE_ASSERT(false, "Buffer：未知API");
		return nullptr;
	}
}