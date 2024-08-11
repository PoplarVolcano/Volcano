#include "volpch.h"
#include "GraphicsContext.h"
#include "RendererAPI.h"
#include <Volcano/Platform/OpenGL/OpenGLContext.h>

namespace Volcano {

	Ref<GraphicsContext> GraphicsContext::Create(GLFWwindow* windowHandle)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:   VOL_CORE_ASSERT(false, "Buffer：API为None不支持"); return nullptr;
		case RendererAPIType::OpenGL: return std::make_shared<OpenGLContext>(windowHandle);
		}

		VOL_CORE_ASSERT(false, "Buffer：未知API");
		return nullptr;
	}
}
