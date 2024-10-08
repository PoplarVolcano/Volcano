#include "volpch.h"
#include "Quad.h"

#include "glm/glm.hpp"
#include "Volcano/Renderer/Renderer.h"

namespace Volcano {

	Ref<QuadData> Quad::m_QuadData;

	void Quad::Init()
	{
		m_QuadData = std::make_shared<QuadData>();
		// Quad
		m_QuadData->vertexArray = VertexArray::Create();
		m_QuadData->vertexBuffer = VertexBuffer::Create(m_QuadData->MaxVertices * sizeof(QuadVertex));
		m_QuadData->vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float4, "a_Color"        },
			{ ShaderDataType::Float2, "a_TexCoords"     },
			{ ShaderDataType::Float,  "a_TextureIndex" },
			{ ShaderDataType::Float,  "a_TilingFactor" },
			{ ShaderDataType::Int,    "a_EntityID"     }
			});
		m_QuadData->vertexArray->AddVertexBuffer(m_QuadData->vertexBuffer);
		m_QuadData->vertexBufferBase = new QuadVertex[m_QuadData->MaxVertices];
		uint32_t* quadIndices = new uint32_t[m_QuadData->MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < m_QuadData->MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}
		Ref<IndexBuffer> quadIndexBuffer = IndexBuffer::Create(quadIndices, m_QuadData->MaxIndices);;
		m_QuadData->vertexArray->SetIndexBuffer(quadIndexBuffer);
		delete[] quadIndices;	// cpu上传到gpu上了可以删除cpu的索引数据块了

		m_QuadData->vertexPosition[0] = { -0.5, -0.5, 0.0f, 1.0f };
		m_QuadData->vertexPosition[1] = {  0.5, -0.5, 0.0f, 1.0f };
		m_QuadData->vertexPosition[2] = {  0.5,  0.5, 0.0f, 1.0f };
		m_QuadData->vertexPosition[3] = { -0.5,  0.5, 0.0f, 1.0f };

		Renderer::GetShaderLibrary()->Load("assets/shaders/Renderer2D_Quad.glsl");
		m_QuadData->shader = Renderer::GetShaderLibrary()->Get("Renderer2D_Quad");
	}

	void Quad::DrawIndexed()
	{
		Renderer::DrawIndexed(m_QuadData->vertexArray, m_QuadData->indexCount);
	}

}