#pragma once

#include <glm/glm.hpp>
#include "VertexArray.h"

namespace Volcano {

	enum class RendererAPIType
	{
		None,
		OpenGL
	};

	class RendererAPI
	{
	public:
		static void Init();

		static void SetClearColor(const glm::vec4& color);
		static void Clear();

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		static void Clear(float r, float g, float b, float a);
		static void SetClearColor(float r, float g, float b, float a);

		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount, bool depthTest = true);

		static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount);

		static void SetLineWidth(float width);

		inline static RendererAPIType Current() { return s_CurrentRendererAPI; }
	private:
		static RendererAPIType s_CurrentRendererAPI;
	};
}