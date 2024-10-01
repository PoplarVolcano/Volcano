#include "volpch.h"
#include "RenderPass.h"
#include "Volcano/Renderer/RendererAPI.h"
#include "Volcano/Platform/OpenGL/OpenGLRenderPass.h"

namespace Volcano {

	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    VOL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPIType::OpenGL:  return std::make_shared<OpenGLRenderPass>(spec);
		}

		VOL_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}