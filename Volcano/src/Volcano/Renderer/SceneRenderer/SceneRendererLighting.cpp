#include "volpch.h"
#include "Volcano/Renderer/SceneRenderer.h"

#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Renderer/RendererItem/FullQuad.h"
#include "Volcano/Renderer/UniformBuffer.h"
#include "Volcano/Renderer/Texture.h"

#include "Volcano/Scene/SceneBuffer.h"

namespace Volcano {

	void SceneRenderer::DirectionalLightShadow()
	{
		if (SceneBuffer::GetInstance().lightCount.directionalLightShadowCount == 0)
			return;

		m_DirectionalLightDepthMapFramebuffer->Bind();
		{
			RendererAPI::SetStencilMask(0xFF);
			Renderer::Clear();

			RendererAPI::SetCullFaceEnable(true);
			RendererAPI::SetStencilTestEnable(false);
			RendererAPI::SetDepthTestEnable(true);
			RendererAPI::SetBlendEnable(false);

			Renderer::GetShaderLibrary()->Get("DirectionalLightShadowDepth")->Bind();
			// 应对阴影悬浮：阴影偏移后阴影明显地偏离了实际物体，通过剔除正面修复悬浮。
			// 因为该方法会使接近阴影面的物体可能出现不正确的效果，故暂时注释掉不用
			//RendererAPI::SetCullFaceFunc(CullFaceFunc::FRONT);
			m_ActiveScene->SetRenderType(RenderType::SHADOW);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);
			//RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);

			m_DirectionalLightDepthMapFramebuffer->Unbind();
		}

	}

	void SceneRenderer::PointLightShadow()
	{
		if (SceneBuffer::GetInstance().lightCount.pointLightShadowCount == 0)
			return;

		m_PointLightDepthMapFramebuffer->Bind();
		{
			RendererAPI::SetStencilMask(0xFF);
			Renderer::Clear();

			RendererAPI::SetCullFaceEnable(true);
			RendererAPI::SetStencilTestEnable(false);
			RendererAPI::SetDepthTestEnable(true);
			RendererAPI::SetBlendEnable(false);

			Renderer::GetShaderLibrary()->Get("PointLightShadowDepth")->Bind();
			//RendererAPI::SetCullFaceFunc(CullFaceFunc::FRONT);
			m_ActiveScene->SetRenderType(RenderType::SHADOW);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);
			//RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);

			m_PointLightDepthMapFramebuffer->Unbind();
		}

	}

	void SceneRenderer::SpotLightShadow()
	{
		if (SceneBuffer::GetInstance().lightCount.spotLightShadowCount == 0)
			return;

		m_SpotLightDepthMapFramebuffer->Bind();
		{
			RendererAPI::SetStencilMask(0xFF);
			Renderer::Clear();

			RendererAPI::SetCullFaceEnable(true);
			RendererAPI::SetStencilTestEnable(false);
			RendererAPI::SetDepthTestEnable(true);
			RendererAPI::SetBlendEnable(false);

			Renderer::GetShaderLibrary()->Get("SpotLightShadowDepth")->Bind();
			//RendererAPI::SetCullFaceFunc(CullFaceFunc::FRONT);
			m_ActiveScene->SetRenderType(RenderType::SHADOW);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);
			//RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);

			m_SpotLightDepthMapFramebuffer->Unbind();
		}
	}

	void SceneRenderer::DirectionalLight()
	{
		auto& sceneBuffer = SceneBuffer::GetInstance();
		if (sceneBuffer.lightCount.directionalLightCount == 0)
			return;

		m_LightShadingFramebuffer->Bind();
		{
			uint32_t positionDepthTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t normalTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(2);
			uint32_t lightingModeTextureID = m_LightingModeFramebuffer->GetColorAttachmentRendererID();

			uint32_t directionalLightShadowTextureID = m_DirectionalLightDepthMapFramebuffer->GetDepthAttachmentRendererID();

			Texture::Bind(positionDepthTextureID, 0);
			Texture::Bind(normalTextureID, 1);
			Texture::Bind(lightingModeTextureID, 2);

			Texture::Bind(directionalLightShadowTextureID, 3);

			RendererAPI::SetCullFaceEnable(false);
			RendererAPI::SetStencilTestEnable(false);
			RendererAPI::SetDepthTestEnable(false);
			RendererAPI::SetDepthTestDepthMask(false);
			RendererAPI::SetBlendEnable(true);
			RendererAPI::SetBlendFunc(BlendFunc::ONE, BlendFunc::ONE);

			Renderer::GetShaderLibrary()->Get("DirectionalLightShading")->Bind();
			FullQuad::DrawIndexed();

			RendererAPI::SetDepthTestDepthMask(true);

			m_LightShadingFramebuffer->Unbind();
		}

	}

	void SceneRenderer::PointLight()
	{
		auto& sceneBuffer = SceneBuffer::GetInstance();
		if (sceneBuffer.lightVolumePointLightEntityList.size() == 0)
			return;

		m_LightShadingFramebuffer->Bind();
		{
			RendererAPI::SetStencilMask(0xFF);
			RendererAPI::ClearStencil();

			uint32_t positionDepthTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t normalTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(2);
			uint32_t lightingModeTextureID = m_LightingModeFramebuffer->GetColorAttachmentRendererID();

			uint32_t pointLightShadowTextureID = m_PointLightDepthMapFramebuffer->GetDepthAttachmentRendererID();

			Texture::Bind(positionDepthTextureID, 0);
			Texture::Bind(normalTextureID, 1);
			Texture::Bind(lightingModeTextureID, 2);

			Texture::Bind(pointLightShadowTextureID, 3);

			// 剔除正面，模板测试必定通过，深度测试通过的像素模板值设置为1，打开模板写入
			// 深度测试为体积光背面像素的深度比当前像素深度值更小的时候通过（当前像素在体积光内部或更近）
			RendererAPI::SetCullFaceEnable(true);
			RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);

			RendererAPI::SetStencilTestEnable(true);
			RendererAPI::SetStencilFunc(StencilFunc::ALWAYS, 1, 0xFF);
			RendererAPI::SetStencilOp(StencilOp::KEEP, StencilOp::KEEP, StencilOp::REPLACE);
			RendererAPI::SetStencilMask(0xFF);

			RendererAPI::SetDepthTestEnable(true);
			RendererAPI::SetDepthTestDepthMask(false);
			RendererAPI::SetDepthTestFunc(DepthTestFunc::LEQUAL);

			RendererAPI::SetBlendEnable(false);
			RendererAPI::SetColorMask(false, false, false, false);

			Renderer::GetShaderLibrary()->Get("PointLightShading")->Bind();
			m_ActiveScene->SetRenderType(RenderType::POINT_LIGHT);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);


			// 剔除背面，模板测试为等于1通过，关闭模板写入
			// 深度测试为体积光正面像素的深度比当前深度值更大的时候通过（当前像素在体积光内部）
			RendererAPI::SetCullFaceEnable(true);
			RendererAPI::SetCullFaceFunc(CullFaceFunc::FRONT);

			RendererAPI::SetStencilTestEnable(true);
			RendererAPI::SetStencilFunc(StencilFunc::EQUAL, 1, 0xFF);
			RendererAPI::SetStencilOp(StencilOp::KEEP, StencilOp::KEEP, StencilOp::KEEP);
			RendererAPI::SetStencilMask(0x00);

			RendererAPI::SetBlendEnable(true);
			RendererAPI::SetBlendFunc(BlendFunc::ONE, BlendFunc::ONE);
			RendererAPI::SetColorMask(true, true, true, true);

			RendererAPI::SetDepthTestEnable(true);
			RendererAPI::SetDepthTestDepthMask(false);
			RendererAPI::SetDepthTestFunc(DepthTestFunc::GEQUAL);

			m_ActiveScene->SetRenderType(RenderType::POINT_LIGHT);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);


			RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);
			RendererAPI::SetDepthTestFunc(DepthTestFunc::LESS);
			RendererAPI::SetDepthTestDepthMask(true);

			m_LightShadingFramebuffer->Unbind();
		}

	}

	void SceneRenderer::SpotLight()
	{
		auto& sceneBuffer = SceneBuffer::GetInstance();
		if (sceneBuffer.lightVolumeSpotLightEntityList.size() == 0)
			return;

		m_LightShadingFramebuffer->Bind();
		{
			RendererAPI::SetStencilMask(0xFF);
			RendererAPI::ClearStencil();

			uint32_t positionDepthTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t normalTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(2);
			uint32_t lightingModeTextureID = m_LightingModeFramebuffer->GetColorAttachmentRendererID();

			uint32_t spotLightShadowTextureID = m_SpotLightDepthMapFramebuffer->GetDepthAttachmentRendererID();

			Texture::Bind(positionDepthTextureID, 0);
			Texture::Bind(normalTextureID, 1);
			Texture::Bind(lightingModeTextureID, 2);

			Texture::Bind(spotLightShadowTextureID, 3);

			// 剔除正面，模板测试必定通过，深度测试通过的像素模板值设置为1，打开模板写入
			// 深度测试为体积光背面像素的深度比当前像素深度值更小的时候通过（当前像素在体积光内部或更近）
			RendererAPI::SetCullFaceEnable(true);
			RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);

			RendererAPI::SetStencilTestEnable(true);
			RendererAPI::SetStencilFunc(StencilFunc::ALWAYS, 1, 0xFF);
			RendererAPI::SetStencilOp(StencilOp::KEEP, StencilOp::KEEP, StencilOp::REPLACE);
			RendererAPI::SetStencilMask(0xFF);

			RendererAPI::SetDepthTestEnable(true);
			RendererAPI::SetDepthTestDepthMask(false);
			RendererAPI::SetDepthTestFunc(DepthTestFunc::LEQUAL);

			RendererAPI::SetBlendEnable(false);
			RendererAPI::SetColorMask(false, false, false, false);

			Renderer::GetShaderLibrary()->Get("SpotLightShading")->Bind();
			m_ActiveScene->SetRenderType(RenderType::SPOT_LIGHT);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);


			// 剔除背面，模板测试为等于1通过，关闭模板写入
			// 深度测试为体积光正面像素的深度比当前深度值更大的时候通过（当前像素在体积光内部）
			RendererAPI::SetCullFaceEnable(true);
			RendererAPI::SetCullFaceFunc(CullFaceFunc::FRONT);

			RendererAPI::SetStencilTestEnable(true);
			RendererAPI::SetStencilFunc(StencilFunc::EQUAL, 1, 0xFF);
			RendererAPI::SetStencilOp(StencilOp::KEEP, StencilOp::KEEP, StencilOp::KEEP);
			RendererAPI::SetStencilMask(0x00);

			RendererAPI::SetBlendEnable(true);
			RendererAPI::SetBlendFunc(BlendFunc::ONE, BlendFunc::ONE);

			RendererAPI::SetColorMask(true, true, true, true);

			RendererAPI::SetDepthTestEnable(true);
			RendererAPI::SetDepthTestDepthMask(false);
			RendererAPI::SetDepthTestFunc(DepthTestFunc::GEQUAL);

			m_ActiveScene->SetRenderType(RenderType::SPOT_LIGHT);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);

			RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);
			RendererAPI::SetDepthTestFunc(DepthTestFunc::LESS);
			RendererAPI::SetDepthTestDepthMask(true);

			m_LightShadingFramebuffer->Unbind();
		}

	}

	void SceneRenderer::LightShading()
	{
		m_ActiveScene->UpdateLight();

		auto& sceneBuffer = SceneBuffer::GetInstance();
		if (sceneBuffer.pointLightEntityList.size() == 0 &&
			sceneBuffer.spotLightEntityList.size() == 0
			)
		{
			return;
		}

		m_LightShadingFramebuffer->Bind();
		{
			uint32_t positionDepthTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t albedoTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(1);
			uint32_t normalTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(2);
			uint32_t lightingModeTextureID = m_LightingModeFramebuffer->GetColorAttachmentRendererID();

			uint32_t pointLightShadowTextureID = m_PointLightDepthMapFramebuffer->GetDepthAttachmentRendererID();
			uint32_t spotLightShadowTextureID = m_SpotLightDepthMapFramebuffer->GetDepthAttachmentRendererID();

			Texture::Bind(positionDepthTextureID, 0);
			Texture::Bind(albedoTextureID, 1);
			Texture::Bind(normalTextureID, 2);
			Texture::Bind(lightingModeTextureID, 3);

			Texture::Bind(pointLightShadowTextureID, 4);
			Texture::Bind(spotLightShadowTextureID, 5);

			RendererAPI::SetCullFaceEnable(false);
			RendererAPI::SetStencilTestEnable(false);
			RendererAPI::SetDepthTestEnable(false);
			RendererAPI::SetDepthTestDepthMask(false);
			RendererAPI::SetBlendEnable(true);
			RendererAPI::SetBlendFunc(BlendFunc::ONE, BlendFunc::ONE);

			Renderer::GetShaderLibrary()->Get("LightShading")->Bind();
			FullQuad::DrawIndexed();

			RendererAPI::SetDepthTestDepthMask(true);

			m_LightShadingFramebuffer->Unbind();
		}
	}

	void SceneRenderer::PBRDirectionalLight()
	{
		auto& sceneBuffer = SceneBuffer::GetInstance();
		if (sceneBuffer.lightCount.directionalLightCount == 0)
			return;

		m_PBRLightShadingFramebuffer->Bind();
		{
			uint32_t positionDepthTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t albedoTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(1);
			uint32_t normalTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(2);
			uint32_t roughnessAndAOTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(3);
			uint32_t lightingModeTextureID = m_LightingModeFramebuffer->GetColorAttachmentRendererID();

			uint32_t directionalLightShadowTextureID = m_DirectionalLightDepthMapFramebuffer->GetDepthAttachmentRendererID();

			Texture::Bind(positionDepthTextureID, 0);
			Texture::Bind(albedoTextureID, 1);
			Texture::Bind(normalTextureID, 2);
			Texture::Bind(roughnessAndAOTextureID, 3);
			Texture::Bind(lightingModeTextureID, 4);

			Texture::Bind(directionalLightShadowTextureID, 5);

			RendererAPI::SetCullFaceEnable(false);
			RendererAPI::SetStencilTestEnable(false);
			RendererAPI::SetDepthTestEnable(false);
			RendererAPI::SetDepthTestDepthMask(false);
			RendererAPI::SetBlendEnable(true);
			RendererAPI::SetBlendFunc(BlendFunc::ONE, BlendFunc::ONE);

			Renderer::GetShaderLibrary()->Get("PBRDirectionalLightShading")->Bind();
			FullQuad::DrawIndexed();

			RendererAPI::SetDepthTestDepthMask(true);

			m_PBRLightShadingFramebuffer->Unbind();
		}

	}

	void SceneRenderer::PBRPointLight()
	{
		auto& sceneBuffer = SceneBuffer::GetInstance();
		if (sceneBuffer.lightVolumePointLightEntityList.size() == 0)
			return;

		m_PBRLightShadingFramebuffer->Bind();
		{
			RendererAPI::SetStencilMask(0xFF);
			RendererAPI::ClearStencil();

			uint32_t positionDepthTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t albedoTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(1);
			uint32_t normalTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(2);
			uint32_t roughnessAndAOTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(3);
			uint32_t lightingModeTextureID = m_LightingModeFramebuffer->GetColorAttachmentRendererID();

			uint32_t pointLightShadowTextureID = m_PointLightDepthMapFramebuffer->GetDepthAttachmentRendererID();

			Texture::Bind(positionDepthTextureID, 0);
			Texture::Bind(albedoTextureID, 1);
			Texture::Bind(normalTextureID, 2);
			Texture::Bind(roughnessAndAOTextureID, 3);
			Texture::Bind(lightingModeTextureID, 4);

			Texture::Bind(pointLightShadowTextureID, 5);

			// 剔除正面，模板测试必定通过，深度测试通过的像素模板值设置为1，打开模板写入
			// 深度测试为体积光背面像素的深度比当前像素深度值更小的时候通过（当前像素在体积光内部或更近）
			RendererAPI::SetCullFaceEnable(true);
			RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);

			RendererAPI::SetStencilTestEnable(true);
			RendererAPI::SetStencilFunc(StencilFunc::ALWAYS, 1, 0xFF);
			RendererAPI::SetStencilOp(StencilOp::KEEP, StencilOp::KEEP, StencilOp::REPLACE);
			RendererAPI::SetStencilMask(0xFF);

			RendererAPI::SetDepthTestEnable(true);
			RendererAPI::SetDepthTestDepthMask(false);
			RendererAPI::SetDepthTestFunc(DepthTestFunc::LEQUAL);

			RendererAPI::SetBlendEnable(false);
			RendererAPI::SetColorMask(false, false, false, false);

			Renderer::GetShaderLibrary()->Get("PBRPointLightShading")->Bind();
			m_ActiveScene->SetRenderType(RenderType::POINT_LIGHT);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);


			// 剔除背面，模板测试为等于1通过，关闭模板写入
			// 深度测试为体积光正面像素的深度比当前深度值更大的时候通过（当前像素在体积光内部）
			RendererAPI::SetCullFaceEnable(true);
			RendererAPI::SetCullFaceFunc(CullFaceFunc::FRONT);

			RendererAPI::SetStencilTestEnable(true);
			RendererAPI::SetStencilFunc(StencilFunc::EQUAL, 1, 0xFF);
			RendererAPI::SetStencilOp(StencilOp::KEEP, StencilOp::KEEP, StencilOp::KEEP);
			RendererAPI::SetStencilMask(0x00);

			RendererAPI::SetBlendEnable(true);
			RendererAPI::SetBlendFunc(BlendFunc::ONE, BlendFunc::ONE);
			RendererAPI::SetColorMask(true, true, true, true);

			RendererAPI::SetDepthTestEnable(true);
			RendererAPI::SetDepthTestDepthMask(false);
			RendererAPI::SetDepthTestFunc(DepthTestFunc::GEQUAL);

			m_ActiveScene->SetRenderType(RenderType::POINT_LIGHT);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);


			RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);
			RendererAPI::SetDepthTestFunc(DepthTestFunc::LESS);
			RendererAPI::SetDepthTestDepthMask(true);

			m_PBRLightShadingFramebuffer->Unbind();
		}

	}

	void SceneRenderer::PBRSpotLight()
	{
		auto& sceneBuffer = SceneBuffer::GetInstance();
		if (sceneBuffer.lightVolumeSpotLightEntityList.size() == 0)
			return;

		m_PBRLightShadingFramebuffer->Bind();
		{
			RendererAPI::SetStencilMask(0xFF);
			RendererAPI::ClearStencil();

			uint32_t positionDepthTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t albedoTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(1);
			uint32_t normalTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(2);
			uint32_t roughnessAndAOTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(3);
			uint32_t lightingModeTextureID = m_LightingModeFramebuffer->GetColorAttachmentRendererID();

			uint32_t spotLightShadowTextureID = m_SpotLightDepthMapFramebuffer->GetDepthAttachmentRendererID();

			Texture::Bind(positionDepthTextureID, 0);
			Texture::Bind(albedoTextureID, 1);
			Texture::Bind(normalTextureID, 2);
			Texture::Bind(roughnessAndAOTextureID, 3);
			Texture::Bind(lightingModeTextureID, 4);

			Texture::Bind(spotLightShadowTextureID, 5);

			// 剔除正面，模板测试必定通过，深度测试通过的像素模板值设置为1，打开模板写入
			// 深度测试为体积光背面像素的深度比当前像素深度值更小的时候通过（当前像素在体积光内部或更近）
			RendererAPI::SetCullFaceEnable(true);
			RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);

			RendererAPI::SetStencilTestEnable(true);
			RendererAPI::SetStencilFunc(StencilFunc::ALWAYS, 1, 0xFF);
			RendererAPI::SetStencilOp(StencilOp::KEEP, StencilOp::KEEP, StencilOp::REPLACE);
			RendererAPI::SetStencilMask(0xFF);

			RendererAPI::SetDepthTestEnable(true);
			RendererAPI::SetDepthTestDepthMask(false);
			RendererAPI::SetDepthTestFunc(DepthTestFunc::LEQUAL);

			RendererAPI::SetBlendEnable(false);
			RendererAPI::SetColorMask(false, false, false, false);

			Renderer::GetShaderLibrary()->Get("PBRSpotLightShading")->Bind();
			m_ActiveScene->SetRenderType(RenderType::SPOT_LIGHT);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);


			// 剔除背面，模板测试为等于1通过，关闭模板写入
			// 深度测试为体积光正面像素的深度比当前深度值更大的时候通过（当前像素在体积光内部）
			RendererAPI::SetCullFaceEnable(true);
			RendererAPI::SetCullFaceFunc(CullFaceFunc::FRONT);

			RendererAPI::SetStencilTestEnable(true);
			RendererAPI::SetStencilFunc(StencilFunc::EQUAL, 1, 0xFF);
			RendererAPI::SetStencilOp(StencilOp::KEEP, StencilOp::KEEP, StencilOp::KEEP);
			RendererAPI::SetStencilMask(0x00);

			RendererAPI::SetBlendEnable(true);
			RendererAPI::SetBlendFunc(BlendFunc::ONE, BlendFunc::ONE);
			RendererAPI::SetColorMask(true, true, true, true);

			RendererAPI::SetDepthTestEnable(true);
			RendererAPI::SetDepthTestDepthMask(false);
			RendererAPI::SetDepthTestFunc(DepthTestFunc::GEQUAL);

			m_ActiveScene->SetRenderType(RenderType::SPOT_LIGHT);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);


			RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);
			RendererAPI::SetDepthTestFunc(DepthTestFunc::LESS);
			RendererAPI::SetDepthTestDepthMask(true);

			m_PBRLightShadingFramebuffer->Unbind();
		}

	}

	void SceneRenderer::PBRLightShading()
	{
		m_ActiveScene->UpdateLight();

		auto& sceneBuffer = SceneBuffer::GetInstance();
		if (sceneBuffer.pointLightEntityList.size() == 0 &&
			sceneBuffer.spotLightEntityList.size() == 0
			)
		{
			return;
		}

		m_PBRLightShadingFramebuffer->Bind();
		{
			uint32_t positionDepthTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t albedoTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(1);
			uint32_t normalTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(2);
			uint32_t roughnessAndAOTextureID = m_ResolvedGBufferFramebuffer->GetColorAttachmentRendererID(3);
			uint32_t lightingModeTextureID = m_LightingModeFramebuffer->GetColorAttachmentRendererID();

			uint32_t pointLightShadowTextureID = m_PointLightDepthMapFramebuffer->GetDepthAttachmentRendererID();
			uint32_t spotLightShadowTextureID = m_SpotLightDepthMapFramebuffer->GetDepthAttachmentRendererID();

			Texture::Bind(positionDepthTextureID, 0);
			Texture::Bind(albedoTextureID, 1);
			Texture::Bind(normalTextureID, 2);
			Texture::Bind(roughnessAndAOTextureID, 3);
			Texture::Bind(lightingModeTextureID, 4);

			Texture::Bind(pointLightShadowTextureID, 5);
			Texture::Bind(spotLightShadowTextureID, 6);

			RendererAPI::SetCullFaceEnable(false);
			RendererAPI::SetStencilTestEnable(false);
			RendererAPI::SetDepthTestEnable(false);
			RendererAPI::SetDepthTestDepthMask(false);
			RendererAPI::SetBlendEnable(true);
			RendererAPI::SetBlendFunc(BlendFunc::ONE, BlendFunc::ONE);

			Renderer::GetShaderLibrary()->Get("PBRLightShading")->Bind();
			FullQuad::DrawIndexed();

			RendererAPI::SetDepthTestDepthMask(true);

			m_PBRLightShadingFramebuffer->Unbind();
		}
	}

}