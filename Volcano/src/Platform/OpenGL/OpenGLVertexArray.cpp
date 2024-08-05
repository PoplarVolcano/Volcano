#include "volpch.h"
#include "OpenGLVertexArray.h"
#include <glad/glad.h>

namespace Volcano {

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
		switch (type)
		{
			case Volcano::ShaderDataType::Float:     return GL_FLOAT;
			case Volcano::ShaderDataType::Float2:    return GL_FLOAT;
			case Volcano::ShaderDataType::Float3:    return GL_FLOAT;
			case Volcano::ShaderDataType::Float4:    return GL_FLOAT;
			case Volcano::ShaderDataType::Mat3:      return GL_FLOAT;
			case Volcano::ShaderDataType::Mat4:      return GL_FLOAT;
			case Volcano::ShaderDataType::Int:       return GL_INT;
			case Volcano::ShaderDataType::Int2:      return GL_INT;
			case Volcano::ShaderDataType::Int3:      return GL_INT;
			case Volcano::ShaderDataType::Int4:      return GL_INT;
			case Volcano::ShaderDataType::Bool:      return GL_BOOL;
		}

		VOL_CORE_ASSERT(false, "Unknow ShaderDataType!");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::UnBind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		VOL_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "OpenGLVertexArray:VertexBuffer没有布局（Layout）");
		
		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout) {
			glEnableVertexAttribArray(index);
			//设置缓冲区数据格式：缓冲区序号、顶点属性的大小、什么数据类型、会不会被归一化、
			glVertexAttribPointer(index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset);
			index++;
		}
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffers = indexBuffer;
	}
}