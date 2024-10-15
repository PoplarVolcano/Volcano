#include "volpch.h"
#include "ModelMesh.h"

namespace Volcano {

	ModelMesh::ModelMesh(std::vector<MeshVertex> vertices, std::vector<uint32_t> indices)
	{
		m_VertexSize = vertices.size();
		m_IndexSize = indices.size();
		MaxMeshes = 1;
		MaxVertices = MaxMeshes * m_VertexSize;
		MaxIndices = MaxMeshes * m_IndexSize;
		m_Vertices = vertices;
		m_Indices = indices;
		SetupMesh();
	}

}
