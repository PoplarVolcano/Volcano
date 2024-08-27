#include "volpch.h"
#include "Renderer.h"
#include "Renderer2D.h"


namespace Volcano {

	RendererAPIType RendererAPI::s_CurrentRendererAPI = RendererAPIType::OpenGL;

	static Scope<ShaderLibrary> s_ShaderLibrary;

	void Renderer::Init()
	{
		s_ShaderLibrary = std::make_unique<ShaderLibrary>();
		RendererAPI::Init(); 

		Renderer2D::Init();
	}

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RendererAPI::SetViewport(0, 0, width, height); 
	}

	void Renderer::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		RendererAPI::DrawIndexed(vertexArray, indexCount);
	}

	void Renderer::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		RendererAPI::DrawLines(vertexArray, vertexCount);
	}

	void Renderer::SetLineWidth(float width)
	{
		RendererAPI::SetLineWidth(width);
	}

	const Scope<ShaderLibrary>& Renderer::GetShaderLibrary()
	{
		return s_ShaderLibrary;
	}

	void Renderer::Clear()
	{
		RendererAPI::Clear();
	}

	void Renderer::Clear(float r, float g, float b, float a)
	{
		RendererAPI::Clear(r, g, b, a);
	}

	void Renderer::SetClearColor(float r, float g, float b, float a)
	{
		RendererAPI::SetClearColor(r, g, b, a);
	}
}