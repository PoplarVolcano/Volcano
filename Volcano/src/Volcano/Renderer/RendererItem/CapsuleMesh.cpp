#include "volpch.h"
#include "CapsuleMesh.h"
#include "Volcano/Renderer/Renderer.h"

namespace Volcano {

	std::once_flag CapsuleMesh::init_flag;
	Scope<CapsuleMesh> CapsuleMesh::m_instance;

	Scope<CapsuleMesh>& CapsuleMesh::GetInstance()
	{
		std::call_once(init_flag, []() { m_instance.reset(new CapsuleMesh()); });
		return m_instance;
	}

	Ref<CapsuleMesh> CapsuleMesh::CloneRef()
	{
		return std::make_shared<CapsuleMesh>(*GetInstance().get());
	}

	CapsuleMesh::CapsuleMesh()
	{
		const float radius = 0.5f; // ��뾶
		const float height = 1.0f; // ����
		// ��ΪGL_TRIANGLE_STRIP�����ԣ��½ǵ�ʵ��Ƭ����(X_SEGMENTS + 1)����ǵ�ʵ��Ƭ����(Y_SEGMENTS + 2)���ܶ��������Ⱦ������Ӱ��
		const uint32_t X_SEGMENTS = 65; // �½�
		const uint32_t Y_SEGMENTS = 64; // ����
		const uint32_t VertexSize = (X_SEGMENTS + 1) * (Y_SEGMENTS + 2);
		const uint32_t IndexSize = (X_SEGMENTS + 1) * (Y_SEGMENTS + 1) * 2;

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
			// �ϰ���
			for (unsigned int y = 0; y <= Y_SEGMENTS / 2; y++)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI) * radius;
				float yPos = std::cos(ySegment * PI) * radius + height / 2.0f;
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI) * radius;

				vertexPosition[y + x * (Y_SEGMENTS + 2)] = glm::vec3(xPos, yPos, zPos);
				normal[y + x * (Y_SEGMENTS + 2)] = glm::vec3(xPos, yPos, zPos);
				UV[y + x * (Y_SEGMENTS + 2)] = glm::vec2(xSegment, ySegment);
			}
			// �°���
			for (unsigned int y = Y_SEGMENTS / 2; y <= Y_SEGMENTS; y++)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI) * radius;
				float yPos = std::cos(ySegment * PI) * radius - height / 2.0f;
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI) * radius;

				vertexPosition[y + 1 + x * (Y_SEGMENTS + 2)] = glm::vec3(xPos, yPos, zPos);
				normal[y + 1 + x * (Y_SEGMENTS + 2)] = glm::vec3(xPos, yPos, zPos);
				UV[y + 1 + x * (Y_SEGMENTS + 2)] = glm::vec2(xSegment, ySegment);
			}
		}

		bool oddRow = false;
		for (unsigned int y = 0; y <= Y_SEGMENTS / 2; ++y)
		{
			// һ��һżΪһ�����Σ�[0,0],[0,1],[1,1],[1,0]
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

		for (unsigned int y = Y_SEGMENTS / 2; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
				{
					m_Indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					m_Indices.push_back((y + 2) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					m_Indices.push_back((y + 2) * (X_SEGMENTS + 1) + x);
					m_Indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
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

	void CapsuleMesh::Draw()
	{
		//RendererAPI::SetPolygonMode(true);
		Renderer::DrawStripIndexed(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount());
		//RendererAPI::SetPolygonMode(false);
	}

	void CapsuleMesh::DrawCapsule(glm::mat4 transform)
	{
		if (m_IndexCount)
		{
			// ����uint8_tת����õ�Ԫ������, ��uint8_t������charΪ��λռ�ö���Ԫ��
			uint32_t dataSize = (uint32_t)((uint8_t*)vertexBufferPtr - (uint8_t*)vertexBufferBase);
			m_VertexBuffer->SetData(vertexBufferBase, dataSize);

			UniformBufferManager::GetUniformBuffer("ModelTransform")->SetData(&transform, sizeof(glm::mat4));
		}

		Renderer::DrawIndexed(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount());
	}


}
