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
		VOL_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "OpenGLVertexArray:VertexBufferû�в��֣�Layout��");

		Bind();
		vertexBuffer->Bind();
		const auto& layout = vertexBuffer->GetLayout();
		// shader���ж��layout��ÿ��layout��Ӧһ������
		// ����ÿ��layout����һ������
		for (const auto& element : layout)
		{
			auto glBaseType = ShaderDataTypeToOpenGLBaseType(element.Type);
			//�������ݵ�����m_VertexBufferIndex
			glEnableVertexAttribArray(m_VertexBufferIndex);
			if (glBaseType == GL_INT)
			{
				//���û��������ݸ�ʽ����������š��������ԵĴ�С��ʲô�������͡��᲻�ᱻ��һ�������ࡢƫ����
				glVertexAttribIPointer(m_VertexBufferIndex,
					element.GetComponentCount(),             // �����м������ݣ�float3��Ӧvec3��3�����ݣ�
					glBaseType,                              // ����(layout)������(��float3Ϊfloat����)
					layout.GetStride(),                      // ÿһ�ζ�ȡ�Ĳ���
					(const void*)(intptr_t)element.Offset);  // ƫ����
			}
			else
			{
				glVertexAttribPointer(m_VertexBufferIndex,
					element.GetComponentCount(),
					glBaseType,
					element.Normalized ? GL_TRUE : GL_FALSE,  // �����Ƿ��һ��
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