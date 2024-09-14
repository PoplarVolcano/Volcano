#pragma once

#include <glm/glm.hpp>
#include "VertexArray.h"

namespace Volcano {

	enum class DepthFunc
	{
		NEVER,
		LESS,
		EQUAL,
		LEQUAL,
		GREATER,
		NOTEQUAL,
		GEQUAL,
		ALWAYS
	};

	enum class CullFaceFunc
	{
		BACK,
		FRONT,
		FRONT_AND_BACK
	};

	enum class RendererAPIType
	{
		None,
		OpenGL
	};

	struct RenderAPICapabilities
	{
		std::string Vendor;
		std::string Renderer;
		std::string Version;

		int MaxSamples = 0;
		float MaxAnisotropy = 0.0f;
		int MaxTextureUnits = 0;
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
		static void DrawInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t amount, bool depthTest = true);

		static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount);

		static void SetLineWidth(float width);

		static void SetDepthTest(bool depthTest);

		static void SetDepthFunc(DepthFunc func);
		static void SetCullFaceFunc(CullFaceFunc func);

		static RenderAPICapabilities& GetCapabilities()
		{
			static RenderAPICapabilities capabilities;
			return capabilities;
		}

		inline static RendererAPIType Current() { return s_CurrentRendererAPI; }
	private:
		static RendererAPIType s_CurrentRendererAPI;
	};
}