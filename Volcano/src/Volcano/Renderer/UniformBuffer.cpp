#include "volpch.h"
#include "UniformBuffer.h"
#include "Volcano/Renderer/RendererAPI.h"
#include "Volcano/Renderer/LightRenderer.h"
#include "Volcano/Platform/OpenGL/OpenGLUniformBuffer.h"
#include "Volcano/Renderer/InstanceData.h"
#include "Volcano/Renderer/Material.h"
#include "Camera.h"
#include "Volcano/Scene/Scene.h"

namespace Volcano {

	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding, UniformBlockType uniformBlockType)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    VOL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPIType::OpenGL:  return CreateRef<OpenGLUniformBuffer>(size, binding, uniformBlockType);
		}

		VOL_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::unordered_map<std::string, Ref<UniformBuffer>> UniformBufferManager::m_UniformBuffers;

	void UniformBufferManager::Init()
	{
		UniformBufferManager::m_UniformBuffers["CameraData"] = UniformBuffer::Create(sizeof(CameraData), 0);

		const uint32_t uintOffset = (sizeof(uint32_t) + 15) & ~15; // 将“加 15 后的值”与“掩码”进行按位与操作，实现‌向上取整到最近的 16 倍数‌。
		UniformBufferManager::m_UniformBuffers["DirectionalLight"] =
			UniformBuffer::Create(uintOffset + LightRenderer::MaxDirectionalLight * sizeof(DirectionalLight), 2, UniformBlockType::ShaderStorageBlock);
		UniformBufferManager::m_UniformBuffers["PointLight"] =
			UniformBuffer::Create(uintOffset + LightRenderer::MaxPointLight * sizeof(PointLight), 3, UniformBlockType::ShaderStorageBlock);
		UniformBufferManager::m_UniformBuffers["SpotLight"] =
			UniformBuffer::Create(uintOffset + LightRenderer::MaxSpotLight * sizeof(SpotLight), 4, UniformBlockType::ShaderStorageBlock);
		
		UniformBufferManager::m_UniformBuffers["Material"]                    = UniformBuffer::Create(sizeof(float), 5);
		
		UniformBufferManager::m_UniformBuffers["LightCount"]                  = UniformBuffer::Create(3 * sizeof(int), 6);
		
		UniformBufferManager::m_UniformBuffers["DirectionalLightShadowData"]  = UniformBuffer::Create(sizeof(glm::mat4), 8);
		UniformBufferManager::m_UniformBuffers["PointLightShadowData"]        = UniformBuffer::Create(sizeof(PointLightShadowData), 9);
		UniformBufferManager::m_UniformBuffers["SpotLightShadowData"]         = UniformBuffer::Create(sizeof(glm::mat4), 10);
		
		UniformBufferManager::m_UniformBuffers["PostProcessing"]              = UniformBuffer::Create(10 * sizeof(float), 13);
		UniformBufferManager::m_UniformBuffers["Exposure"]                    = UniformBuffer::Create(sizeof(float), 14);
		UniformBufferManager::m_UniformBuffers["GaussianBlur"]                = UniformBuffer::Create(sizeof(bool), 15);
		UniformBufferManager::m_UniformBuffers["BloomEnabled"]                = UniformBuffer::Create(sizeof(bool), 16);

		const uint32_t ssaoKernelSize = 64;
		UniformBufferManager::m_UniformBuffers["Samples"]                     = UniformBuffer::Create(ssaoKernelSize * 4 * sizeof(float), 17);
		UniformBufferManager::m_UniformBuffers["SSAO"]                        = UniformBuffer::Create(sizeof(SSAOData), 18);

		UniformBufferManager::m_UniformBuffers["IrradianceEnabled"]           = UniformBuffer::Create(sizeof(bool), 19);

		UniformBufferManager::m_UniformBuffers["MaterialHandle"] =
			UniformBuffer::Create(1024u * sizeof(MaterialHandle), 20, UniformBlockType::ShaderStorageBlock);
		UniformBufferManager::m_UniformBuffers["InstanceData"] =
			UniformBuffer::Create(s_MaxInstanceData * sizeof(InstanceData), 21, UniformBlockType::ShaderStorageBlock);
		UniformBufferManager::m_UniformBuffers["InstanceDataMaterial"] =
			UniformBuffer::Create(s_MaxInstanceData * sizeof(InstanceDataMaterial), 22, UniformBlockType::ShaderStorageBlock);
		UniformBufferManager::m_UniformBuffers["InstanceDataExplosion"] =
			UniformBuffer::Create(s_MaxInstanceData * sizeof(InstanceDataExplosion), 23, UniformBlockType::ShaderStorageBlock);
		UniformBufferManager::m_UniformBuffers["InstanceDataOutline"] =
			UniformBuffer::Create(s_MaxInstanceData * sizeof(InstanceDataOutline), 24, UniformBlockType::ShaderStorageBlock);
		UniformBufferManager::m_UniformBuffers["InstanceDataNormalVisualization"] =
			UniformBuffer::Create(s_MaxInstanceData * sizeof(InstanceDataNormalVisualization), 25, UniformBlockType::ShaderStorageBlock);
		UniformBufferManager::m_UniformBuffers["InstanceDataEntityID"] =
			UniformBuffer::Create(s_MaxInstanceData * sizeof(InstanceDataEntityID), 29, UniformBlockType::ShaderStorageBlock);
		UniformBufferManager::m_UniformBuffers["InstanceDataLightingMode"] =
			UniformBuffer::Create(s_MaxInstanceData * sizeof(InstanceDataLightingMode), 30, UniformBlockType::ShaderStorageBlock);
		
		
		UniformBufferManager::m_UniformBuffers["TemporaryMat4"] = UniformBuffer::Create(4 * 4 * sizeof(float), 40);
		UniformBufferManager::m_UniformBuffers["TemporaryFloat"] = UniformBuffer::Create(sizeof(float), 41);



	}

}