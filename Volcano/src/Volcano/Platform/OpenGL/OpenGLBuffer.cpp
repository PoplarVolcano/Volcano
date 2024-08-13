#include "volpch.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>

#include "Volcano/Renderer/Renderer.h"

namespace Volcano {

	///////////////////////////////////////////////////////////////
	// VertexBuffer ///////////////////////////////////////////////
	///////////////////////////////////////////////////////////////

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
	{
		//Renderer::Submit([=]() {
			glCreateBuffers(1, &m_RendererID);
			glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
			glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		//	});
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size)
	{
		//Renderer::Submit([=]() {
			glCreateBuffers(1, &m_RendererID);
			glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
			glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
		//	});
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		//Renderer::Submit([this]() {
			glDeleteBuffers(1, &m_RendererID);
		//	});
	}

	void OpenGLVertexBuffer::Bind() const
	{
		//Renderer::Submit([this]() {
			glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		//	});
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		//Renderer::Submit([this]() {
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		//	});
	}

	void OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
	{
		//Renderer::Submit([=]() {
			glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
			// 用来更新一个已有缓冲区对象中的一部分数据，
			//data:一个指向新数据源的指针，将新的数据源拷贝到缓冲区对象中完成更新
			glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
		//	});
	}

	///////////////////////////////////////////////////////////////
	// IndevBuffer ////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
		:m_Count(count)
	{
		//Renderer::Submit([=]() {
			glCreateBuffers(1, &m_RendererID);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
		//	});
	}
	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		//Renderer::Submit([this]() {
			glDeleteBuffers(1, &m_RendererID);
		//	});
	}

	void OpenGLIndexBuffer::Bind() const
	{
		//Renderer::Submit([this]() {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		//	});
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		//Renderer::Submit([this]() {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		//	});
	}
}