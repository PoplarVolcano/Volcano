#include "volpch.h"
#include "OpenGLUniformBuffer.h"

#include <glad/glad.h>

namespace Volcano {

	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding, UniformBlockType uniformBlockType)
		: m_Size(size)
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);

		// 将在glsl上设置的bingding 0号缓冲区与真正的缓冲区m_RendererID联系起来
		switch (uniformBlockType)
		{
		case UniformBlockType::UniformBlock:
			glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
			break;
		case UniformBlockType::ShaderStorageBlock:
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_RendererID);
			break;
		default:
			break;
		}
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}


	void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		if (!data || size == 0)
			return;

		// 检查是否需要扩容
		if (offset + size > m_Size)
		{
			// 扩容策略：至少为 (offset + size)，并乘以 1.5 或 2 倍，避免频繁扩容
			uint32_t newSize = std::max<uint32_t>(offset + size, m_Size * 2);

			// 重新分配存储，丢弃旧数据（每帧重新上传）
			glNamedBufferData(m_RendererID, newSize, nullptr, GL_DYNAMIC_DRAW);
			m_Size = newSize;
		}

		// 上传数据给m_RendererID号缓冲区，实则给GPU的bingding号缓冲区
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}


}