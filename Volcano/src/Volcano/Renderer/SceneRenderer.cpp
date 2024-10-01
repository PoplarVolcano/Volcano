#include "volpch.h"
#include "SceneRenderer.h"
#include "Volcano/Renderer/Shader.h"
#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	struct SceneRendererData
	{
		const Scene* ActiveScene = nullptr;
		struct SceneInfo
		{
			Camera SceneCamera;

		} SceneData;

		Ref<RenderPass> GeoPass;
		Ref<RenderPass> CompositePass;//组合渲染管道

		struct DrawCommand
		{
			Ref<Mesh> Mesh;
			glm::mat4 Transform;
		};
		std::vector<DrawCommand> DrawList;

		SceneRendererOptions Options;
	};

	static SceneRendererData s_SceneRendererData;


	void SceneRenderer::Init()
	{
		FramebufferSpecification geoFramebufferSpec;
		geoFramebufferSpec.Width = 1280;
		geoFramebufferSpec.Height = 720;
		geoFramebufferSpec.Attachments = { FramebufferTextureFormat::RGBA16F };
		geoFramebufferSpec.Samples = 8;
		geoFramebufferSpec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };

		RenderPassSpecification geoRenderPassSpec;
		geoRenderPassSpec.TargetFramebuffer = Framebuffer::Create(geoFramebufferSpec);

		FramebufferSpecification compFramebufferSpec;
		compFramebufferSpec.Width = 1280;
		compFramebufferSpec.Height = 720;
		compFramebufferSpec.Attachments = { FramebufferTextureFormat::RGBA8 };
		compFramebufferSpec.ClearColor = { 0.5f, 0.1f, 0.1f, 1.0f };

		RenderPassSpecification compRenderPassSpec;
		compRenderPassSpec.TargetFramebuffer = Framebuffer::Create(compFramebufferSpec);
		s_SceneRendererData.CompositePass = RenderPass::Create(compRenderPassSpec);

	}

	void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		s_SceneRendererData.GeoPass->GetSpecification().TargetFramebuffer->Resize(width, height);
		s_SceneRendererData.CompositePass->GetSpecification().TargetFramebuffer->Resize(width, height);
	}

	void SceneRenderer::BeginScene(const Scene* scene)
	{
		VOL_CORE_ASSERT(!s_SceneRendererData.ActiveScene, "");

		s_SceneRendererData.ActiveScene = scene;

		//s_SceneRendererData.SceneData.SceneCamera = scene->m_Camera;
		//s_SceneRendererData.SceneData.SkyboxMaterial = scene->m_SkyboxMaterial;
		//s_SceneRendererData.SceneData.SceneEnvironment = scene->m_Environment;
		//s_SceneRendererData.SceneData.ActiveLight = scene->m_Light;
	}

	void SceneRenderer::EndScene()
	{
		VOL_CORE_ASSERT(s_SceneRendererData.ActiveScene, "");

		s_SceneRendererData.ActiveScene = nullptr;

		FlushDrawList();
	}

	void SceneRenderer::SubmitEntity(Entity* entity)
	{
		// TODO: Culling, sorting, etc.

		auto mesh = entity->GetMesh();
		if (!mesh)
			return;

		s_SceneRendererData.DrawList.push_back({ mesh, entity->GetTransform() });
	}

}