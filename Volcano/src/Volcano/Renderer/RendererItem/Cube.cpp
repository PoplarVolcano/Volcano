#include "volpch.h"
#include "Cube.h"

#include "Volcano/Renderer/Renderer.h"

namespace Volcano {
	
	Cube::Cube()
	{
		// 背 正 左 右 下 上
		glm::vec3 position[] =
		{
			{  0.5f, -0.5f, -0.5f },
			{ -0.5f, -0.5f, -0.5f },
			{ -0.5f,  0.5f, -0.5f },
			{ -0.5f,  0.5f, -0.5f },
			{  0.5f,  0.5f, -0.5f },
			{  0.5f, -0.5f, -0.5f },

			{ -0.5f, -0.5f,  0.5f },
			{  0.5f, -0.5f,  0.5f },
			{  0.5f,  0.5f,  0.5f },
			{  0.5f,  0.5f,  0.5f },
			{ -0.5f,  0.5f,  0.5f },
			{ -0.5f, -0.5f,  0.5f },

			{ -0.5f, -0.5f, -0.5f },
			{ -0.5f, -0.5f,  0.5f },
			{ -0.5f,  0.5f,  0.5f },
			{ -0.5f,  0.5f,  0.5f },
			{ -0.5f,  0.5f, -0.5f },
			{ -0.5f, -0.5f, -0.5f },

			{  0.5f, -0.5f,  0.5f },
			{  0.5f, -0.5f, -0.5f },
			{  0.5f,  0.5f, -0.5f },
			{  0.5f,  0.5f, -0.5f },
			{  0.5f,  0.5f,  0.5f },
			{  0.5f, -0.5f,  0.5f },

			{ -0.5f, -0.5f, -0.5f },
			{  0.5f, -0.5f, -0.5f },
			{  0.5f, -0.5f,  0.5f },
			{  0.5f, -0.5f,  0.5f },
			{ -0.5f, -0.5f,  0.5f },
			{ -0.5f, -0.5f, -0.5f },

			{ -0.5f,  0.5f,  0.5f },
			{  0.5f,  0.5f,  0.5f },
			{  0.5f,  0.5f, -0.5f },
			{  0.5f,  0.5f, -0.5f },
			{ -0.5f,  0.5f, -0.5f },
			{ -0.5f,  0.5f,  0.5f }
		};

		glm::vec2 texCoord[] =
		{
			{ 0.0f, 0.0f},
			{ 1.0f, 0.0f},
			{ 1.0f, 1.0f},
			{ 1.0f, 1.0f},
			{ 0.0f, 1.0f},
			{ 0.0f, 0.0f}
		};

		glm::vec3 normal[] =
		{
			{ 0.0f,  0.0f, -1.0f},
			{ 0.0f,  0.0f,  1.0f},
			{-1.0f,  0.0f,  0.0f},
			{ 1.0f,  0.0f,  0.0f},
			{ 0.0f, -1.0f,  0.0f},
			{ 0.0f,  1.0f,  0.0f}
		};

		glm::vec3 tangent[] =
		{
			{-1.0f,  0.0f,  0.0f},
			{ 1.0f,  0.0f,  0.0f},
			{ 0.0f,  0.0f,  1.0f},
			{ 0.0f,  0.0f, -1.0f},
			{ 1.0f,  0.0f,  0.0f},
			{ 1.0f,  0.0f,  0.0f}
		};

		glm::vec3 bitangent[] =
		{
			{ 0.0f,  1.0f,  0.0f},
			{ 0.0f,  1.0f,  0.0f},
			{ 0.0f,  1.0f,  0.0f},
			{ 0.0f,  1.0f,  0.0f},
			{ 0.0f,  0.0f,  1.0f},
			{ 0.0f,  0.0f, -1.0f}
		};

		m_VertexSize = 36;
		m_IndexSize = 36;
		m_Vertices.reserve(m_VertexSize);
		m_Indices.reserve(m_IndexSize);

		for (uint32_t i = 0; i < m_VertexSize; i++)
		{
			m_Vertices.emplace_back(
				position[i],
				texCoord[i % 6],
				normal[i / 6],
				tangent[i / 6],
				bitangent[i / 6]
				);
		}

		for (uint32_t i = 0; i < m_IndexSize; i++)
			m_Indices.emplace_back(i);

		m_VertexArray = VertexArray::Create();
		m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), m_Vertices.size() * sizeof(MeshVertex));
		m_VertexBuffer->SetLayout(m_BufferLayout);
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(m_Indices.data(), m_IndexSize);
		m_VertexArray->SetIndexBuffer(indexBuffer);

	}

	Cube::~Cube()
	{
	}

	void Cube::FlushInstances()
	{
		Renderer::DrawIndexInstanced(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount(), m_InstanceDataList.size());
	}


}