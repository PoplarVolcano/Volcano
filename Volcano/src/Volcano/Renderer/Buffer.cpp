#include "volpch.h"
#include "Buffer.h"

#include "Renderer.h"

#include "Volcano/Platform/OpenGL/OpenGLBuffer.h"

namespace Volcano {

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:   VOL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPIType::OpenGL: return std::make_shared<OpenGLVertexBuffer>(size);
		}
		VOL_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(void* vertices, uint32_t size)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:   VOL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPIType::OpenGL: return std::make_shared<OpenGLVertexBuffer>(vertices, size);
		}
		VOL_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}


	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPIType::None:   VOL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPIType::OpenGL: return std::make_shared<OpenGLIndexBuffer>(indices, count);

		}
		VOL_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}