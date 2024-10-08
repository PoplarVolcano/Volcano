#include "volpch.h"
#include "CubeMesh.h"

namespace Volcano {


	CubeMesh::CubeMesh()
    {
		m_IndexSize = 36;
		MaxMeshes   = 1;
		MaxVertices = MaxMeshes * 36;
		MaxIndices  = MaxMeshes * m_IndexSize;
		
		// ±³ Õý ×ó ÓÒ ÏÂ ÉÏ
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

        for (uint32_t i = 0; i < 36; i++)
        {
            MeshTempVertex vertex;
            vertex.Position  = vertexPosition[i];
            vertex.TexCoords = texCoords[i % 6];
            vertex.Normal    = normal[i / 6];
            vertex.Tangent   = tangent[i / 6];
            vertex.Bitangent = bitangent[i / 6];
            m_Vertices.push_back(vertex);
        }

		for (uint32_t i = 0; i < m_IndexSize; i++)
			m_Indices.push_back(i);

		SetupMesh();
    }



}
