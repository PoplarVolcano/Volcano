#pragma once
#include <memory>
#include "Volcano/Renderer/Buffer.h"

namespace Volcano {

	class VertexArray {
	public:
		virtual~VertexArray() {}

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;

		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers()const = 0;
		virtual const Ref<IndexBuffer>& GetIndexBuffer()const = 0;

		virtual uint32_t GetRendererID() const = 0;

		static Ref<VertexArray> Create();
	};
}

