#include "volpch.h"
#include "Volcano/Renderer/SceneRenderer.h"

#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Renderer/RendererItem/FullQuad.h"
#include "Volcano/Renderer/UniformBuffer.h"
#include "Volcano/Scene/Entity.h"
#include "Volcano/Renderer/RendererItem/Skybox.h"
#include "Volcano/Renderer/Texture.h"

#include "Volcano/Scene/SceneBuffer.h"

namespace Volcano {


	void SceneRenderer::GBuffer()
	{
		m_MSGBufferFramebuffer->Bind();
		{
			// glStencilMask(0x00) 会阻止 glClear 清除模板缓冲。
			RendererAPI::SetStencilMask(0xFF);
			Renderer::Clear();

			RendererAPI::SetCullFaceEnable(true);

			// 把需要模板测试的像素，在模板缓冲区中标记为 1；背景（没有物体）标记为 0。
			RendererAPI::SetStencilTestEnable(true);
			// 所有片元通过，设置 ref 为 1
			RendererAPI::SetStencilFunc(StencilFunc::ALWAYS, 1, 0xFF);
			// 片元通过模板测试后，把缓冲区值替换为 1（ref）
			RendererAPI::SetStencilOp(StencilOp::KEEP, StencilOp::REPLACE, StencilOp::REPLACE);
			// 禁止写入模板值，只在RenderScene中对有需求的物体渲染时会允许写入模板值
			RendererAPI::SetStencilMask(0x00);

			RendererAPI::SetDepthTestEnable(true);
			// 注意！！！ GBuffer渲染时不能开混合模式，渲染会出错
			RendererAPI::SetBlendEnable(false);

			Renderer::GetShaderLibrary()->Get("GBuffer")->Bind();
			m_ActiveScene->SetRenderType(RenderType::G_BUFFER);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);

			m_MSGBufferFramebuffer->Unbind();

		}

		// 使用 BlitColorFramebuffer 将 6 个颜色附件从多采样 FBO 逐附件拷贝到单采样 FBO，
		// 利用 glBlitFramebuffer 自动完成 MSAA 解析（即平均子采样点）。
		m_MSGBufferFramebuffer->BlitColorFramebuffer(
			m_MSGBufferFramebuffer->GetRendererID(),
			m_ResolvedGBufferFramebuffer->GetRendererID(),
			{ 0, 1, 2, 3, 4, 5 },
			0, 0,
			m_MSGBufferFramebuffer->GetSpecification().Width,
			m_MSGBufferFramebuffer->GetSpecification().Height,
			0, 0,
			m_ResolvedGBufferFramebuffer->GetSpecification().Width,
			m_ResolvedGBufferFramebuffer->GetSpecification().Height
		);

		m_MSGBufferFramebuffer->BlitDepthFramebuffer(
			m_MSGBufferFramebuffer->GetRendererID(),
			m_ResolvedGBufferFramebuffer->GetRendererID(),
			0, 0,
			m_MSGBufferFramebuffer->GetSpecification().Width,
			m_MSGBufferFramebuffer->GetSpecification().Height,
			0, 0,
			m_ResolvedGBufferFramebuffer->GetSpecification().Width,
			m_ResolvedGBufferFramebuffer->GetSpecification().Height
		);

