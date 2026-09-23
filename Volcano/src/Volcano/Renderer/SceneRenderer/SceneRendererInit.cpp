#include "volpch.h"
#include "Volcano/Renderer/SceneRenderer.h"

#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Renderer/RendererItem/FullQuad.h"
#include "Volcano/Renderer/UniformBuffer.h"
#include "Volcano/Renderer/Texture.h"

namespace Volcano {

	Ref<Framebuffer> SceneRenderer::m_DirectionalLightDepthMapFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_PointLightDepthMapFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_SpotLightDepthMapFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_MSGBufferFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_ResolvedGBufferFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_SSAOFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_SSAOBlurFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_DeferredShadingFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_LightShadingFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_PBRLightShadingFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_PostProcessingFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_GaussianBlurFramebuffer[2];
	Ref<Framebuffer> SceneRenderer::m_EntityIDFramebuffer;
	Ref<Framebuffer> SceneRenderer::m_LightingModeFramebuffer;

	Ref<Framebuffer> SceneRenderer::m_CaptureFramebuffer;

	Ref<Texture2D>   SceneRenderer::m_NoiseTexture;
	Ref<Texture2D>   SceneRenderer::m_BRDFLUT;


	void SceneRenderer::Init()
	{
		Volcano::FramebufferSpecification fbSpec;

		fbSpec.Attachments = {
			FramebufferTextureFormat::RED_INTEGER, // EntityID缓冲
			FramebufferTextureFormat::Depth        // 深度附件，没有深度附件的话生成的图像没有深度检测
		};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 4;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_EntityIDFramebuffer = Framebuffer::Create(fbSpec);

		fbSpec.Attachments = {
			FramebufferTextureFormat::RED_INTEGER, // LightingMode缓冲
			FramebufferTextureFormat::Depth        // 深度附件，没有深度附件的话生成的图像没有深度检测
		};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 4;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_LightingModeFramebuffer = Framebuffer::Create(fbSpec);


		fbSpec.Attachments = {
			FramebufferTextureFormat::DEPTH_COMPONENT
		};
		fbSpec.Width = 4096;
		fbSpec.Height = 4096;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_DirectionalLightDepthMapFramebuffer = Framebuffer::Create(fbSpec);


		fbSpec.Attachments = {
			FramebufferTextureFormat::DEPTH_COMPONENT
		};
		fbSpec.Width = 1024;
		fbSpec.Height = 1024;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_CUBE_MAP;
		m_PointLightDepthMapFramebuffer = Framebuffer::Create(fbSpec);


		fbSpec.Attachments = {
			FramebufferTextureFormat::DEPTH_COMPONENT
		};
		fbSpec.Width = 1024;
		fbSpec.Height = 1024;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_SpotLightDepthMapFramebuffer = Framebuffer::Create(fbSpec);


		fbSpec.Attachments = {
			FramebufferTextureFormat::RGBA16F,     // 位置+深度缓冲，深度范围0.1到50.0
			FramebufferTextureFormat::RGBA8,       // 颜色+镜面缓冲Albedo
			FramebufferTextureFormat::RGBA8,       // 法线缓冲
			FramebufferTextureFormat::RGBA8,       // 粗糙度+AO缓冲
			FramebufferTextureFormat::RGBA8,       // 放射光贴图
			FramebufferTextureFormat::Depth        // 深度附件，没有深度附件的话生成的图像没有深度检测
		};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		//fbSpec.Samples = 4;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		//fbSpec.DepthType = TextureType::TEXTURE_2D_MULTISAMPLE;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_MSGBufferFramebuffer = Framebuffer::Create(fbSpec);

		fbSpec.Attachments = {
			FramebufferTextureFormat::RGBA16F,     // 位置+深度缓冲，深度范围0.1到50.0
			FramebufferTextureFormat::RGBA8,       // 颜色+镜面缓冲Albedo
			FramebufferTextureFormat::RGBA8,       // 法线缓冲
			FramebufferTextureFormat::RGBA8,       // 粗糙度+AO缓冲
			FramebufferTextureFormat::RGBA8,       // 放射光贴图
			FramebufferTextureFormat::Depth        // 深度附件，没有深度附件的话生成的图像没有深度检测
		};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_ResolvedGBufferFramebuffer = Framebuffer::Create(fbSpec);

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

		fbSpec.Attachments = {
			FramebufferTextureFormat::RGBA16F,      // 颜色缓冲
			FramebufferTextureFormat::RGBA16F,      // 亮度颜色BrightColor
			FramebufferTextureFormat::Depth         // 添加一个深度附件，默认Depth = DEPTH24STENCIL8
		};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_DeferredShadingFramebuffer = Framebuffer::Create(fbSpec);

		fbSpec.Attachments = {
			FramebufferTextureFormat::RGBA16F, // Ambient：环境光
			FramebufferTextureFormat::RGBA16F, // Diffuse：漫反射
			FramebufferTextureFormat::RGBA16F, // Specular：镜面反射
			FramebufferTextureFormat::Depth    // 添加一个深度附件，默认Depth = DEPTH24STENCIL8
		};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_LightShadingFramebuffer = Framebuffer::Create(fbSpec);

		fbSpec.Attachments = {
			FramebufferTextureFormat::RGBA16F, // L0：反射率
			FramebufferTextureFormat::Depth    // 添加一个深度附件，默认Depth = DEPTH24STENCIL8
		};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_PBRLightShadingFramebuffer = Framebuffer::Create(fbSpec);

		fbSpec.Attachments = {
			FramebufferTextureFormat::RGBA16F,      // 颜色缓冲
			FramebufferTextureFormat::RED_INTEGER,  // EntityID缓冲
			FramebufferTextureFormat::Depth         // 添加一个深度附件，默认Depth = DEPTH24STENCIL8
		};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_PostProcessingFramebuffer = Framebuffer::Create(fbSpec);

		fbSpec.Attachments = {
			FramebufferTextureFormat::RGBA16F
		};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_GaussianBlurFramebuffer[0] = Framebuffer::Create(fbSpec);
		m_GaussianBlurFramebuffer[1] = Framebuffer::Create(fbSpec);



		// Sample kernel
		// 采样核心
		// 在切线空间中以-1.0到1.0为范围变换x和y方向，并以0.0和1.0为范围变换样本的z方向(半球形的样本)。
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // 随机浮点数，[0.0, 1.0]
		std::default_random_engine generator;
		std::vector<glm::vec4> ssaoKernel;
		const uint32_t ssaoKernelSize = 64;
		for (uint32_t i = 0; i != ssaoKernelSize; i++)
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator));
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);
			float scale = float(i) / float(ssaoKernelSize);

			// 缩放样本，使其更接近原点
			scale = 0.1f + scale * scale * (1.0f - 0.1f);
			sample *= scale;
			ssaoKernel.emplace_back(glm::vec4(sample, 1.0f));
		}

		UniformBufferManager::GetUniformBuffer("Samples")->SetData(ssaoKernel.data(), ssaoKernel.size() * sizeof(glm::vec4));


		fbSpec.Attachments = {
			FramebufferTextureFormat::RGBA16F
		};
		fbSpec.Width = 512;
		fbSpec.Height = 512;
		fbSpec.Samples = 1;
		fbSpec.ColorType = TextureType::TEXTURE_2D;
		fbSpec.DepthType = TextureType::TEXTURE_2D;
		m_CaptureFramebuffer = Framebuffer::Create(fbSpec);


		// Noise texture
		// 4x4朝向切线空间平面法线的随机旋转向量数组
		std::vector<glm::vec3> ssaoNoise;
		for (uint32_t i = 0; i < 16; i++)
		{
			glm::vec3 noise(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f); // rotate around z-axis (in tangent space)
			ssaoNoise.push_back(noise);
		}

		m_NoiseTexture = Texture2D::Create(4, 4, TextureInternalFormat::RGB16F, TextureDataFormat::RGB);
		m_NoiseTexture->SetData(ssaoNoise.data(), ssaoNoise.size() * 3);


		// Epic Games 推荐16位精度浮点格式。环绕模式为 GL_CLAMP_TO_EDGE 以防止边缘采样的伪像
		m_BRDFLUT = Texture2D::Create(512, 512, TextureInternalFormat::RG16F, TextureDataFormat::RG, TextureWrap::CLAMP_TO_EDGE);

		m_CaptureFramebuffer->Bind();
		{
			RendererAPI::Clear();

			m_CaptureFramebuffer->SetColorAttachment(m_BRDFLUT, TextureType::TEXTURE_2D);

			Renderer::GetShaderLibrary()->Get("BRDF")->Bind();
			FullQuad::DrawIndexed();

			m_CaptureFramebuffer->Unbind();
		}
	}

}