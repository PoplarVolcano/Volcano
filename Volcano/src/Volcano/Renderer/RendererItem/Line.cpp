#include "volpch.h"
#include "Line.h"

#include "Volcano/Renderer/Renderer.h"

namespace Volcano {

	Line::Line()
	{
		vertexArray = VertexArray::Create();
		vertexBuffer = VertexBuffer::Create(MaxVertices * sizeof(LineVertex));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"  },
			{ ShaderDataType::Float4, "a_Color"     },
			{ ShaderDataType::Int,    "a_EntityID"  }
			});
		vertexArray->AddVertexBuffer(vertexBuffer);
		vertexBufferBase = new LineVertex[MaxVertices];

		shader = Renderer::GetShaderLibrary()->Get("Renderer2D_Line");
	}

	Line::~Line()
	{
		delete[] vertexBufferBase;
	}

	void Line::StartBatch()
	{
		vertexCount = 0;
		vertexBufferPtr = vertexBufferBase;
	}

	void Line::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Line::Flush()
	{
		if (vertexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)vertexBufferPtr - (uint8_t*)vertexBufferBase);
			vertexBuffer->SetData(vertexBufferBase, dataSize);

			shader->Bind();
			SetLineWidth();
			FlushInstances();
		}
	}

	void Line::SetLineWidth()
	{
		Renderer::SetLineWidth(width);
	}

	void Line::FlushInstances()
	{
		Renderer::DrawLines(vertexArray, vertexCount);
	}

}