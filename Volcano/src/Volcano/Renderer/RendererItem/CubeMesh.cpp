#include "volpch.h"
#include "CubeMesh.h"
#include "Volcano/Renderer/Renderer.h"

namespace Volcano {


	std::once_flag CubeMesh::init_flag;
	Scope<CubeMesh> CubeMesh::m_instance;

	Scope<CubeMesh>& CubeMesh::GetInstance()
	{
		std::call_once(init_flag, []() { m_instance.reset(new CubeMesh()); });
		return m_instance;
	}

	Ref<CubeMesh> CubeMesh::CloneRef()
	{
		return std::make_shared<CubeMesh>(*GetInstance().get());
	}


	CubeMesh::CubeMesh()
    {
		m_VertexSize = 36;
		m_IndexSize = 36;
		MaxMeshes   = 1;
		MaxVertices = MaxMeshes * m_VertexSize;
		MaxIndices  = MaxMeshes * m_IndexSize;
		
		// 背 正 左 右 下 上
		glm::vec3 vertexPosition[] =
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

		glm::vec2 texCoords[] =
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

        for (uint32_t i = 0; i < m_VertexSize; i++)
        {
            MeshVertex vertex;
            vertex.Position  = vertexPosition[i];
            vertex.TexCoords = texCoords[i % 6];
            vertex.Normal    = normal[i / 6];
            vertex.Tangent   = tangent[i / 6];
            vertex.Bitangent = bitangent[i / 6];
			Mesh::SetVertexBoneDataToDefault(vertex);
            m_Vertices.push_back(vertex);
        }

		for (uint32_t i = 0; i < m_IndexSize; i++)
			m_Indices.push_back(i);

		SetupMesh();
    }

	void CubeMesh::DrawCube(glm::mat4 transform)
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
