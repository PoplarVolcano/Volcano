#include "volpch.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>

//#include "Volcano/Renderer/Renderer.h"

namespace Volcano {

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(void* vertices, uint32_t size)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
	{
		
		// 将 ID 为 m_RendererID 的缓冲区对象绑定到 GL_ARRAY_BUFFER 目标上。
		// GL_ARRAY_BUFFER 是一个 OpenGL 的状态目标（Target），代表顶点属性数据缓冲区。
		// 绑定后，所有针对 GL_ARRAY_BUFFER 的操作都会作用于 m_RendererID 这个缓冲区对象。
		// 这是一个上下文切换操作，后续的 glBufferSubData 将操作这个绑定的缓冲区。
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		// 更新当前绑定的缓冲区对象中的部分或全部数据，而不重新分配显存。
		// data:一个指向新数据源的指针，将新的数据源拷贝到缓冲区对象中完成更新
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
		:m_Count(count)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}