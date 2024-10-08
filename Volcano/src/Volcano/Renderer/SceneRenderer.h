#pragma once
#include "Volcano/Scene/Scene.h"
#include "Volcano/Renderer/Framebuffer.h"

namespace Volcano {

	class SceneRenderer
	{
	public:
		static void Init();

		static void SetViewportSize(uint32_t width, uint32_t height);

		static void BeginScene(Ref<Scene> scene, Timestep ts, SceneState sceneState, EditorCamera& camera);
		static void EndScene();

		static void DirectionalShadow();
		static void PointShadow();
		static void SpotShadow();
		static void GBuffer();
		static void SSAO();
		static void DeferredShading();
		static void HDR();


		static Ref<Framebuffer> GetDirectionalDepthMapFramebuffer() { return m_DirectionalDepthMapFramebuffer; }
		static Ref<Framebuffer> GetPointDepthMapFramebuffer() { return m_PointDepthMapFramebuffer; }
		static Ref<Framebuffer> GetSpotDepthMapFramebuffer() { return m_SpotDepthMapFramebuffer; }
		static Ref<Framebuffer> GetGBufferFramebuffer() { return m_GBufferFramebuffer; }
		static Ref<Framebuffer> GetSSAOFramebuffer() { return m_SSAOFramebuffer; }
		static Ref<Framebuffer> GetSSAOBlurFramebuffer() { return m_SSAOBlurFramebuffer; }
		static Ref<Framebuffer> GetDeferredShadingFramebuffer() { return m_DeferredShadingFramebuffer; }
		static Ref<Framebuffer> GetHDRFramebuffer() { return m_HDRFramebuffer; }
		static int* GetKernelSize() { return &m_KernelSize; }
		static float* GetRadius() { return &m_Radius; }
		static float* GetBias() { return &m_Bias; }
		static float* GetPower() { return &m_Power; }
		static float* GetExposure() { return &m_Exposure; }
		static bool* GetBloom() { return &m_Bloom; }
		static bool* GetSSAOSwitch() { return &m_SSAOSwitch; }


		static Ref<Scene> GetActiveScene() { return m_ActiveScene; }
		static void RenderScene();
	private:
		static Ref<Framebuffer> m_DirectionalDepthMapFramebuffer;
		static Ref<Framebuffer> m_PointDepthMapFramebuffer;
		static Ref<Framebuffer> m_SpotDepthMapFramebuffer;
		static Ref<Framebuffer> m_GBufferFramebuffer;
		static Ref<Framebuffer> m_DeferredShadingFramebuffer;
		static Ref<Framebuffer> m_SSAOFramebuffer;
		static Ref<Framebuffer> m_SSAOBlurFramebuffer;
		static bool m_SSAOSwitch;
		static Ref<Texture2D>   m_NoiseTexture;
		static int   m_KernelSize;
		static float m_Radius;
		static float m_Bias;
		static float m_Power;
		static Ref<Framebuffer> m_BlurFramebuffer[2];
		static Ref<Framebuffer> m_HDRFramebuffer;
		static float m_Exposure;
		static bool  m_Bloom;
		static Ref<Framebuffer> m_PBRFramebuffer;
		static Ref<Texture2D>   m_EquirectangularMap;
		static Ref<TextureCube> m_EnvCubemap;
		static Ref<TextureCube> m_IrradianceMap;
		static Ref<TextureCube> m_PrefilterMap;
		static Ref<Texture2D>   m_BRDFLUT;


		static Ref<Scene> m_ActiveScene;
		static Timestep m_Timestep;
		static SceneState m_SceneState;
		static EditorCamera m_EditorCamera;
	};


}