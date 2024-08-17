#pragma once

#include "Volcano.h"
#include <Volcano/Renderer/OrthographicCameraController.h>
#include <ParticleSystem.h>
#include <Panels/SceneHierarchyPanel.h>

namespace Volcano {

	class ExampleLayer : public Layer 
	{
	public:
		ExampleLayer();
		virtual ~ExampleLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;
	private:
		bool OnKeyPressed(KeyPressedEvent& e);

		void NewScene();
		void OpenScene();
		void SaveSceneAs();
	private:
		OrthographicCameraController m_CameraController;

		Ref<VertexArray> m_VertexArray;
		Ref<Texture2D> m_Texture, m_AlterTexture;
		Ref<Texture2D> m_SpriteSheet;
		Ref<SubTexture2D> m_TextureStairs, m_TextureTree;
		Ref<Framebuffer> m_Framebuffer;

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		bool m_ViewportFocused = false, m_ViewportHovered = false;

		Ref<Scene> m_ActiveScene;
		Entity m_SquareEntity;
		Entity m_CameraEntity;
		Entity m_SecondCamera;

		bool m_PrimaryCamera = true;

		struct ProfileResult
		{
			const char* Name;
			float Time;
		};
		std::vector<ProfileResult> m_ProfileResults;

		ParticleProps m_Particle;
		ParticleSystem m_ParticleSystem;

		uint32_t m_MapWidth, m_MapHeight;
		std::unordered_map<char, Ref<SubTexture2D>> s_TextureMap;
		
		int m_GizmoType = -1;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
	};
}