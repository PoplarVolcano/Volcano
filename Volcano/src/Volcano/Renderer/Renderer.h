#pragma once

#include "Volcano/Core/Core.h"
#include "Volcano/Renderer/RendererAPI.h"
#include "Volcano/Renderer/VertexArray.h"
#include "Volcano/Renderer/Shader.h"

namespace Volcano {

	class VOL_API Renderer
	{
	public:
		typedef void(*RenderCommandFn)(void*);

		static void Init();
		static void Shutdown();

		static void OnWindowResize(uint32_t width, uint32_t height);

		// Commands
		static void Clear();
		static void Clear(float r, float g, float b, float a = 1.0f);
		static void SetClearColor(float r, float g, float b, float a);

		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount);
		static void DrawArrays(const Ref<VertexArray>& vertexArray, uint32_t count);
		static void DrawStripIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount);
		static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount);

		static void DrawIndexInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t amount);
		static void DrawStripIndexInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t amount);


		static void SetLineWidth(float width);

		static void SetDepthTest(bool depthTest);

		static const Scope<ShaderLibrary>& GetShaderLibrary();
	};
}