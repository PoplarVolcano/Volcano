#include "volpch.h"
#include "Framebuffer.h"
#include "Volcano/Renderer/RendererAPI.h"
#include "Volcano/Platform/OpenGL/OpenGLFramebuffer.h"

namespace Volcano {
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:   VOL_CORE_ASSERT(false, "Buffer��APIΪNone��֧��"); return nullptr;
		case RendererAPIType::OpenGL: return std::make_shared<OpenGLFramebuffer>(spec);
		}

		VOL_CORE_ASSERT(false, "Buffer��δ֪API");
		return nullptr;
	}
}