		m_MSGBufferFramebuffer->BlitStencilFramebuffer(
			m_MSGBufferFramebuffer->GetRendererID(),
			m_ResolvedGBufferFramebuffer->GetRendererID(),
			0, 0,
			m_MSGBufferFramebuffer->GetSpecification().Width,
			m_ResolvedGBufferFramebuffer->GetSpecification().Height,
			0, 0,
			m_ResolvedGBufferFramebuffer->GetSpecification().Width,
			m_ResolvedGBufferFramebuffer->GetSpecification().Height
		);

	}

	void SceneRenderer::DeferredShading()
	{
		m_DeferredShadingFramebuffer->Bind();
		{
			// glStencilMask(0x00) 会阻止 glClear 清除模板缓冲。
			RendererAPI::SetStencilMask(0xFF);
			Renderer::Clear();

			uint32_t positionAndDepthTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t albedoTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(1);
			uint32_t normalTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(2);
			uint32_t roughnessAndAOTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(3);
			uint32_t emissionTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(4);
			uint32_t ssaoColorBufferBlur = m_SSAOBlurFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t lightingModeTextureID = m_LightingModeFramebuffer->GetColorAttachmentRendererID();

			uint32_t ambientTextureID = m_LightShadingFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t diffuseTextureID = m_LightShadingFramebuffer->GetColorAttachmentRendererID(1);
			uint32_t specularTextureID = m_LightShadingFramebuffer->GetColorAttachmentRendererID(2);
			uint32_t PBRTextureID = m_PBRLightShadingFramebuffer->GetColorAttachmentRendererID();

			Texture::Bind(positionAndDepthTextureID, 0);
			Texture::Bind(albedoTextureID, 1);
			Texture::Bind(normalTextureID, 2);
			Texture::Bind(roughnessAndAOTextureID, 3);
			Texture::Bind(emissionTextureID, 4);
			Texture::Bind(ssaoColorBufferBlur, 5);
			Texture::Bind(lightingModeTextureID, 6);

			Texture::Bind(ambientTextureID, 10);
			Texture::Bind(diffuseTextureID, 11);
			Texture::Bind(specularTextureID, 12);
			Texture::Bind(PBRTextureID, 13);

			auto& sceneBuffer = SceneBuffer::GetInstance();
			if (sceneBuffer.hdrEntity != nullptr)
			{
				bool temp = true;
				UniformBufferManager::GetUniformBuffer("IrradianceEnabled")->SetData(&temp, sizeof(bool));

				HDRComponent hdrComp = sceneBuffer.hdrEntity->GetComponent<HDRComponent>();
				uint32_t irradianceMapTextureID = hdrComp.irradianceMap->GetRendererID();
				uint32_t prefilterMapTextureID = hdrComp.prefilterMap->GetRendererID();
				Texture::Bind(irradianceMapTextureID, 14);
				Texture::Bind(prefilterMapTextureID, 15);
				Texture::Bind(m_BRDFLUT->GetRendererID(), 16);

				hdrComp.envCubeMap->Bind(20);
			}
			else
			{
				bool temp = false;
				UniformBufferManager::GetUniformBuffer("IrradianceEnabled")->SetData(&temp, sizeof(bool));

				auto& skyboxEntity = sceneBuffer.skyboxEntity;
				if (skyboxEntity != nullptr)
				{
					auto& skyboxComp = skyboxEntity->GetComponent<SkyboxComponent>();
					if (skyboxComp.textureType == 0)
					{
						skyboxComp.textureCubeMap->Bind(20);
					}
					else
					{
						skyboxComp.textureCubeSixSided->Bind(20);
					}
				}
				else
				{
					Mesh::GetBlackTextureCube()->Bind(20);
				}
			}

			RendererAPI::SetCullFaceEnable(false);
			RendererAPI::SetStencilTestEnable(false);
			RendererAPI::SetDepthTestEnable(false);
			RendererAPI::SetBlendEnable(false);

			Renderer::GetShaderLibrary()->Get("DeferredShading")->Bind();
			FullQuad::DrawIndexed();

			m_DeferredShadingFramebuffer->Unbind();
		}

		m_DeferredShadingFramebuffer->BlitDepthFramebuffer(
			m_ResolvedGBufferFramebuffer->GetRendererID(),
			m_DeferredShadingFramebuffer->GetRendererID(),
			0, 0,
			m_ResolvedGBufferFramebuffer->GetSpecification().Width,
			m_ResolvedGBufferFramebuffer->GetSpecification().Height,
			0, 0,
			m_DeferredShadingFramebuffer->GetSpecification().Width,
			m_DeferredShadingFramebuffer->GetSpecification().Height
		);

		m_DeferredShadingFramebuffer->BlitStencilFramebuffer(
			m_ResolvedGBufferFramebuffer->GetRendererID(),
			m_DeferredShadingFramebuffer->GetRendererID(),
			0, 0,
			m_ResolvedGBufferFramebuffer->GetSpecification().Width,
			m_ResolvedGBufferFramebuffer->GetSpecification().Height,
			0, 0,
			m_DeferredShadingFramebuffer->GetSpecification().Width,
			m_DeferredShadingFramebuffer->GetSpecification().Height
		);

		ForwardShading();

		if (m_ActiveScene->GetHDREnabled())
		{
			HDRAndBloom();
		}

		SkyboxRender();
		DrawOutlines();
		NormalVisualization();

		Particle();
	}

}