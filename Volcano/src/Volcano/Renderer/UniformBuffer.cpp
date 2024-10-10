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

	std::unordered_map<std::string, Ref<UniformBuffer>> UniformBufferManager::m_UniformBuffers;

	void UniformBufferManager::Init()
	{
		UniformBufferManager::m_UniformBuffers["CameraViewProjection"] = UniformBuffer::Create(4 * 4 * sizeof(float), 0);
		UniformBufferManager::m_UniformBuffers["CameraPosition"]       = UniformBuffer::Create(4 * sizeof(float), 1);
		
		UniformBufferManager::m_UniformBuffers["DirectionalLight"]            = UniformBuffer::Create((4 + 4 + 4 + 4) * sizeof(float), 2);
		UniformBufferManager::m_UniformBuffers["PointLight"]                  = UniformBuffer::Create(4 * (4 + 4 + 4 + 3 + 1 + 1 + 1 + 2/* 2用于填充std140的空位*/) * sizeof(float), 3); // 最多4个点光源
		UniformBufferManager::m_UniformBuffers["SpotLight"]                   = UniformBuffer::Create(4 * (4 + 4 + 4 + 4 + 3 + 1 + 1 + 1 + 1 + 1) * sizeof(float), 4);

		UniformBufferManager::m_UniformBuffers["Material"]                    = UniformBuffer::Create(sizeof(float), 5);

		UniformBufferManager::m_UniformBuffers["ModelTransform"]              = UniformBuffer::Create(4 * 4 * 2 * sizeof(float), 6);

		UniformBufferManager::m_UniformBuffers["CameraData"]                  = UniformBuffer::Create(4 * 4 * 2 * sizeof(float), 7);

		UniformBufferManager::m_UniformBuffers["DirectionalLightSpaceMatrix"] = UniformBuffer::Create(4 * 4 * sizeof(float), 8);
		UniformBufferManager::m_UniformBuffers["PointLightSpaceMatrix"]       = UniformBuffer::Create((4 * 4 * 6 + 1) * sizeof(float), 9);
		UniformBufferManager::m_UniformBuffers["SpotLightSpaceMatrix"]        = UniformBuffer::Create((4 * 4 + 1) * sizeof(float), 10);


		UniformBufferManager::m_UniformBuffers["Exposure"] = UniformBuffer::Create(2 * sizeof(float), 11);
		UniformBufferManager::m_UniformBuffers["Blur"]     = UniformBuffer::Create(sizeof(float), 12);

		const uint32_t ssaoKernelSize = 64;
		UniformBufferManager::m_UniformBuffers["Samples"]  = UniformBuffer::Create(ssaoKernelSize * 4 * sizeof(float), 13);
		UniformBufferManager::m_UniformBuffers["SSAO"]     = UniformBuffer::Create(4 * sizeof(float), 14);

		UniformBufferManager::m_UniformBuffers["BonesMatrices"] = UniformBuffer::Create(100 * 4 * 4 * sizeof(float), 15);

		UniformBufferManager::m_UniformBuffers["PBR"]       = UniformBuffer::Create(4 * 4 * sizeof(float), 16);
		UniformBufferManager::m_UniformBuffers["Prefilter"] = UniformBuffer::Create(sizeof(float), 17);

	}

}