#include "volpch.h"
#include "Circle.h"

#include "Volcano/Renderer/Renderer.h"

namespace Volcano {

	Circle::Circle()
	{
		vertexArray = VertexArray::Create();
		vertexBuffer = VertexBuffer::Create(MaxVertices * sizeof(CircleVertex));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_WorldPosition"  },
			{ ShaderDataType::Float2, "a_LocalPosition"  },
			{ ShaderDataType::Float4, "a_Color"     },
			{ ShaderDataType::Float,  "a_Thickness" },
			{ ShaderDataType::Float,  "a_Fade"      },
			{ ShaderDataType::Int,    "a_EntityID"  }
			});
		vertexArray->AddVertexBuffer(vertexBuffer);
		vertexBufferBase = new CircleVertex[MaxVertices];

		uint32_t* indices = new uint32_t[MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < MaxIndices; i += 6)
		{
			indices[i + 0] = offset + 0;
			indices[i + 1] = offset + 1;
			indices[i + 2] = offset + 2;

			indices[i + 3] = offset + 2;
			indices[i + 4] = offset + 3;
			indices[i + 5] = offset + 0;

			offset += 4;
		}
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, MaxIndices);;
		vertexArray->SetIndexBuffer(indexBuffer);
		delete[] indices;
		
		vertexPosition[0] = { -0.5, -0.5, 0.0f, 1.0f };
		vertexPosition[1] = {  0.5, -0.5, 0.0f, 1.0f };
		vertexPosition[2] = {  0.5,  0.5, 0.0f, 1.0f };
		vertexPosition[3] = { -0.5,  0.5, 0.0f, 1.0f };

		shader = Renderer::GetShaderLibrary()->Get("Renderer2D_Circle");
	}

	Circle::~Circle()
	{
		delete[] vertexBufferBase;
	}

	void Circle::StartBatch()
	{
		indexCount = 0;
		vertexBufferPtr = vertexBufferBase;

	}

	void Circle::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Circle::Flush()
	{
		if (indexCount)
		{
			// 不加(uint8_t*)会得到元素数量, 加uint8_t返回以char为单位占用多少元素
			uint32_t dataSize = (uint32_t)((uint8_t*)vertexBufferPtr - (uint8_t*)vertexBufferBase);
			vertexBuffer->SetData(vertexBufferBase, dataSize);

			shader->Bind();
			FlushInstances();
		}

	}
	
	void Circle::FlushInstances()
	{
		Renderer::DrawIndexed(vertexArray, indexCount);
	}

}