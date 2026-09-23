#include "volpch.h"
#include "SceneRenderer.h"

#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Renderer/RendererItem/FullQuad.h"
#include "Volcano/Renderer/UniformBuffer.h"
#include "Volcano/Scene/Entity.h"
#include "Volcano/Renderer/RendererItem/Skybox.h"
#include "Volcano/Renderer/PostProcessing.h"
#include "Volcano/Renderer/Material.h"
#include "Volcano/Renderer/Texture.h"

#include "Volcano/Scene/SceneBuffer.h"
#include "Volcano/Utils/PlatformUtils.h"

namespace Volcano {


	Ref<Scene> SceneRenderer::m_ActiveScene;
	SceneState SceneRenderer::m_SceneState;
	EditorCamera SceneRenderer::m_EditorCamera;

	void SceneRenderer::BeginScene(Ref<Scene> scene, SceneState sceneState, EditorCamera& editorCamera)
	{
		m_ActiveScene = scene;
		m_SceneState = sceneState;
		m_EditorCamera = editorCamera;


		auto& materialHandles = m_ActiveScene->GetMaterialLibrary()->GetMaterialHandles();
		UniformBufferManager::GetUniformBuffer("MaterialHandle")->SetData(materialHandles.data(), materialHandles.size() * sizeof(MaterialHandle));

	}

	void SceneRenderer::EndScene()
	{
	}

	void SceneRenderer::PreProcessing()
	{
		EntityID();
		LightingMode();
	}

