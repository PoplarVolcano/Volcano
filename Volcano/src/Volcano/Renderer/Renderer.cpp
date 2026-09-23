#include "volpch.h"
#include "Renderer.h"
#include "Volcano/Renderer/Renderer2D.h"
#include "Volcano/Renderer/UniformBuffer.h"
#include "Volcano/Renderer/RendererItem/Mesh.h"
#include "Volcano/Renderer/RendererItem/FullQuad.h"
#include "Volcano/Renderer/SceneRenderer.h"

namespace Volcano {

	RendererAPIType RendererAPI::s_CurrentRendererAPI = RendererAPIType::OpenGL;

	static Scope<ShaderLibrary> s_ShaderLibrary;

	void Renderer::Init()
	{
		s_ShaderLibrary = std::make_unique<ShaderLibrary>();

		Renderer::GetShaderLibrary()->Load("Resources/shaders/Renderer2D_Circle.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/Renderer2D_Line.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/lighting/DirectionalLightShadowDepth.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/lighting/PointLightShadowDepth.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/lighting/SpotLightShadowDepth.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/GBuffer.glsl", ShaderBackend::OpenGL);
		Renderer::GetShaderLibrary()->Load("Resources/shaders/DeferredShading.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/lighting/LightShading.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/lighting/DirectionalLightShading.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/lighting/PointLightShading.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/lighting/SpotLightShading.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/lighting/PBRLightShading.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/lighting/PBRDirectionalLightShading.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/lighting/PBRPointLightShading.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/lighting/PBRSpotLightShading.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/Particle.glsl", ShaderBackend::OpenGL);
		Renderer::GetShaderLibrary()->Load("Resources/shaders/Outline.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/DebugDrawQuad.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/PostProcessing.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/NormalVisualization.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/Skybox.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/HDRAndBloom.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/GaussianBlur.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/ForwardShading.glsl", ShaderBackend::OpenGL);
		Renderer::GetShaderLibrary()->Load("Resources/shaders/EntityID.glsl", ShaderBackend::OpenGL);
		Renderer::GetShaderLibrary()->Load("Resources/shaders/LightingMode.glsl", ShaderBackend::OpenGL);
		Renderer::GetShaderLibrary()->Load("Resources/shaders/SSAO.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/SSAOBlur.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/EquirectangularToEnvCubeMap.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/EnvCubeMapToIrradianceMap.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/EnvCubeMapToPrefilterMap.glsl");
		Renderer::GetShaderLibrary()->Load("Resources/shaders/BRDF.glsl");


		// 初始化OpenGL配置
		RendererAPI::Init();

		Mesh::Init();
		FullQuad::Init();
		Renderer2D::Init();

		UniformBufferManager::Init();

		SceneRenderer::Init();

	}

	void Renderer::Shutdown()
	{
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

	void Renderer::DrawIndexInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t amount)
	{
		RendererAPI::DrawIndexInstanced(vertexArray, indexCount, amount);
	}

	void Renderer::DrawStripIndexInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t amount)
	{
		RendererAPI::DrawStripIndexInstanced(vertexArray, indexCount, amount);
	}

	void Renderer::SetLineWidth(float width)
	{
		RendererAPI::SetLineWidth(width);
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
		RendererAPI::SetDepthTestEnable(depthTest);
	}

	const Scope<ShaderLibrary>& Renderer::GetShaderLibrary()
	{
		return s_ShaderLibrary;
	}

}