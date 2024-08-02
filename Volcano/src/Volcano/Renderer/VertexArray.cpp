
#include "volpch.h"
#include "VertexArray.h"
#include"Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Volcano {
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::None:   VOL_CORE_ASSERT(false, "Buffer��APIΪNone��֧��"); return nullptr;
			case RendererAPI::OpenGL: return new OpenGLVertexArray();
		}

		VOL_CORE_ASSERT(false, "Buffer��δ֪API");
		return nullptr;
	}
}