	void SceneRenderer::EntityID()
	{
		m_EntityIDFramebuffer->Bind();
		{
			RendererAPI::SetStencilMask(0xFF);
			Renderer::Clear();

			m_EntityIDFramebuffer->ClearAttachmentInt(0, -1); // 把EntityID数据置-1

			RendererAPI::SetCullFaceEnable(true);
			RendererAPI::SetStencilTestEnable(false);
			RendererAPI::SetDepthTestEnable(true);
			RendererAPI::SetBlendEnable(false);

			Renderer::GetShaderLibrary()->Get("EntityID")->Bind();
			m_ActiveScene->SetRenderType(RenderType::ENTITYID);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);

			m_EntityIDFramebuffer->Unbind();
		}

	}

	void SceneRenderer::LightingMode()
	{
		m_LightingModeFramebuffer->Bind();
		{
			RendererAPI::SetStencilMask(0xFF);
			Renderer::Clear();

			m_LightingModeFramebuffer->ClearAttachmentInt(0, 0); // 把LightingMode数据置0

			RendererAPI::SetCullFaceEnable(true);
			RendererAPI::SetStencilTestEnable(false);
			RendererAPI::SetDepthTestEnable(true);
			RendererAPI::SetBlendEnable(false);

			Renderer::GetShaderLibrary()->Get("LightingMode")->Bind();
			m_ActiveScene->SetRenderType(RenderType::LIGHTING_MODE);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);

			m_LightingModeFramebuffer->Unbind();
		}

	}
	
	void SceneRenderer::MidProcessing()
	{
		SSAO();

		DirectionalLightShadow();
		PointLightShadow();
		SpotLightShadow();

		m_LightShadingFramebuffer->Bind();
		{
			RendererAPI::SetStencilMask(0xFF);
			Renderer::Clear();

			m_LightShadingFramebuffer->Unbind();
		}

		m_LightShadingFramebuffer->BlitDepthFramebuffer(
			m_ResolvedGBufferFramebuffer->GetRendererID(),
			m_LightShadingFramebuffer->GetRendererID(),
			0, 0,
			m_ResolvedGBufferFramebuffer->GetSpecification().Width,
			m_ResolvedGBufferFramebuffer->GetSpecification().Height,
			0, 0,
			m_LightShadingFramebuffer->GetSpecification().Width,
			m_LightShadingFramebuffer->GetSpecification().Height
		);

		DirectionalLight();
		PointLight();
		SpotLight();
		LightShading();

		m_PBRLightShadingFramebuffer->Bind();
		{
			RendererAPI::SetStencilMask(0xFF);
			Renderer::Clear();

			m_PBRLightShadingFramebuffer->Unbind();
		}

		m_PBRLightShadingFramebuffer->BlitDepthFramebuffer(
			m_ResolvedGBufferFramebuffer->GetRendererID(),
			m_PBRLightShadingFramebuffer->GetRendererID(),
			0, 0,
			m_ResolvedGBufferFramebuffer->GetSpecification().Width,
			m_ResolvedGBufferFramebuffer->GetSpecification().Height,
			0, 0,
			m_PBRLightShadingFramebuffer->GetSpecification().Width,
			m_PBRLightShadingFramebuffer->GetSpecification().Height
		);

		PBRDirectionalLight();
		PBRPointLight();
		PBRSpotLight();
		PBRLightShading();
	}

	void SceneRenderer::SSAO()
	{
		auto ssao = m_ActiveScene->GetSSAO();
		UniformBufferManager::GetUniformBuffer("SSAO")->SetData(&ssao, sizeof(SSAOData));
		m_SSAOFramebuffer->Bind();
		{
			Renderer::Clear();
			Renderer::GetShaderLibrary()->Get("SSAO")->Bind();
			uint32_t positionDepthTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t normalTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(2);
			Texture::Bind(positionDepthTextureID, 0);
			Texture::Bind(normalTextureID, 1);
			m_NoiseTexture->Bind(2);
			FullQuad::DrawIndexed();

			m_SSAOFramebuffer->Unbind();
		}

		m_SSAOBlurFramebuffer->Bind();
		{
			Renderer::Clear();
			Renderer::GetShaderLibrary()->Get("SSAOBlur")->Bind();
			uint32_t ssaoColorBuffer = m_SSAOFramebuffer->GetColorAttachmentRendererID(0);
			Texture::Bind(ssaoColorBuffer, 0);
			FullQuad::DrawIndexed();
			m_SSAOFramebuffer->Unbind();
		}

	}

	void SceneRenderer::SkyboxRender()
	{
		m_DeferredShadingFramebuffer->Bind();
		{
			// 天空盒
			RendererAPI::SetCullFaceEnable(false);
			RendererAPI::SetStencilTestEnable(false);
			RendererAPI::SetDepthTestEnable(true);
			RendererAPI::SetBlendEnable(false);

			Renderer::GetShaderLibrary()->Get("Skybox")->Bind();
			RendererAPI::SetDepthTestFunc(DepthTestFunc::LEQUAL);  // 修改深度函数，小于等于深度缓冲区的内容时，深度测试能够通过
			m_ActiveScene->SetRenderType(RenderType::SKYBOX);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);
			RendererAPI::SetDepthTestFunc(DepthTestFunc::LESS); // 将深度函数恢复为默认值

			m_DeferredShadingFramebuffer->Unbind();
		}

	}

	void SceneRenderer::Particle()
	{
		m_DeferredShadingFramebuffer->Bind();
		{
			RendererAPI::SetCullFaceEnable(false);
			RendererAPI::SetStencilTestEnable(false);
			RendererAPI::SetDepthTestEnable(true);            // 深度测试开启，确保被不透明物体遮挡
			//RendererAPI::SetDepthTestDepthMask(false);      // 深度写入关闭（透明物体不应写深度）
			RendererAPI::SetBlendEnable(true);
			RendererAPI::SetBlendFunc(BlendFunc::SRC_ALPHA, BlendFunc::ONE_MINUS_SRC_ALPHA); // 标准 Alpha 混合


			Renderer::GetShaderLibrary()->Get("Particle")->Bind();
			m_ActiveScene->SetRenderType(RenderType::PARTICLE);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);

			//RendererAPI::SetDepthTestDepthMask(true);

			m_DeferredShadingFramebuffer->Unbind();
		}
	}

	void SceneRenderer::DrawOutlines()
	{
		m_DeferredShadingFramebuffer->Bind();
		{
			// 物体轮廓(Object Outlining)：绘制放大的物体，但只绘制“模板值不等于 1”的像素，即只绘制边缘扩展出去的那一圈。
			RendererAPI::SetCullFaceEnable(false);

			// 仅绘制模板值 != 1 的区域
			RendererAPI::SetStencilTestEnable(true);
			// 只有模板值 不是 1 的片元才通过（即背景区域 0 通过）。
			RendererAPI::SetStencilFunc(StencilFunc::NOTEQUAL, 1, 0xFF);
			// 不论任何测试的结果是如何，模板缓冲都会保留它的值
			RendererAPI::SetStencilOp(StencilOp::KEEP, StencilOp::KEEP, StencilOp::KEEP);
			// 禁止写入（保持模板不变）
			RendererAPI::SetStencilMask(0x00);

			RendererAPI::SetDepthTestEnable(false);
			// Debug记录：glDepthMask(GL_FALSE)（禁止写入深度）导致 BlitDepthFramebuffer 调用无法把源深度数据拷贝到目标深度缓冲。
			// 目标帧缓冲的深度数据残留或损坏，最终表现为奇怪的黑影。
			// 解决方案：在 DrawOutlines 内部恢复状态
			// 禁止写入深度
			RendererAPI::SetDepthTestDepthMask(false);
			// 如果需要半透明轮廓，开启混合
			RendererAPI::SetBlendEnable(true);
			RendererAPI::SetBlendFunc(BlendFunc::SRC_ALPHA, BlendFunc::ONE_MINUS_SRC_ALPHA);

			Renderer::GetShaderLibrary()->Get("Outline")->Bind();
			m_ActiveScene->SetRenderType(RenderType::OUTLINE);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);

			RendererAPI::SetDepthTestDepthMask(true);

			m_DeferredShadingFramebuffer->Unbind();
		}
	}

	void SceneRenderer::HDRAndBloom()
	{
		RendererAPI::SetCullFaceEnable(false);
		RendererAPI::SetStencilTestEnable(false);
		RendererAPI::SetDepthTestEnable(false);
		RendererAPI::SetBlendEnable(false);

		bool horizontal = true, firstIteration = true;
		// 模糊处理10次（5次垂直5次水平）
		uint32_t amount = 10;
		Renderer::GetShaderLibrary()->Get("GaussianBlur")->Bind();
		m_GaussianBlurFramebuffer[horizontal]->Bind();
		{
			RendererAPI::SetStencilMask(0xFF);
			Renderer::Clear();
		}
		m_GaussianBlurFramebuffer[!horizontal]->Bind();
		{
			RendererAPI::SetStencilMask(0xFF);
			Renderer::Clear();
		}
		uint32_t bloomTextureID = m_DeferredShadingFramebuffer->GetColorAttachmentRendererID(1);
		for (uint32_t i = 0; i < amount; i++)
		{
			m_GaussianBlurFramebuffer[horizontal]->Bind();
			{
				RendererAPI::SetStencilMask(0xFF);
				Renderer::Clear();

				UniformBufferManager::GetUniformBuffer("GaussianBlur")->SetData(&horizontal, sizeof(bool));
				Texture::Bind(firstIteration ? bloomTextureID : m_GaussianBlurFramebuffer[!horizontal]->GetColorAttachmentRendererID(0), 0);
				FullQuad::DrawIndexed();
				horizontal = !horizontal;

				if (firstIteration)
					firstIteration = false;
			}
		}
		m_GaussianBlurFramebuffer[0]->Unbind();

		bool bloomEnabled = m_ActiveScene->GetBloomEnabled();
		UniformBufferManager::GetUniformBuffer("BloomEnabled")->SetData(&bloomEnabled, sizeof(bool));
		float exposure = m_ActiveScene->GetExposure();
		UniformBufferManager::GetUniformBuffer("Exposure")->SetData(&exposure, sizeof(float));
		m_DeferredShadingFramebuffer->Bind();
		{
			Renderer::GetShaderLibrary()->Get("HDRAndBloom")->Bind();
			uint32_t screenTextureID = m_DeferredShadingFramebuffer->GetColorAttachmentRendererID(0);
			Texture::Bind(screenTextureID, 0);
			uint32_t gaussianBlurTextureID = m_GaussianBlurFramebuffer[!horizontal]->GetColorAttachmentRendererID(0);
			Texture::Bind(gaussianBlurTextureID, 1);
			FullQuad::DrawIndexed();

			m_DeferredShadingFramebuffer->Unbind();

		}
	}

	void SceneRenderer::NormalVisualization()
	{
		m_DeferredShadingFramebuffer->Bind();
		{
			RendererAPI::SetCullFaceEnable(false);
			RendererAPI::SetStencilTestEnable(false);
			RendererAPI::SetDepthTestEnable(true);
			RendererAPI::SetBlendEnable(false);

			Renderer::GetShaderLibrary()->Get("NormalVisualization")->Bind();
			m_ActiveScene->SetRenderType(RenderType::NORMAL_VISUALIZATION);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);

			m_DeferredShadingFramebuffer->Unbind();
		}
	}

	void SceneRenderer::PostProcessing()
	{
		struct PostProcessingUBO
		{
			uint32_t flags = 0;
			float kernel0 = 0.0f;
			float kernel1 = 0.0f;
			float kernel2 = 0.0f;
			float kernel3 = 1.0f;
			float kernel4 = 0.0f;
			float kernel5 = 0.0f;
			float kernel6 = 0.0f;
			float kernel7 = 0.0f;
			float kernel8 = 0.0f;
		};

		m_PostProcessingFramebuffer->Bind();
		{
			auto& postProcessing = PostProcessing::GetInstance();
			PostProcessingUBO ubo;
			ubo.flags = postProcessing.GetPostProcessingFlags();
			if (postProcessing.HasPostProcessingFlag(PostProcessingFlags::KERNEL))
			{
				if (postProcessing.HasKernelFlag(KernelFlags::SHARPEN))
				{
					float kernel[9] = {
						-1, -1, -1,
						-1,  9, -1,
						-1, -1, -1
					};
					ubo.kernel0 = kernel[0];
					ubo.kernel1 = kernel[1];
					ubo.kernel2 = kernel[2];
					ubo.kernel3 = kernel[3];
					ubo.kernel4 = kernel[4];
					ubo.kernel5 = kernel[5];
					ubo.kernel6 = kernel[6];
					ubo.kernel7 = kernel[7];
					ubo.kernel8 = kernel[8];
				}
				else if (postProcessing.HasPostProcessingFlag(PostProcessingFlags::KERNEL) && postProcessing.HasKernelFlag(KernelFlags::BLUR))
				{
					float kernel[9] = {
						1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
						2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
						1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f
					};
					ubo.kernel0 = kernel[0];
					ubo.kernel1 = kernel[1];
					ubo.kernel2 = kernel[2];
					ubo.kernel3 = kernel[3];
					ubo.kernel4 = kernel[4];
					ubo.kernel5 = kernel[5];
					ubo.kernel6 = kernel[6];
					ubo.kernel7 = kernel[7];
					ubo.kernel8 = kernel[8];
				}
			}
			int size = sizeof(PostProcessingUBO);
			UniformBufferManager::GetUniformBuffer("PostProcessing")->SetData(&ubo, sizeof(PostProcessingUBO));

			uint32_t screenTextureID = m_DeferredShadingFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t entityIDTextureID = m_EntityIDFramebuffer->GetColorAttachmentRendererID(0);
			Texture::Bind(screenTextureID, 0);
			Texture::Bind(entityIDTextureID, 1);

			RendererAPI::SetCullFaceEnable(false);
			RendererAPI::SetStencilTestEnable(false);
			RendererAPI::SetDepthTestEnable(false);
			RendererAPI::SetBlendEnable(false);

			Renderer::GetShaderLibrary()->Get("PostProcessing")->Bind();
			FullQuad::DrawIndexed();

			m_PostProcessingFramebuffer->Unbind();
		}
	}

	void SceneRenderer::RenderScene()
	{
		switch (m_SceneState)
		{
		case SceneState::Edit:
			m_ActiveScene->OnRenderEditor();
			break;
		case SceneState::Simulate:
			m_ActiveScene->OnRenderSimulation();
			break;
		case SceneState::Play:
			m_ActiveScene->OnRenderRuntime();
			break;
		}
	}

}