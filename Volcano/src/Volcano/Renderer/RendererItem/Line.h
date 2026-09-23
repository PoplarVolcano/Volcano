#pragma once

#include "Volcano/Renderer/VertexArray.h"
#include "Volcano/Renderer/Buffer.h"
#include "Volcano/Renderer/Shader.h"
#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	struct LineVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;

		// Editor-only
		int EntityID;
	};

	class Line : public Mesh
	{
	public:
		Line();
		~Line();

		void StartBatch();
		void NextBatch();
		void Flush();

		void SetLineWidth();
		virtual void FlushInstances() override;
	private:

		static const uint32_t MaxLines = 20000;
		static const uint32_t MaxVertices = MaxLines * 2;

		Ref<VertexArray> vertexArray;
		Ref<VertexBuffer> vertexBuffer;
		Ref<Shader> shader;

		uint32_t vertexCount = 0;  // Line只需要提供顶点数量
		LineVertex* vertexBufferBase = nullptr;
		LineVertex* vertexBufferPtr = nullptr;
		float width = 2.0f;

		friend class Renderer2D;
	};
}