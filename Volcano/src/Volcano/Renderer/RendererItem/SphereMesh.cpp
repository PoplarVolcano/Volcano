#include "volpch.h"
#include "SphereMesh.h"
#include "Volcano/Renderer/Renderer.h"

namespace Volcano {


	SphereMesh::SphereMesh()
	{
		const uint32_t X_SEGMENTS = 64; // β角
		const uint32_t Y_SEGMENTS = 64; // α角
		const uint32_t VertexSize = (X_SEGMENTS + 1) * (Y_SEGMENTS + 1);
		const uint32_t IndexSize = (X_SEGMENTS + 1) * Y_SEGMENTS * 2;

		m_VertexSize = VertexSize;
		m_IndexSize = IndexSize;
		MaxMeshes = 1;
		MaxVertices = MaxMeshes * m_VertexSize;
		MaxIndices = MaxMeshes * m_IndexSize;

		glm::vec3 vertexPosition[VertexSize];
		glm::vec3 normal[VertexSize];
		glm::vec2 UV[VertexSize];

		const float PI = 3.14159265359f;
		for (unsigned int x = 0; x <= X_SEGMENTS; x++)
		{
			for (unsigned int y = 0; y <= Y_SEGMENTS; y++)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

				vertexPosition[y + x * (Y_SEGMENTS + 1)] = glm::vec3(xPos, yPos, zPos);
				normal[y + x * (Y_SEGMENTS + 1)] = glm::vec3(xPos, yPos, zPos);
				UV[y + x * (Y_SEGMENTS + 1)] = glm::vec2(xSegment, ySegment);
			}
		}

		bool oddRow = false;
		for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
		{
			// 一奇一偶为一个矩形，[0,0],[0,1],[1,1],[1,0]
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
				{
					m_Indices.push_back(y * (X_SEGMENTS + 1) + x);
					m_Indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					m_Indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					m_Indices.push_back(y * (X_SEGMENTS + 1) + x);
				}
			}
			oddRow = !oddRow;
		}

		for (uint32_t i = 0; i < m_VertexSize; i++)
		{
			MeshVertex vertex;
			vertex.Position = vertexPosition[i];
			vertex.TexCoords = UV[i];
			vertex.Normal = normal[i];
			Mesh::SetVertexBoneDataToDefault(vertex);
			m_Vertices.push_back(vertex);
		}


		SetupMesh();
	}

	void SphereMesh::Draw()
	{
		Renderer::DrawStripIndexed(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount());
	}



}
