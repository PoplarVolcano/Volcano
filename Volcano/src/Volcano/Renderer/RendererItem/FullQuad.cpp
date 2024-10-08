#include "volpch.h"
#include "FullQuad.h"

#include "glm/glm.hpp"
#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Renderer/Buffer.h"
#include "Volcano/Renderer/Shader.h"
#include "Volcano/Renderer/Texture.h"

namespace Volcano {

	Ref<VertexArray> FullQuad::m_VertexArray;

	void FullQuad::Init()
	{
		float vertices[] = {
			-1.0, -1.0, 0.0f, 0.0f, 0.0f,
			 1.0, -1.0, 0.0f, 1.0f, 0.0f,
			 1.0,  1.0, 0.0f, 1.0f, 1.0f,
			-1.0,  1.0, 0.0f, 0.0f, 1.0f
		};

		m_VertexArray = VertexArray::Create();
		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float2, "a_TexCoords"    }
			});
		m_VertexArray->AddVertexBuffer(vertexBuffer);
		uint32_t indices[] = { 0, 1, 2, 2, 3 ,0 };
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));;
		m_VertexArray->SetIndexBuffer(indexBuffer);
	}

	void FullQuad::DrawIndexed()
	{
		Renderer::DrawIndexed(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount());
	}

}