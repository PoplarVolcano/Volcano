#pragma once
#include "Volcano/Scene/Scene.h"
#include "Volcano/Renderer/Framebuffer.h"

namespace Volcano {
	
	class VOL_API SceneRenderer
	{
	public:
		static void Init();

		static void BeginScene(Ref<Scene> scene, SceneState sceneState, EditorCamera& camera);
		static void EndScene();

		static void PreProcessing();
		static void EntityID();
		static void LightingMode();
		static void GBuffer();
		static void MidProcessing();
		static void SSAO();
		static void DirectionalLightShadow();
		static void PointLightShadow();
		static void SpotLightShadow();
		static void DirectionalLight();
		static void PointLight();
		static void SpotLight();
		static void LightShading();
		static void PBRDirectionalLight();
		static void PBRPointLight();
		static void PBRSpotLight();
		static void PBRLightShading();
		static void DeferredShading();
		static void ForwardShading();
		static void SkyboxRender();
		static void Particle();
		static void NormalVisualization();
		static void DrawOutlines();
		static void HDRAndBloom();
		static void PostProcessing();

		static void UpdateEnvCubeMap(HDRComponent* component);

		static Ref<Scene> GetActiveScene() { return m_ActiveScene; }
		static void RenderScene();

		static Ref<Framebuffer> GetDirectionalLightDepthMapFramebuffer() { return m_DirectionalLightDepthMapFramebuffer; }
		static Ref<Framebuffer> GetPointLightDepthMapFramebuffer() { return m_PointLightDepthMapFramebuffer; }
		static Ref<Framebuffer> GetSpotLightDepthMapFramebuffer() { return m_SpotLightDepthMapFramebuffer; }
		static Ref<Framebuffer> GetMSGBufferFramebuffer() { return m_MSGBufferFramebuffer; }
		static Ref<Framebuffer> GetResolvedGBufferFramebuffer() { return m_ResolvedGBufferFramebuffer; }
		static Ref<Framebuffer> GetSSAOFramebuffer() { return m_SSAOFramebuffer; }
		static Ref<Framebuffer> GetSSAOBlurFramebuffer() { return m_SSAOBlurFramebuffer; }
		static Ref<Framebuffer> GetDeferredShadingFramebuffer() { return m_DeferredShadingFramebuffer; }
		static Ref<Framebuffer> GetLightShadingFramebuffer() { return m_LightShadingFramebuffer; }
		static Ref<Framebuffer> GetPBRLightShadingFramebuffer() { return m_PBRLightShadingFramebuffer; }
		static Ref<Framebuffer> GetPostProcessingFramebuffer() { return m_PostProcessingFramebuffer; }

		static Ref<Texture2D>   GetBRDFLUT() { return m_BRDFLUT; }
	private:
		static Ref<Scene> m_ActiveScene;
		static SceneState m_SceneState;
		static EditorCamera m_EditorCamera;

		static Ref<Framebuffer> m_DirectionalLightDepthMapFramebuffer;
		static Ref<Framebuffer> m_PointLightDepthMapFramebuffer;
		static Ref<Framebuffer> m_SpotLightDepthMapFramebuffer;
		static Ref<Framebuffer> m_MSGBufferFramebuffer;
		static Ref<Framebuffer> m_ResolvedGBufferFramebuffer;
		static Ref<Framebuffer> m_SSAOFramebuffer;
		static Ref<Framebuffer> m_SSAOBlurFramebuffer;
		static Ref<Framebuffer> m_DeferredShadingFramebuffer;
		static Ref<Framebuffer> m_LightShadingFramebuffer;
		static Ref<Framebuffer> m_PBRLightShadingFramebuffer;
		static Ref<Framebuffer> m_PostProcessingFramebuffer;
		static Ref<Framebuffer> m_GaussianBlurFramebuffer[2];
		static Ref<Framebuffer> m_EntityIDFramebuffer;
		static Ref<Framebuffer> m_LightingModeFramebuffer;

		static Ref<Framebuffer> m_CaptureFramebuffer;

		static Ref<Texture2D>   m_NoiseTexture;
		static Ref<Texture2D>   m_BRDFLUT;
	};

}