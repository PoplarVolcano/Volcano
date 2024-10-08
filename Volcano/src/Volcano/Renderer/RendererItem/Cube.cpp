#include "volpch.h"
#include "Cube.h"
#include "Volcano/Renderer/Renderer.h"

namespace Volcano {

	Ref<CubeData> Cube::m_CubeData;

	void Cube::Init()
	{
		m_CubeData = std::make_shared<CubeData>();
		// Cube
		m_CubeData->vertexArray = VertexArray::Create();
		m_CubeData->vertexBuffer = VertexBuffer::Create(m_CubeData->MaxVertices * sizeof(CubeVertex));
		m_CubeData->vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float2, "a_TexCoords"    },
			{ ShaderDataType::Float3, "a_Normal"       },
			{ ShaderDataType::Float3, "a_Tangent"      },
			{ ShaderDataType::Float3, "a_Bitangent"    },
			{ ShaderDataType::Int,    "a_EntityID"     }
			});
		m_CubeData->vertexArray->AddVertexBuffer(m_CubeData->vertexBuffer);
		m_CubeData->vertexBufferBase = new CubeVertex[m_CubeData->MaxVertices];

		uint32_t* indicesBuffer = new uint32_t[m_CubeData->MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < m_CubeData->MaxIndices; i += m_CubeData->IndexSize)
		{
			for (uint32_t j = 0; j < 36; j++)
				indicesBuffer[i + j] = offset + j;
			offset += 36;
		}
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indicesBuffer, m_CubeData->MaxIndices);;
		m_CubeData->vertexArray->SetIndexBuffer(indexBuffer);
		delete[] indicesBuffer;	// cpu上传到gpu上了可以删除cpu的索引数据块了

		m_CubeData->whiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		m_CubeData->whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

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

		for(uint32_t i = 0; i < 36; i++)
		    m_CubeData->vertexPosition[i] = vertexPosition[i];
		for (uint32_t i = 0; i < 36; i++)
			m_CubeData->texCoords[i] = texCoords[i % 6];
		for (uint32_t i = 0; i < 36; i++)
			m_CubeData->normal[i] = normal[i / 6];
		for (uint32_t i = 0; i < 36; i++)
			m_CubeData->tangent[i] = tangent[i / 6];
		for (uint32_t i = 0; i < 36; i++)
			m_CubeData->bitangent[i] = bitangent[i / 6];

	}

	void Cube::DrawIndexed()
	{
		Renderer::DrawIndexed(m_CubeData->vertexArray, m_CubeData->indexCount);
	}


}