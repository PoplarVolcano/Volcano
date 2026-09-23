#include "volpch.h"
#include "Quad.h"

#include "Volcano/Renderer/Renderer.h"

namespace Volcano {

	Quad::Quad()
	{
		glm::vec4 position[] = {
			{ -0.5, -0.5, 0.0f, 1.0f },
		    {  0.5, -0.5, 0.0f, 1.0f },
		    {  0.5,  0.5, 0.0f, 1.0f },
		    { -0.5,  0.5, 0.0f, 1.0f }
		};

		glm::vec2 texCoord[] = {
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 1.0f }
		};

		int indices[] = { 0, 1, 2, 2, 3, 0 };

		m_VertexSize = 4;
		m_IndexSize = 6;
		m_Vertices.reserve(m_VertexSize);
		m_Indices.reserve(m_IndexSize);

		for (uint32_t i = 0; i < m_VertexSize; i++)
		{
			m_Vertices.emplace_back(
				position[i],
				texCoord[i],
				glm::vec3(0.0f, 0.0f, 1.0f),
				glm::vec3(1.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f)
			);
		}

		for (uint32_t i = 0; i < m_IndexSize; i++)
			m_Indices.emplace_back(indices[i]);

		m_VertexArray = VertexArray::Create();
		m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), m_Vertices.size() * sizeof(MeshVertex));
		m_VertexBuffer->SetLayout(m_BufferLayout);
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(m_Indices.data(), m_IndexSize);
		m_VertexArray->SetIndexBuffer(indexBuffer);

	}

	Quad::~Quad()
	{
	}

	void Quad::FlushInstances()
	{
		Renderer::DrawIndexInstanced(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount(), m_InstanceDataList.size());
	}

}