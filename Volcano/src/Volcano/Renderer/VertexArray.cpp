#include "volpch.h"
#include "VertexArray.h"

#include"Renderer.h"
#include "Volcano/Platform/OpenGL/OpenGLVertexArray.h"

namespace Volcano {
	Ref<VertexArray> VertexArray::Create()
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:   VOL_CORE_ASSERT(false, "Buffer��APIΪNone��֧��"); return nullptr;
		case RendererAPIType::OpenGL: return std::make_shared<OpenGLVertexArray>();
		}

		VOL_CORE_ASSERT(false, "Buffer��δ֪API");
		return nullptr;
	}
}