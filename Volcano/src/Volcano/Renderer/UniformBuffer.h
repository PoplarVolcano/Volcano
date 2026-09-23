#pragma once

#include "Volcano/Core/Core.h"

namespace Volcano {

	enum UniformBlockType
	{
		UniformBlock,      // Uniform Block (UBO)        layout(std140) uniform LightBlock { ... };
		ShaderStorageBlock // Shader Storage Block(SSBO) layout(std430) readonly buffer LightBuffer { ... };
	};

	class UniformBuffer
	{
	public:
		virtual ~UniformBuffer() {}
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

		static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding, UniformBlockType uniformBlockType = UniformBlockType::UniformBlock);
	};

	class VOL_API UniformBufferManager
	{
	public:
		static void Init();
		static Ref<UniformBuffer> GetUniformBuffer(std::string name) { return m_UniformBuffers[name]; }
	private:
		UniformBufferManager();
		static std::unordered_map<std::string, Ref<UniformBuffer>> m_UniformBuffers;
	};
}