#include "volpch.h"

#include "PlaneMesh.h"
#include "Volcano/Renderer/Renderer.h"

namespace Volcano {


	std::once_flag PlaneMesh::init_flag;
	Scope<PlaneMesh> PlaneMesh::m_instance;

	Scope<PlaneMesh>& PlaneMesh::GetInstance()
	{
		std::call_once(init_flag, []() { m_instance.reset(new PlaneMesh()); });
		return m_instance;
	}

	Ref<PlaneMesh> PlaneMesh::CloneRef()
	{
		return std::make_shared<PlaneMesh>(*GetInstance().get());
	}


	PlaneMesh::PlaneMesh()
	{
		m_VertexSize = 4;
		m_IndexSize = 6;
		MaxMeshes = 1;
		MaxVertices = MaxMeshes * m_VertexSize;
		MaxIndices = MaxMeshes * m_IndexSize;

		glm::vec3 vertexPosition[] =
		{
			{ -5.0f,  0.0f,  5.0f },
			{  5.0f,  0.0f,  5.0f },
			{  5.0f,  0.0f, -5.0f },
			{ -5.0f,  0.0f, -5.0f },
		};

		glm::vec2 texCoords[] =
		{
			{ 0.0f, 0.0f},
			{ 1.0f, 0.0f},
			{ 1.0f, 1.0f},
			{ 0.0f, 1.0f}
		};

		glm::vec3 normal(0.0f,  1.0f,  0.0f);

		glm::vec3 tangent(1.0f,  0.0f,  0.0f);

		glm::vec3 bitangent(0.0f, 0.0f, -1.0f);


		for (uint32_t i = 0; i < m_VertexSize; i++)
		{
			MeshVertex vertex;
			vertex.Position = vertexPosition[i];
			vertex.TexCoords = texCoords[i];
			vertex.Normal = normal;
			vertex.Tangent = tangent;
			vertex.Bitangent = bitangent;
			Mesh::SetVertexBoneDataToDefault(vertex);
			m_Vertices.push_back(vertex);
		}

		m_Indices = { 0, 1, 2, 2, 3, 0 };

		SetupMesh();
	}


	void PlaneMesh::DrawPlane(glm::mat4 transform)
	{
		if (m_IndexCount)
		{
			// 不加uint8_t转化会得到元素数量, 加uint8_t返回以char为单位占用多少元素
			uint32_t dataSize = (uint32_t)((uint8_t*)vertexBufferPtr - (uint8_t*)vertexBufferBase);
			m_VertexBuffer->SetData(vertexBufferBase, dataSize);

			UniformBufferManager::GetUniformBuffer("ModelTransform")->SetData(&transform, sizeof(glm::mat4));
		}

		Renderer::DrawIndexed(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount());
	}



}
