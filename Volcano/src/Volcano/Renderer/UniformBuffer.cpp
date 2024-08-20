#include "volpch.h"
#include "UniformBuffer.h"
#include "Volcano/Renderer/RendererAPI.h"
#include "Volcano/Platform/OpenGL/OpenGLUniformBuffer.h"

namespace Volcano {

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    VOL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPIType::OpenGL:  return CreateRef<OpenGLUniformBuffer>(size, binding);
		}

		VOL_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}