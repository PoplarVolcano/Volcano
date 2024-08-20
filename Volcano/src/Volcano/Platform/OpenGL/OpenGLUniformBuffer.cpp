#include "volpch.h"
#include "OpenGLUniformBuffer.h"

#include <glad/glad.h>

namespace Volcano {

	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW); // TODO: investigate usage hint
		// 将在glsl上设置的bingding 0号缓冲区与真正的缓冲区m_RendererID联系起来
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}


	void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		// 上传数据给m_RendererID号缓冲区吧，实则给GPU的bingding号缓冲区
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}


}