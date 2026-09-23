#include "volpch.h"
#include "Volcano/Renderer/SceneRenderer.h"

#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Scene/Entity.h"
#include "Volcano/Renderer/RendererItem/Skybox.h"
#include "Volcano/Renderer/Texture.h"

#include "Volcano/Scene/SceneBuffer.h"

namespace Volcano {

	void SceneRenderer::ForwardShading()
	{
		RendererAPI::SetCullFaceEnable(true);
		RendererAPI::SetStencilTestEnable(false);
		RendererAPI::SetDepthTestEnable(true);
		RendererAPI::SetBlendEnable(true);

		m_DeferredShadingFramebuffer->Bind();
		{
			uint32_t directionalLightShadowTextureID = m_DirectionalLightDepthMapFramebuffer->GetDepthAttachmentRendererID();
			uint32_t pointLightShadowTextureID = m_PointLightDepthMapFramebuffer->GetDepthAttachmentRendererID();
			uint32_t spotLightShadowTextureID = m_SpotLightDepthMapFramebuffer->GetDepthAttachmentRendererID();

			Texture::Bind(directionalLightShadowTextureID, 3);
			Texture::Bind(pointLightShadowTextureID, 4);
			Texture::Bind(spotLightShadowTextureID, 5);

			auto& skyboxEntity = SceneBuffer::GetInstance().skyboxEntity;
			if (skyboxEntity != nullptr)
			{
				auto& skybox = skyboxEntity->GetComponent<SkyboxComponent>();
				if (skybox.textureType == 0)
				{
					skybox.textureCubeMap->Bind(20);
				}
				else
				{
					skybox.textureCubeSixSided->Bind(20);
				}
			}
			else
			{
				Mesh::GetBlackTextureCube()->Bind(20);
			}

			Renderer::GetShaderLibrary()->Get("ForwardShading")->Bind();
			m_ActiveScene->SetRenderType(RenderType::FORWARD_SHADING);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);

			m_DeferredShadingFramebuffer->Unbind();
		}
	}

}