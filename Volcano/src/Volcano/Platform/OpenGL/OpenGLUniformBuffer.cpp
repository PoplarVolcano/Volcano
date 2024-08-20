#include "volpch.h"
#include "OpenGLUniformBuffer.h"

#include <glad/glad.h>

namespace Volcano {

	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW); // TODO: investigate usage hint
		// ����glsl�����õ�bingding 0�Ż������������Ļ�����m_RendererID��ϵ����
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}


	void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		// �ϴ����ݸ�m_RendererID�Ż������ɣ�ʵ���GPU��bingding�Ż�����
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}


}