#pragma once

namespace Volcano {

	class UniformBuffer
	{
	public:
		virtual ~UniformBuffer() {}
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

		static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);
	};

	class UniformBufferManager
	{
	public:
		static void Init();
		static Ref<UniformBuffer> GetUniformBuffer(std::string name) { return m_UniformBuffers[name]; }
	private:
		UniformBufferManager();
		static std::unordered_map<std::string, Ref<UniformBuffer>> m_UniformBuffers;
	};
}