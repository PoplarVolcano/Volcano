#include "volpch.h"
#include "PlaneMesh.h"

namespace Volcano {


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



}
