#include "volpch.h"
#include "GraphicsContext.h"
#include "RendererAPI.h"
#include <Volcano/Platform/OpenGL/OpenGLContext.h>

namespace Volcano {

	Ref<GraphicsContext> GraphicsContext::Create(GLFWwindow* windowHandle)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:   VOL_CORE_ASSERT(false, "Buffer��APIΪNone��֧��"); return nullptr;
		case RendererAPIType::OpenGL: return std::make_shared<OpenGLContext>(windowHandle);
		}

		VOL_CORE_ASSERT(false, "Buffer��δ֪API");
		return nullptr;
	}
}
