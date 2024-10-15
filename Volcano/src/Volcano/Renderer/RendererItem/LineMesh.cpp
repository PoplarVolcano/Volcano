#include "volpch.h"
#include "LineMesh.h"

namespace Volcano {


	LineMesh::LineMesh()
    {
		m_VertexSize = 2;
		m_IndexSize = 2;
		MaxMeshes   = 2000;
		MaxVertices = MaxMeshes * m_VertexSize;
		MaxIndices  = MaxMeshes * m_IndexSize;
		
		// ±³ Õý ×ó ÓÒ ÏÂ ÉÏ
		glm::vec3 vertexPosition[] =
		{
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f }

		};

        for (uint32_t i = 0; i < m_VertexSize; i++)
        {
            MeshVertex vertex;
            vertex.Position  = vertexPosition[i];
			Mesh::SetVertexBoneDataToDefault(vertex);
            m_Vertices.push_back(vertex);
        }

		for (uint32_t i = 0; i < m_IndexSize; i++)
			m_Indices.push_back(i);

		SetupMesh();
    }



}
