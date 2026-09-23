#include "volpch.h"
#include "Plane.h"

#include "Volcano/Renderer/Renderer.h"

namespace Volcano {

	Plane::Plane()
	{
		glm::vec3 vertexPosition[] = {
		  { -0.5, 0.0f,  0.5},
		  {  0.5, 0.0f,  0.5},
		  {  0.5, 0.0f, -0.5},
		  {  0.5, 0.0f, -0.5},
		  { -0.5, 0.0f, -0.5},
		  { -0.5, 0.0f,  0.5}
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

		const uint32_t row = 10;
		const uint32_t column = 10;
		m_VertexSize = 6 * row * column;
		m_IndexSize = 6 * row * column;
		m_Vertices.reserve(m_VertexSize);
		m_Indices.reserve(m_IndexSize);

		for (uint32_t i = 0;i < 6; i++)
		{
			texCoord[i] = texCoord[i] / glm::vec2((float)column, (float)row);
		}


		float reciprocalRow = 1.0f / (float)row;
		float reciprocalColumn = 1.0f / (float)column;

		for (uint32_t rowTemp = 0; rowTemp != row; rowTemp++)
		{
			float rowOffset = (float)rowTemp - (float)row * 0.5f + 0.5f;
			for (uint32_t columnTemp = 0; columnTemp != column; columnTemp++)
			{
				float columnOffset = (float)columnTemp - (float)column * 0.5f + 0.5f;
				uint32_t quadIndex = (columnTemp + rowTemp * column) * 6;
				for (uint32_t i = 0; i < 6; i++)
				{
					uint32_t index = i + quadIndex;
					m_Vertices.emplace_back(
						vertexPosition[i] + glm::vec3(columnOffset, 0, -rowOffset),
						texCoord[i] + glm::vec2(columnTemp * reciprocalColumn, rowTemp * reciprocalRow),
						glm::vec3(0.0f, 1.0f, 0.0f),
						glm::vec3(1.0f, 0.0f, 0.0f),					
						glm::vec3(0.0f, 0.0f, 1.0f)
						);
				}
			}
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

	Plane::~Plane()
	{
	}

	void Plane::FlushInstances()
	{
		Renderer::DrawIndexInstanced(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount(), m_InstanceDataList.size());
	}

}