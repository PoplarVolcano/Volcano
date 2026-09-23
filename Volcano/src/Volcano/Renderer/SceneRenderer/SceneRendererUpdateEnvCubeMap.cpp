#include "volpch.h"
#include "Volcano/Renderer/SceneRenderer.h"

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

	void SceneRenderer::UpdateEnvCubeMap(HDRComponent* component)
	{
		if (component->equirectangularMapKey.key == "")
			return;

		component->envCubeMap = TextureCube::Create(512, 512, TextureInternalFormat::RGB16F, TextureDataFormat::RGB);
		auto& equirectangularMapKey = component->equirectangularMapKey;
		auto equirectangularMap = Texture::GetTextureLibrary()->Get(equirectangularMapKey.key, equirectangularMapKey.keyHash, equirectangularMapKey.flip, equirectangularMapKey.internalFormat);
		
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
		for (uint32_t i = 0; i < 6; i++)
			captureViewProjections[i] = captureProjection * captureViewProjections[i];

		Renderer::SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		m_CaptureFramebuffer->Resize(512, 512);

		// 等距柱状投影图映射为立方体贴图
		m_CaptureFramebuffer->Bind();
		{
			Renderer::GetShaderLibrary()->Get("EquirectangularToEnvCubeMap")->Bind();
			equirectangularMap->Bind();

			for (uint32_t i = 0; i < 6; ++i)
			{
				UniformBufferManager::GetUniformBuffer("TemporaryMat4")->SetData(&captureViewProjections[i], 4 * 4 * sizeof(float));
				// 每一轮循环将envCubeMap的一个面绑定到0号颜色附件，然后渲染它
				m_CaptureFramebuffer->SetColorAttachment(component->envCubeMap, TextureType(uint32_t(TextureType::TEXTURE_CUBE_MAP_POSITIVE_X) + i));
				
				Renderer::Clear();

				// 取消面剔除
				RendererAPI::SetCullFaceEnable(false);
				Ref<Skybox> skybox = Mesh::GetMeshLibrary()->Get<Skybox>(MeshType::Skybox)->mesh;
				skybox->DrawSkybox();
				RendererAPI::SetCullFaceEnable(true);

			}
			m_CaptureFramebuffer->Unbind();
		}

		component->irradianceMap = TextureCube::Create(32, 32, TextureInternalFormat::RGB16F, TextureDataFormat::RGB);
		m_CaptureFramebuffer->Resize(32, 32);
	
		m_CaptureFramebuffer->Bind();
		{
			Renderer::GetShaderLibrary()->Get("EnvCubeMapToIrradianceMap")->Bind();
			component->envCubeMap->Bind();

			for (uint32_t i = 0; i < 6; ++i)
			{
				UniformBufferManager::GetUniformBuffer("TemporaryMat4")->SetData(&captureViewProjections[i], 4 * 4 * sizeof(float));
				m_CaptureFramebuffer->SetColorAttachment(component->irradianceMap, TextureType(uint32_t(TextureType::TEXTURE_CUBE_MAP_POSITIVE_X) + i));
				
				Renderer::Clear();

				RendererAPI::SetCullFaceEnable(false);
				Ref<Skybox> skybox = Mesh::GetMeshLibrary()->Get<Skybox>(MeshType::Skybox)->mesh;
				skybox->DrawSkybox();
				RendererAPI::SetCullFaceEnable(true);
			}
			m_CaptureFramebuffer->Unbind();
		}


		// 基础 mip 级别的分辨率是每面 128×128，对于大多数反射来说可能已经足够了，
		// 但如果场景里有大量光滑材料（如汽车上的反射），可能需要提高分辨率。
		uint32_t prefilterMapWidth = 128;
		uint32_t prefilterMapHeight = 128;

		// 因为我们计划采样 prefilterMap 的 mipmap，所以需要确保将其缩小过滤器设置为 GL_LINEAR_MIPMAP_LINEAR 以启用三线性过滤。
		component->prefilterMap = TextureCube::Create(
			prefilterMapWidth,
			prefilterMapHeight,
			TextureInternalFormat::RGB16F,
			TextureDataFormat::RGB,
			TextureWrap::CLAMP_TO_EDGE,
			TextureFilter::LINEAR_MIPMAP_LINEAR,
			TextureFilter::LINEAR
			);

		// 预滤波环境贴图，随着粗糙度不同应用不同的mipmap
		// 在 5 个 mipmap 级别中存储 5 个不同粗糙度值的预卷积结果
		uint32_t maxMipLevels = 5;
		for (uint32_t mip = 0; mip < maxMipLevels; ++mip)
		{
			// 根据不同的mipmap等级重新设置帧缓冲尺寸
			prefilterMapWidth = static_cast<uint32_t>(128 * std::pow(0.5, mip));
			prefilterMapHeight = static_cast<uint32_t>(128 * std::pow(0.5, mip));
			m_CaptureFramebuffer->Resize(prefilterMapWidth, prefilterMapHeight);
			m_CaptureFramebuffer->Bind();
			{
				Renderer::GetShaderLibrary()->Get("EnvCubeMapToPrefilterMap")->Bind();
				component->envCubeMap->Bind();
				float roughness = (float)mip / (float)(maxMipLevels - 1);
				UniformBufferManager::GetUniformBuffer("TemporaryFloat")->SetData(&roughness, sizeof(float));
				for (uint32_t i = 0; i < 6; ++i)
				{
					UniformBufferManager::GetUniformBuffer("TemporaryMat4")->SetData(&captureViewProjections[i], 4 * 4 * sizeof(float));
					m_CaptureFramebuffer->SetColorAttachment(component->prefilterMap, TextureType(uint32_t(TextureType::TEXTURE_CUBE_MAP_POSITIVE_X) + i), 0, mip);
					
					Renderer::Clear();

					RendererAPI::SetCullFaceEnable(false);
					Ref<Skybox> skybox = Mesh::GetMeshLibrary()->Get<Skybox>(MeshType::Skybox)->mesh;
					skybox->DrawSkybox();
					RendererAPI::SetCullFaceEnable(true);
				}
				m_CaptureFramebuffer->Unbind();
			}
		}

	}
}