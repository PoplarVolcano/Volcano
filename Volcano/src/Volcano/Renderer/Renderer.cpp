#include "volpch.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "RendererModel.h"
#include "RendererItem/Skybox.h"
#include "RendererItem/Shadow.h"
#include "Volcano/Renderer/RendererItem/Sphere.h"
#include "Volcano/Renderer/UniformBuffer.h"


namespace Volcano {

	RendererAPIType RendererAPI::s_CurrentRendererAPI = RendererAPIType::OpenGL;

	static Ref<RenderPass> s_ActiveRenderPass;
	static Scope<ShaderLibrary> s_ShaderLibrary;

	void Renderer::Init()
	{
		s_ShaderLibrary = std::make_unique<ShaderLibrary>();

		// 初始化OpenGL配置
		RendererAPI::Init(); 

		// 初始化2D Shader
		Renderer2D::Init();
		RendererModel::Init();
		Skybox::Init();
		Shadow::Init();
		Sphere::Init();


		Renderer::GetShaderLibrary()->Load("assets/shaders/GBuffer.glsl");
		Renderer::GetShaderLibrary()->Load("assets/shaders/shadow/LightShading.glsl");
		Renderer::GetShaderLibrary()->Load("assets/shaders/DeferredShading.glsl");
		Renderer::GetShaderLibrary()->Load("assets/shaders/SSAO.glsl");
		Renderer::GetShaderLibrary()->Load("assets/shaders/SSAOBlur.glsl");
		Renderer::GetShaderLibrary()->Load("assets/shaders/3D/EquirectangularToCubemap.glsl");
		Renderer::GetShaderLibrary()->Load("assets/shaders/3D/IrradianceConvolution.glsl");
		Renderer::GetShaderLibrary()->Load("assets/shaders/3D/Prefilter.glsl");
		Renderer::GetShaderLibrary()->Load("assets/shaders/3D/BRDF.glsl");

		UniformBufferManager::Init();
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

	void Renderer::DrawArrays(const Ref<VertexArray>& vertexArray, uint32_t count)
	{
		RendererAPI::DrawArrays(vertexArray, count);
	}

	void Renderer::DrawStripIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		RendererAPI::DrawStripIndexed(vertexArray, indexCount);
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
	void Renderer::SetDepthTest(bool depthTest)
	{
		RendererAPI::SetDepthTest(depthTest);
	}


	void Renderer::BeginRenderPass(const Ref<RenderPass>& renderPass, bool clear)
	{
		VOL_CORE_ASSERT(renderPass, "Render pass cannot be null!");

		// TODO: Convert all of this into a render command buffer
		s_ActiveRenderPass = renderPass;

		renderPass->GetSpecification().TargetFramebuffer->Bind();
		if (clear)
		{
			const glm::vec4& clearColor = renderPass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColor;
			RendererAPI::Clear(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		}
	}

	void Renderer::EndRenderPass()
	{
		VOL_CORE_ASSERT(s_ActiveRenderPass, "No active render pass! Have you called Renderer::EndRenderPass twice?");
		s_ActiveRenderPass->GetSpecification().TargetFramebuffer->Unbind();
		s_ActiveRenderPass = nullptr;
	}
}