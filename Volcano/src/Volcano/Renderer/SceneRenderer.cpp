#include "volpch.h"
#include "SceneRenderer.h"
#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Renderer/RendererItem/FullQuad.h"
#include "Volcano/Renderer/RendererItem/Skybox.h"
#include "Volcano/Renderer/RendererItem/Sphere.h"
#include <random>
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace Volcano {

	Ref<Framebuffer> SceneRenderer::m_DirectionalDepthMapFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_PointDepthMapFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_SpotDepthMapFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_GBufferFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_LightShadingFramebuffer[2];
	Ref<Framebuffer> SceneRenderer::m_PBRLightShadingFramebuffer[2];
	bool SceneRenderer::m_PBR;
	Ref<Framebuffer> SceneRenderer::m_DeferredShadingFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_SSAOFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_SSAOBlurFramebuffer;
	bool SceneRenderer::m_SSAOSwitch;
	Ref<Texture2D>   SceneRenderer::m_NoiseTexture;
	int   SceneRenderer::m_KernelSize = 64;
	float SceneRenderer::m_Radius = 0.5;
	float SceneRenderer::m_Bias = 0.035;
	float SceneRenderer::m_Power = 1.0;
	Ref<Framebuffer> SceneRenderer::m_BlurFramebuffer[2];
	Ref<Framebuffer> SceneRenderer::m_HDRFramebuffer;
	float SceneRenderer::m_Exposure = 1.5f;
	bool  SceneRenderer::m_Bloom = true;
	Ref<Framebuffer> SceneRenderer::m_PBRFramebuffer;
	Ref<Texture2D>   SceneRenderer::m_EquirectangularMap;
	Ref<TextureCube> SceneRenderer::m_EnvCubemap;
	Ref<TextureCube> SceneRenderer::m_IrradianceMap;
	Ref<TextureCube> SceneRenderer::m_PrefilterMap;
	Ref<Texture2D>   SceneRenderer::m_BRDFLUT;

	Ref<Scene> SceneRenderer::m_ActiveScene;
	Timestep SceneRenderer::m_Timestep;
	SceneState SceneRenderer::m_SceneState;
	EditorCamera SceneRenderer::m_EditorCamera;

	void SceneRenderer::Init()
	{
		FramebufferSpecification fbSpec;
		fbSpec.Attachments = {
			FramebufferTextureFormat::DEPTH_COMPONENT
		};
		fbSpec.Width = 1024;
		fbSpec.Height = 1024;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_DirectionalDepthMapFramebuffer = Framebuffer::Create(fbSpec);


		fbSpec.Attachments = {
			FramebufferTextureFormat::DEPTH_COMPONENT
		};
		fbSpec.Width = 1024;
		fbSpec.Height = 1024;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_CUBE_MAP;
		m_PointDepthMapFramebuffer = Framebuffer::Create(fbSpec);


		fbSpec.Attachments = {
			FramebufferTextureFormat::DEPTH_COMPONENT
		};
		fbSpec.Width = 1024;
		fbSpec.Height = 1024;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_SpotDepthMapFramebuffer = Framebuffer::Create(fbSpec);

		
        fbSpec.Attachments = {
            FramebufferTextureFormat::RGBA16F,     // 位置+深度缓冲，深度范围0.1到50.0
			FramebufferTextureFormat::RGBA8,       // 法线缓冲
            FramebufferTextureFormat::RGBA8,       // 颜色+镜面缓冲Albedo
			FramebufferTextureFormat::RGBA8,       // 粗糙度+AO缓冲
            FramebufferTextureFormat::RED_INTEGER, // EntityID缓冲
            FramebufferTextureFormat::Depth
        };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        fbSpec.Samples = 1;
        fbSpec.ColorType = TextureType::TEXTURE_2D;
        fbSpec.DepthType = TextureType::TEXTURE_2D;
        m_GBufferFramebuffer = Framebuffer::Create(fbSpec);


		fbSpec.Attachments = {
			FramebufferTextureFormat::RGBA16F,
			FramebufferTextureFormat::RGBA16F,
			FramebufferTextureFormat::RGBA16F,
		};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_LightShadingFramebuffer[0] = Framebuffer::Create(fbSpec);
		m_LightShadingFramebuffer[1] = Framebuffer::Create(fbSpec);

		fbSpec.Attachments = {
			FramebufferTextureFormat::RGBA16F,
		};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_PBRLightShadingFramebuffer[0] = Framebuffer::Create(fbSpec);
		m_PBRLightShadingFramebuffer[1] = Framebuffer::Create(fbSpec);

		fbSpec.Attachments = {
			FramebufferTextureFormat::RGBA16F,
			FramebufferTextureFormat::RED_INTEGER,
			FramebufferTextureFormat::RGBA16F,
			FramebufferTextureFormat::Depth     //添加一个深度附件，默认Depth = DEPTH24STENCIL8
		};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_DeferredShadingFramebuffer = Framebuffer::Create(fbSpec);

		fbSpec.Attachments = {
			FramebufferTextureFormat::RED
		};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_SSAOFramebuffer = Framebuffer::Create(fbSpec);

		fbSpec.Attachments = {
			FramebufferTextureFormat::RED
		};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_SSAOBlurFramebuffer = Framebuffer::Create(fbSpec);

		m_SSAOBlurFramebuffer->Bind();
		m_SSAOBlurFramebuffer->ClearAttachmentFloat(0, 1.0f);
		m_SSAOBlurFramebuffer->Unbind();

		// Sample kernel
		// 采样核心
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // 随机浮点数，[0.0, 1.0]
		std::default_random_engine generator;
		std::vector<glm::vec3> ssaoKernel;
		const uint32_t ssaoKernelSize = 64;
		for (uint32_t i = 0; i < ssaoKernelSize; ++i)
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator));
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);
			float scale = float(i) / float(ssaoKernelSize);

			// 将核心样本靠近原点分布Scale samples s.t. they're more aligned to center of kernel
			scale = 0.1f + scale * scale * (1.0f - 0.1f);//lerp(0.1f, 1.0f, scale * scale);  //return a + f * (b - a);
			sample *= scale;
			ssaoKernel.push_back(sample);
		}
		for (uint32_t i = 0; i < ssaoKernelSize; i++)
			UniformBufferManager::GetUniformBuffer("Samples")->SetData(&ssaoKernel[i], sizeof(glm::vec3), i * 4 * sizeof(float));


		// Noise texture
		// 4x4朝向切线空间平面法线的随机旋转向量数组
		std::vector<glm::vec3> ssaoNoise;
		for (uint32_t i = 0; i < 16; i++)
		{
			glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
			ssaoNoise.push_back(noise);
		}

		m_NoiseTexture = Texture2D::Create(4, 4, TextureFormat::RGBA16F, TextureFormat::RGB);
		m_NoiseTexture->SetData(&ssaoNoise[0], ssaoNoise.size() * 3);


		fbSpec.Attachments = {
			FramebufferTextureFormat::RGBA16F
		};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_BlurFramebuffer[0] = Framebuffer::Create(fbSpec);
		m_BlurFramebuffer[1] = Framebuffer::Create(fbSpec);
		m_HDRFramebuffer = Framebuffer::Create(fbSpec);

		UniformBufferManager::GetUniformBuffer("Exposure")->SetData(&m_Exposure, sizeof(float));
		UniformBufferManager::GetUniformBuffer("Exposure")->SetData(&m_Bloom, sizeof(bool), sizeof(float));


        fbSpec.Attachments = {
            FramebufferTextureFormat::RGBA16F
        };
        fbSpec.Width = 512;
        fbSpec.Height = 512;
        fbSpec.Samples = 1;
        fbSpec.ColorType = TextureType::TEXTURE_2D;
        fbSpec.DepthType = TextureType::TEXTURE_2D;
        m_PBRFramebuffer = Framebuffer::Create(fbSpec);

        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViewProjections[] =
        {
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };
        for(uint32_t i = 0; i < 6; i++)
            captureViewProjections[i] = captureProjection * captureViewProjections[i];

        m_EquirectangularMap = Texture2D::Create("SandBoxProject/Assets/Textures/hdr/newport_loft.hdr", true, TextureFormat::RGB16F);
        m_EnvCubemap = TextureCube::Create(TextureFormat::RGB16F, 512, 512);
        
        Renderer::SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        m_PBRFramebuffer->Bind();
        {
            Renderer::GetShaderLibrary()->Get("EquirectangularToCubemap")->Bind();
            m_EquirectangularMap->Bind();
            
            for (uint32_t i = 0; i < 6; ++i)
            {
                UniformBufferManager::GetUniformBuffer("PBR")->SetData(&captureViewProjections[i], 4 * 4 * sizeof(float));
                // 将envCubemap的6个面依次绑定到m_PBRFramebuffer的颜色附件
                m_PBRFramebuffer->SetColorAttachment(m_EnvCubemap, TextureType(uint32_t(TextureType::TEXTURE_CUBE_MAP_POSITIVE_X) + i));
                Renderer::Clear();

                // 取消面剔除
                RendererAPI::SetCullFace(false);
                Skybox::DrawIndexed();
                RendererAPI::SetCullFace(true);

            }
            m_PBRFramebuffer->Unbind();
        }

        std::vector<std::string> faces
        {
            std::string("SandBoxProject/Assets/Textures/skybox/right.jpg"),
            std::string("SandBoxProject/Assets/Textures/skybox/left.jpg"),
            std::string("SandBoxProject/Assets/Textures/skybox/top.jpg"),
            std::string("SandBoxProject/Assets/Textures/skybox/bottom.jpg"),
            std::string("SandBoxProject/Assets/Textures/skybox/front.jpg"),
            std::string("SandBoxProject/Assets/Textures/skybox/back.jpg"),
        };
        m_EnvCubemap = TextureCube::Create(faces);

        m_IrradianceMap = TextureCube::Create(TextureFormat::RGB16F, 32, 32);
        m_PBRFramebuffer->Resize(32, 32);

        m_PBRFramebuffer->Bind();
        {
            Renderer::GetShaderLibrary()->Get("IrradianceConvolution")->Bind();
            m_EnvCubemap->Bind();

            for (uint32_t i = 0; i < 6; ++i)
            {
                UniformBufferManager::GetUniformBuffer("PBR")->SetData(&captureViewProjections[i], 4 * 4 * sizeof(float));
                m_PBRFramebuffer->SetColorAttachment(m_IrradianceMap, TextureType(uint32_t(TextureType::TEXTURE_CUBE_MAP_POSITIVE_X) + i));
                Renderer::Clear();
                RendererAPI::SetCullFace(false);
                Skybox::DrawIndexed();
                RendererAPI::SetCullFace(true);
            }
            m_PBRFramebuffer->Unbind();
        }

        uint32_t prefilterMapWidth = 128;
        uint32_t prefilterMapHeight = 128;
        m_PrefilterMap = TextureCube::Create(TextureFormat::RGB16F, prefilterMapWidth, prefilterMapHeight);

        uint32_t maxMipLevels = 5;
        for (uint32_t mip = 0; mip < maxMipLevels; ++mip)
        {
            // reisze framebuffer according to mip-level size.
            prefilterMapWidth  = static_cast<uint32_t>(128 * std::pow(0.5, mip));
            prefilterMapHeight = static_cast<uint32_t>(128 * std::pow(0.5, mip));
            m_PBRFramebuffer->Resize(prefilterMapWidth, prefilterMapHeight);
            m_PBRFramebuffer->Bind();
            {
                Renderer::GetShaderLibrary()->Get("Prefilter")->Bind();
                m_EnvCubemap->Bind();
                float roughness = (float)mip / (float)(maxMipLevels - 1);
                UniformBufferManager::GetUniformBuffer("Prefilter")->SetData(&roughness, sizeof(float));
                for (uint32_t i = 0; i < 6; ++i)
                {
                    UniformBufferManager::GetUniformBuffer("PBR")->SetData(&captureViewProjections[i], 4 * 4 * sizeof(float));
                    m_PBRFramebuffer->SetColorAttachment(m_PrefilterMap, TextureType(uint32_t(TextureType::TEXTURE_CUBE_MAP_POSITIVE_X) + i), 0, mip);
                    Renderer::Clear();
                    RendererAPI::SetCullFace(false);
                    Skybox::DrawIndexed();
                    RendererAPI::SetCullFace(true);
                }
                m_PBRFramebuffer->Unbind();
            }
        }

        /*
        m_BRDFLUT = Texture2D::Create(512, 512, TextureFormat::RG16F, TextureFormat::RG, TextureWrap::CLAMP_TO_EDGE);
        m_PBRFramebuffer->Resize(512, 512);
        m_PBRFramebuffer->Bind();
        {
            Renderer::GetShaderLibrary()->Get("BRDF")->Bind();
            m_PBRFramebuffer->SetColorAttachment(m_BRDFLUT, TextureType::TEXTURE_2D);
            Renderer::Clear();
            Renderer::DrawIndexed(windowVa, windowVa->GetIndexBuffer()->GetCount());
            m_PBRFramebuffer->Unbind();
        }
        */
        m_BRDFLUT = Texture2D::Create("SandBoxProject/Assets/Textures/BRDF_LUT.tga", true, TextureFormat::RG16F);
        
        Sphere::SetIrradianceMap(m_IrradianceMap);
        Sphere::SetPrefilterMap(m_PrefilterMap);
        Sphere::SetBRDFLUT(m_BRDFLUT);
        Skybox::SetTexture(m_EnvCubemap);
        

	}

	void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
	{
	}

	void SceneRenderer::BeginScene(Ref<Scene> scene, Timestep ts, SceneState sceneState, EditorCamera& editorCamera)
	{
		m_ActiveScene = scene;
		m_Timestep = ts;
		m_SceneState = sceneState;
		m_EditorCamera = editorCamera;
	}

	void SceneRenderer::EndScene()
	{
	}

	void SceneRenderer::DirectionalShadow()
	{
		m_DirectionalDepthMapFramebuffer->Bind();
		{
			Renderer::Clear();

			//RendererAPI::SetCullFaceFunc(CullFaceFunc::FRONT);

			m_ActiveScene->SetRenderType(RenderType::SHADOW_DIRECTIONALLIGHT);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);
			//RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);

			m_DirectionalDepthMapFramebuffer->Unbind();
		}
	}

	void SceneRenderer::PointShadow()
	{
		m_PointDepthMapFramebuffer->Bind();
		{
			Renderer::Clear();
			//RendererAPI::SetCullFaceFunc(CullFaceFunc::FRONT);
			m_ActiveScene->SetRenderType(RenderType::SHADOW_POINTLIGHT);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);
			//RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);
			m_PointDepthMapFramebuffer->Unbind();
		}
	}

	void SceneRenderer::SpotShadow()
	{
		m_SpotDepthMapFramebuffer->Bind();
		{
			Renderer::Clear();

			//RendererAPI::SetCullFaceFunc(CullFaceFunc::FRONT);

			m_ActiveScene->SetRenderType(RenderType::SHADOW_SPOTLIGHT);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);
			//RendererAPI::SetCullFaceFunc(CullFaceFunc::BACK);

			m_SpotDepthMapFramebuffer->Unbind();
		}
	}

	void SceneRenderer::GBuffer()
	{
        m_GBufferFramebuffer->Bind();
        {
            Renderer::Clear();
            m_GBufferFramebuffer->ClearAttachmentInt(4, -1); // 把EntityID数据置-1

			RendererAPI::SetCullFace(false);
            m_ActiveScene->SetRenderType(RenderType::G_BUFFER);
            RenderScene();
            m_ActiveScene->SetRenderType(RenderType::NORMAL);
			RendererAPI::SetCullFace(true);

            m_GBufferFramebuffer->Unbind();

        }
	}
	
	void SceneRenderer::SSAO()
	{
		UniformBufferManager::GetUniformBuffer("SSAO")->SetData(&m_KernelSize, sizeof(float));
		UniformBufferManager::GetUniformBuffer("SSAO")->SetData(&m_Radius,     sizeof(float), sizeof(float));
		UniformBufferManager::GetUniformBuffer("SSAO")->SetData(&m_Bias,       sizeof(float), 2 * sizeof(float));
		UniformBufferManager::GetUniformBuffer("SSAO")->SetData(&m_Power,      sizeof(float), 3 * sizeof(float));

		m_SSAOFramebuffer->Bind();
		{
			Renderer::Clear();
			Renderer::GetShaderLibrary()->Get("SSAO")->Bind();
			uint32_t positionTextureID = m_GBufferFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t normalTextureID = m_GBufferFramebuffer->GetColorAttachmentRendererID(1);
			Texture::Bind(positionTextureID, 0);
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

	void SceneRenderer::PBRDeferredShading()
	{
		m_DeferredShadingFramebuffer->BlitDepthFramebuffer(
			m_GBufferFramebuffer->GetRendererID(), m_DeferredShadingFramebuffer->GetRendererID(),
			0, 0, m_GBufferFramebuffer->GetSpecification().Width, m_GBufferFramebuffer->GetSpecification().Height,
			0, 0, m_DeferredShadingFramebuffer->GetSpecification().Width, m_DeferredShadingFramebuffer->GetSpecification().Height
		);

		if (!m_SSAOSwitch)
		{
			m_SSAOBlurFramebuffer->Bind();
			m_SSAOBlurFramebuffer->ClearAttachmentFloat(0, 1.0f);
			m_SSAOBlurFramebuffer->Unbind();
		}

		bool flag = false;
		uint32_t directionalLightSize = m_ActiveScene->GetDirectionalLightEntity() == nullptr ? 0 : 1;
		uint32_t pointLightSize = m_ActiveScene->GetPointLightEntities().size();
		uint32_t spotLightSize = m_ActiveScene->GetSpotLightEntities().size();

		m_PBRLightShadingFramebuffer[!flag]->Bind();
		{
			Renderer::Clear(0.0f, 0.0f, 0.0f, 0.0f);
			m_PBRLightShadingFramebuffer[!flag]->Unbind();
		}
		if (glm::max(directionalLightSize, pointLightSize, spotLightSize) > 0)
		{
			for (uint32_t i = 0; i < glm::max(directionalLightSize, pointLightSize, spotLightSize); i++)
			{
				m_ActiveScene->UpdateLight(i);
				if(directionalLightSize > 0)
				    DirectionalShadow();
				if (pointLightSize > i)
				    PointShadow();
				if (spotLightSize > i)
				    SpotShadow();
				m_PBRLightShadingFramebuffer[flag]->Bind();
				{
					Renderer::Clear();
					m_ActiveScene->SetRenderType(RenderType::PBRLIGHT_SHADING);

					uint32_t positionTextureID    = m_GBufferFramebuffer->GetColorAttachmentRendererID(0);
					uint32_t normalTextureID      = m_GBufferFramebuffer->GetColorAttachmentRendererID(1);
					uint32_t albedoTextureID      = m_GBufferFramebuffer->GetColorAttachmentRendererID(2);
					uint32_t roughnessAOTextureID = m_GBufferFramebuffer->GetColorAttachmentRendererID(3);
					uint32_t LoTextureID          = m_PBRLightShadingFramebuffer[!flag]->GetColorAttachmentRendererID(0);
					uint32_t directionalShadowTextureID = m_DirectionalDepthMapFramebuffer->GetDepthAttachmentRendererID();
					uint32_t pointShadowTextureID = m_PointDepthMapFramebuffer->GetDepthAttachmentRendererID();
					uint32_t spotShadowTextureID  = m_SpotDepthMapFramebuffer->GetDepthAttachmentRendererID();
					Texture::Bind(positionTextureID, 0);
					Texture::Bind(normalTextureID, 1);
					Texture::Bind(albedoTextureID, 2);
					Texture::Bind(roughnessAOTextureID, 3);
					Texture::Bind(LoTextureID, 4);
					Texture::Bind(directionalShadowTextureID, 5);
					Texture::Bind(pointShadowTextureID, 6);
					Texture::Bind(spotShadowTextureID, 7);

					Renderer::GetShaderLibrary()->Get("PBRLightShading")->Bind();

					FullQuad::DrawIndexed();

					m_ActiveScene->SetRenderType(RenderType::NORMAL);
					flag = !flag;
					m_PBRLightShadingFramebuffer[flag]->Unbind();
				}
			}
		}
		else
		{
			m_PBRLightShadingFramebuffer[flag]->Bind();
			{
				Renderer::Clear(0.0f, 0.0f, 0.0f, 0.0f);
				m_PBRLightShadingFramebuffer[flag]->Unbind();
			}
		}
		
		m_DeferredShadingFramebuffer->Bind();
		{
			Renderer::Clear();

			// Clear entity ID attachment to -1
			m_DeferredShadingFramebuffer->ClearAttachmentInt(1, -1);// 把EntityID数据置-1

			m_ActiveScene->SetRenderType(RenderType::PBRDEFERRED_SHADING);

			Renderer::GetShaderLibrary()->Get("PBRDeferredShading")->Bind();
			uint32_t positionTextureID    = m_GBufferFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t normalTextureID      = m_GBufferFramebuffer->GetColorAttachmentRendererID(1);
			uint32_t albedoTextureID      = m_GBufferFramebuffer->GetColorAttachmentRendererID(2);
			uint32_t roughnessAOTextureID = m_GBufferFramebuffer->GetColorAttachmentRendererID(3);
			uint32_t entityIDTextureID    = m_GBufferFramebuffer->GetColorAttachmentRendererID(4);
			uint32_t ssaoColorBufferBlur  = m_SSAOBlurFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t LoTextureID          = m_PBRLightShadingFramebuffer[!flag]->GetColorAttachmentRendererID(0);

			Texture::Bind(positionTextureID, 0);
			Texture::Bind(normalTextureID, 1);
			Texture::Bind(albedoTextureID, 2);
			Texture::Bind(roughnessAOTextureID, 3);
			Texture::Bind(entityIDTextureID, 4);
			Texture::Bind(ssaoColorBufferBlur, 5);
			Texture::Bind(LoTextureID, 6);
			Texture::Bind(m_IrradianceMap->GetRendererID(), 7);
			Texture::Bind(m_PrefilterMap->GetRendererID(), 8);
			Texture::Bind(m_BRDFLUT->GetRendererID(), 9);

			//Renderer::SetDepthTest(false);
			FullQuad::DrawIndexed();
			//Renderer::SetDepthTest(true);


			m_ActiveScene->SetRenderType(RenderType::NORMAL);
			//RenderScene();
			m_ActiveScene->SetRenderType(RenderType::SKYBOX);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);

			// 覆盖层
			//OnOverlayRender();

			m_DeferredShadingFramebuffer->Unbind();
		}
	}

	void SceneRenderer::DeferredShading()
	{
		m_DeferredShadingFramebuffer->BlitDepthFramebuffer(
			m_GBufferFramebuffer->GetRendererID(), m_DeferredShadingFramebuffer->GetRendererID(),
			0, 0, m_GBufferFramebuffer->GetSpecification().Width, m_GBufferFramebuffer->GetSpecification().Height,
			0, 0, m_DeferredShadingFramebuffer->GetSpecification().Width, m_DeferredShadingFramebuffer->GetSpecification().Height
		);

		if (!m_SSAOSwitch)
		{
			m_SSAOBlurFramebuffer->Bind();
			m_SSAOBlurFramebuffer->ClearAttachmentFloat(0, 1.0f);
			m_SSAOBlurFramebuffer->Unbind();
		}

		bool flag = false;
		uint32_t directionalLightSize = m_ActiveScene->GetDirectionalLightEntity() == nullptr ? 0 : 1;
		uint32_t pointLightSize = m_ActiveScene->GetPointLightEntities().size();
		uint32_t spotLightSize = m_ActiveScene->GetSpotLightEntities().size();
		
		m_LightShadingFramebuffer[!flag]->Bind();
		{
			Renderer::Clear(0.0f, 0.0f, 0.0f, 0.0f);
			m_LightShadingFramebuffer[!flag]->Unbind();
		}
		if (glm::max(directionalLightSize, pointLightSize, spotLightSize) > 0)
		{
			for (uint32_t i = 0; i < glm::max(directionalLightSize, pointLightSize, spotLightSize); i++)
			{
				m_ActiveScene->UpdateLight(i);
				if(directionalLightSize > 0)
				    DirectionalShadow();
				if (pointLightSize > i)
				    PointShadow();
				if (spotLightSize > i)
				    SpotShadow();
				m_LightShadingFramebuffer[flag]->Bind();
				{
					Renderer::Clear();
					m_ActiveScene->SetRenderType(RenderType::LIGHT_SHADING);
					Renderer::GetShaderLibrary()->Get("LightShading")->Bind();
					uint32_t positionTextureID    = m_GBufferFramebuffer->GetColorAttachmentRendererID(0);
					uint32_t normalTextureID      = m_GBufferFramebuffer->GetColorAttachmentRendererID(1);
					uint32_t albedoTextureID      = m_GBufferFramebuffer->GetColorAttachmentRendererID(2);
					uint32_t ambientTextureID     = m_LightShadingFramebuffer[!flag]->GetColorAttachmentRendererID(0);
					uint32_t diffuseTextureID     = m_LightShadingFramebuffer[!flag]->GetColorAttachmentRendererID(1);
					uint32_t specularTextureID    = m_LightShadingFramebuffer[!flag]->GetColorAttachmentRendererID(2);
					uint32_t directionalShadowTextureID = m_DirectionalDepthMapFramebuffer->GetDepthAttachmentRendererID();
					uint32_t pointShadowTextureID = m_PointDepthMapFramebuffer->GetDepthAttachmentRendererID();
					uint32_t spotShadowTextureID  = m_SpotDepthMapFramebuffer->GetDepthAttachmentRendererID();
					Texture::Bind(positionTextureID, 0);
					Texture::Bind(normalTextureID, 1);
					Texture::Bind(albedoTextureID, 2);
					Texture::Bind(ambientTextureID, 3);
					Texture::Bind(diffuseTextureID, 4);
					Texture::Bind(specularTextureID, 5);
					Texture::Bind(directionalShadowTextureID, 6);
					Texture::Bind(pointShadowTextureID, 7);
					Texture::Bind(spotShadowTextureID, 8);
					FullQuad::DrawIndexed();
					m_ActiveScene->SetRenderType(RenderType::NORMAL);
					flag = !flag;
					m_LightShadingFramebuffer[flag]->Unbind();
				}
			}
		}
		else
		{
			m_LightShadingFramebuffer[flag]->Bind();
			{
				Renderer::Clear(0.0f, 0.0f, 0.0f, 0.0f);
				m_LightShadingFramebuffer[flag]->Unbind();
			}
		}

		m_DeferredShadingFramebuffer->Bind();
		{
			Renderer::Clear();

			// Clear entity ID attachment to -1
			m_DeferredShadingFramebuffer->ClearAttachmentInt(1, -1);// 把EntityID数据置-1

			m_ActiveScene->SetRenderType(RenderType::DEFERRED_SHADING);

			Renderer::GetShaderLibrary()->Get("DeferredShading")->Bind();
			uint32_t positionTextureID    = m_GBufferFramebuffer->GetColorAttachmentRendererID(0);
			uint32_t normalTextureID      = m_GBufferFramebuffer->GetColorAttachmentRendererID(1);
			uint32_t albedoTextureID      = m_GBufferFramebuffer->GetColorAttachmentRendererID(2);
			uint32_t entityIDTextureID = m_GBufferFramebuffer->GetColorAttachmentRendererID(3);
			uint32_t ssaoColorBufferBlur  = m_SSAOBlurFramebuffer->GetColorAttachmentRendererID(0);

			uint32_t ambientTextureID   = m_LightShadingFramebuffer[!flag]->GetColorAttachmentRendererID(0);
			uint32_t diffuseTextureID   = m_LightShadingFramebuffer[!flag]->GetColorAttachmentRendererID(1);
			uint32_t specularTextureID  = m_LightShadingFramebuffer[!flag]->GetColorAttachmentRendererID(2);

			Texture::Bind(positionTextureID, 0);
			Texture::Bind(normalTextureID, 1);
			Texture::Bind(albedoTextureID, 2);
			Texture::Bind(entityIDTextureID, 3);
			Texture::Bind(ssaoColorBufferBlur, 4);
			Texture::Bind(ambientTextureID, 5);
			Texture::Bind(diffuseTextureID, 6);
			Texture::Bind(specularTextureID, 7);

			//Renderer::SetDepthTest(false);
			FullQuad::DrawIndexed();
			//Renderer::SetDepthTest(true);


			m_ActiveScene->SetRenderType(RenderType::NORMAL);
			//RenderScene();
			m_ActiveScene->SetRenderType(RenderType::SKYBOX);
			RenderScene();
			m_ActiveScene->SetRenderType(RenderType::NORMAL);

			// 覆盖层
			//OnOverlayRender();

			m_DeferredShadingFramebuffer->Unbind();
		}
	}

	void SceneRenderer::HDR()
	{

		Renderer::SetDepthTest(false);
		bool horizontal = true, first_iteration = true;
		uint32_t amount = 10;
		Renderer::GetShaderLibrary()->Get("Blur")->Bind();
		m_BlurFramebuffer[horizontal]->Bind();
		Renderer::Clear();
		m_BlurFramebuffer[!horizontal]->Bind();
		Renderer::Clear();
		for (uint32_t i = 0; i < amount; i++)
		{
			m_BlurFramebuffer[horizontal]->Bind();
			UniformBufferManager::GetUniformBuffer("Blur")->SetData(&horizontal, sizeof(float));
			uint32_t bloomTextureID = m_DeferredShadingFramebuffer->GetColorAttachmentRendererID(2);
			Texture::Bind(first_iteration ? bloomTextureID : m_BlurFramebuffer[!horizontal]->GetColorAttachmentRendererID(0), 0);
			FullQuad::DrawIndexed();
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		m_BlurFramebuffer[0]->Unbind();
		Renderer::SetDepthTest(true);



		UniformBufferManager::GetUniformBuffer("Exposure")->SetData(&m_Exposure, sizeof(float));
		UniformBufferManager::GetUniformBuffer("Exposure")->SetData(&m_Bloom, sizeof(bool), sizeof(float));
		m_DeferredShadingFramebuffer->Bind();
		{
			Renderer::GetShaderLibrary()->Get("HDR")->Bind();
			uint32_t hdrTextureID = m_DeferredShadingFramebuffer->GetColorAttachmentRendererID(0);
			Texture::Bind(hdrTextureID, 0);
			uint32_t blurTextureID = m_BlurFramebuffer[!horizontal]->GetColorAttachmentRendererID(0);
			Texture::Bind(blurTextureID, 1);
			Renderer::SetDepthTest(false);
			FullQuad::DrawIndexed();
			Renderer::SetDepthTest(true);


			m_DeferredShadingFramebuffer->Unbind();

		}

	}


	void SceneRenderer::RenderScene()
    {
        switch (m_SceneState)
        {
            case SceneState::Edit:
                m_ActiveScene->OnRenderEditor(m_Timestep, m_EditorCamera);
                break;
            case SceneState::Simulate:
                m_ActiveScene->OnRenderSimulation(m_Timestep, m_EditorCamera);
                break;
            case SceneState::Play:
                m_ActiveScene->OnRenderRuntime(m_Timestep);
                break;
        }
    }

}