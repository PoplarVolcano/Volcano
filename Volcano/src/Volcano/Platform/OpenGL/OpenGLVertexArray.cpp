#include "volpch.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

#include "Volcano/Renderer/Renderer.h"

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

		Bind();
		vertexBuffer->Bind();
		const auto& layout = vertexBuffer->GetLayout();
		// shader中有多个layout，每个layout对应一个索引
		// 对于每个layout进行一次设置
		for (const auto& element : layout)
		{
			auto glBaseType = ShaderDataTypeToOpenGLBaseType(element.Type);
			//启用数据的索引m_VertexBufferIndex
			glEnableVertexAttribArray(m_VertexBufferIndex);
			if (glBaseType == GL_INT)
			{
				//设置缓冲区数据格式：缓冲区序号、顶点属性的大小、什么数据类型、会不会被归一化、步距、偏移量
				glVertexAttribIPointer(m_VertexBufferIndex,
					element.GetComponentCount(),             // 布局有几个数据（float3对应vec3有3个数据）
					glBaseType,                              // 布局(layout)的类型(如float3为float类型)
					layout.GetStride(),                      // 每一次读取的步距
					(const void*)(intptr_t)element.Offset);  // 偏移量
			}
			else
			{
				glVertexAttribPointer(m_VertexBufferIndex,
					element.GetComponentCount(),
					glBaseType,
					element.Normalized ? GL_TRUE : GL_FALSE,  // 数据是否归一化
					layout.GetStride(),
					(const void*)(intptr_t)element.Offset);
			}
			m_VertexBufferIndex++;
		}
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		Bind();
		indexBuffer->Bind();

		m_IndexBuffers = indexBuffer;
	}
}