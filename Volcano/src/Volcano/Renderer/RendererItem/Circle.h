#pragma once

#include "Volcano/Renderer/VertexArray.h"
#include "Volcano/Renderer/Buffer.h"
#include "Volcano/Renderer/Shader.h"
#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	struct CircleVertex
	{
		glm::vec3 WorldPosition;
		glm::vec2 LocalPosition; // 用于配合Circle的Shader的算法
		glm::vec4 Color;
		float Thickness;
		float Fade;

		// Editor-only
		int EntityID;
	};

	class Circle : public Mesh
	{
	public:
		Circle();
		~Circle();

		void StartBatch();
		void NextBatch();
		void Flush();

		virtual void FlushInstances() override;
	private:

		static const uint32_t MaxCircles = 20000;
		static const uint32_t MaxVertices = MaxCircles * 4;
		static const uint32_t MaxIndices = MaxCircles * 6;

		Ref<VertexArray> vertexArray;
		Ref<VertexBuffer> vertexBuffer;
		Ref<Shader> shader;

		uint32_t indexCount = 0;
		CircleVertex* vertexBufferBase = nullptr;
		CircleVertex* vertexBufferPtr = nullptr;

		glm::vec4 vertexPosition[4];

		friend class Renderer2D;
	};
}