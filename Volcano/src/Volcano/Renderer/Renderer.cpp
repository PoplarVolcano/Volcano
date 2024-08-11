#include "volpch.h"
#include "Renderer.h"
#include "Renderer2D.h"


namespace Volcano {

	RendererAPIType RendererAPI::s_CurrentRendererAPI = RendererAPIType::OpenGL;

	struct RendererData
	{
		RenderCommandQueue m_CommandQueue;;
		Scope<ShaderLibrary> m_ShaderLibrary;
	};
	static RendererData s_Data;

	void Renderer::Init()
	{
		s_Data.m_ShaderLibrary = std::make_unique<ShaderLibrary>();
		Renderer::Submit([]() { RendererAPI::Init(); });

		//Renderer::GetShaderLibrary()->Load("assets/shaders/Square.glsl");
		Renderer::GetShaderLibrary()->Load("assets/shaders/Texture.glsl");

		Renderer2D::Init();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		Renderer::Submit([=]() { RendererAPI::SetViewport(0, 0, width, height); });
	}

	void Renderer::DrawIndexed(PrimitiveType type, uint32_t count, bool depthTest)
	{
		Renderer::Submit([=]() {
			RendererAPI::DrawIndexed(type, count, depthTest);
			});
	}

	const Scope<ShaderLibrary>& Renderer::GetShaderLibrary()
	{
		return s_Data.m_ShaderLibrary;
	}

	RenderCommandQueue& Renderer::GetRenderCommandQueue()
	{
		return s_Data.m_CommandQueue;
	}

	void Renderer::WaitAndRender()
	{
		s_Data.m_CommandQueue.Execute();
	}

	void Renderer::Clear()
	{
		Renderer::Submit([]() {
			RendererAPI::Clear();
			});
	}

	void Renderer::Clear(float r, float g, float b, float a)
	{
		Renderer::Submit([=]() {
			RendererAPI::Clear(r, g, b, a);
			});
	}

	void Renderer::SetClearColor(float r, float g, float b, float a)
	{
		Renderer::Submit([=]() {
			RendererAPI::SetClearColor(r, g, b, a);
			});
	}
}