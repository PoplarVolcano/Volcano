#include "volpch.h"
#include "CylinderMesh.h"
#include "Volcano/Renderer/Renderer.h"

namespace Volcano {

	std::once_flag CylinderMesh::init_flag;
	Scope<CylinderMesh> CylinderMesh::m_instance;

	Scope<CylinderMesh>& CylinderMesh::GetInstance()
	{
		std::call_once(init_flag, []() { m_instance.reset(new CylinderMesh()); });
		return m_instance;
	}

	Ref<CylinderMesh> CylinderMesh::CloneRef()
	{
		return std::make_shared<CylinderMesh>(*GetInstance().get());
	}

	CylinderMesh::CylinderMesh()
	{

		// ��������
		const int sectorCount = 36;
		const float pierRadius = 0.5f; // ���뾶
		const float pierHeight = 2.0f; // ����
		// Բ�ܶ���
		const float PI = 3.14159265359f;
		float sectorStep = 2 * PI / sectorCount;
		float sectorAngle = 0.0f;

		m_VertexSize = (sectorCount + 1) * 6;
		m_IndexSize = m_VertexSize;
		MaxMeshes = 1;
		MaxVertices = MaxMeshes * m_VertexSize;
		MaxIndices = MaxMeshes * m_IndexSize;

		// ��ȡ�ϡ���Բ�ܵ�����
		glm::vec3 topPosition[sectorCount + 1];
		glm::vec3 topNormal[sectorCount + 1];
		glm::vec3 botPosition[sectorCount + 1];
		glm::vec3 botNormal[sectorCount + 1];

		std::vector<MeshVertex> vertices;

		for (int i = 0; i <= sectorCount; ++i)
		{
			glm::vec3 position;
			glm::vec3 normal;
			sectorAngle = i * sectorStep;
			position.x = pierRadius * cos(sectorAngle);
			position.y = 0.0f;
			// z��������Ϊ�ӽ���������� positionΪ˳ʱ�룬����ȡ����positionΪ��ʱ��
			position.z = -pierRadius * sin(sectorAngle);

			normal.x = cos(sectorAngle);
			normal.y = 0.0f;
			normal.z = -sin(sectorAngle);
			
			topPosition[i]   = position;
			topPosition[i].y = pierHeight / 2.0f;
			topNormal[i]     = normal;
			
			botPosition[i]   = position;
			botPosition[i].y = -pierHeight / 2.0f;
			botNormal[i]     = normal;
		}

		// ����Բ��
		for (int i = 0; i <= sectorCount; ++i)
		{
			MeshVertex meshVertex;
			meshVertex.Position = glm::vec3(0.0f, pierHeight / 2.0f, 0.0f);
			meshVertex.TexCoords = glm::vec2(0.5f, 0.5f);
			meshVertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
			meshVertex.Tangent = topNormal[0];
			meshVertex.Bitangent = glm::cross(meshVertex.Normal, meshVertex.Tangent);
			vertices.push_back(meshVertex);

			meshVertex.Position = topPosition[i];
			meshVertex.TexCoords = (glm::normalize(glm::vec2(topPosition[i].x, -topPosition[i].z)) + 1.0f) / 2.0f;
			vertices.push_back(meshVertex);
		}

		// Բ������
		for (int i = 0; i <= sectorCount; ++i)
		{
			MeshVertex meshVertex;

			meshVertex.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

			meshVertex.Position = topPosition[i];
			meshVertex.TexCoords = glm::vec2((float)(i) / (float)sectorCount, 1.0f);
			meshVertex.Normal = topNormal[i];
			meshVertex.Tangent = glm::cross(meshVertex.Bitangent, meshVertex.Normal);
			vertices.push_back(meshVertex);

			meshVertex.Position = botPosition[i];
			meshVertex.TexCoords = glm::vec2((float)(i) / (float)sectorCount, 0.0f);
			meshVertex.Normal = botNormal[i];
			meshVertex.Tangent = glm::cross(meshVertex.Bitangent, meshVertex.Normal);
			vertices.push_back(meshVertex);

		}

		// �ײ�Բ��
		for (int i = 0; i <= sectorCount; ++i)
		{
			MeshVertex meshVertex;
			meshVertex.Bitangent = glm::cross(meshVertex.Normal, meshVertex.Tangent);
			meshVertex.Normal    = glm::vec3(0.0f, -1.0f, 0.0f);
			meshVertex.Tangent   = topNormal[0];

			meshVertex.Position = botPosition[i];
			meshVertex.TexCoords = (glm::normalize(glm::vec2(topPosition[i].x, topPosition[i].z)) + 1.0f) / 2.0f;
			vertices.push_back(meshVertex);

			meshVertex.Position = glm::vec3(0.0f, -pierHeight / 2.0f, 0.0f);
			meshVertex.TexCoords = glm::vec2(0.5f, 0.5f);
			vertices.push_back(meshVertex);

		}

		m_Vertices = vertices;
		for (uint32_t i = 0; i < m_VertexSize; i++)
		{
			Mesh::SetVertexBoneDataToDefault(m_Vertices[i]);
		}

		for (uint32_t i = 0; i < m_IndexSize; i++)
			m_Indices.push_back(i);

		SetupMesh();
	}

	void CylinderMesh::Draw()
	{
		Renderer::DrawStripIndexed(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount());
	}

	void CylinderMesh::DrawCylinder(glm::mat4 transform)
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
