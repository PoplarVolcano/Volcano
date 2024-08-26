#pragma once

#include "Volcano.h"
#include <Volcano/Renderer/OrthographicCameraController.h>
#include <ParticleSystem.h>
#include <Panels/SceneHierarchyPanel.h>
#include <Panels/ContentBrowserPanel.h>
#include "Volcano/Renderer/EditorCamera.h"

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
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void OnOverlayRender();

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene();
		void SaveSceneAs();

		void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneSimulate();
		void OnScenePause();
		void OnSceneStop();

		void OnDuplicateEntity();

		// UI Panels
		void UI_Toolbar();
	private:
		OrthographicCameraController m_CameraController;

		Ref<VertexArray> m_VertexArray;
		Ref<Texture2D> m_Texture, m_AlterTexture;
		Ref<Texture2D> m_SpriteSheet;
		Ref<SubTexture2D> m_TextureStairs, m_TextureTree;
		Ref<Framebuffer> m_Framebuffer;

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];
		bool m_ViewportFocused = false, m_ViewportHovered = false;

		// 活动场景
		Ref<Scene> m_ActiveScene;
		// 编辑器场景
		Ref<Scene> m_EditorScene;
		// 编辑器场景文件路径
		std::filesystem::path m_EditorScenePath;

		Entity m_SquareEntity;
		Entity m_CameraEntity;
		Entity m_SecondCamera;
		EditorCamera m_EditorCamera;

		Entity m_HoveredEntity;

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

		bool m_ShowPhysicsColliders = false;

		enum class SceneState
		{
			Edit = 0, Play = 1, Simulate = 2
		};
		SceneState m_SceneState = SceneState::Edit;
		SceneState m_ActiveSceneState = SceneState::Edit;
		
		// Editor resources
		Ref<Texture2D> m_IconPlay, m_IconPause, m_IconStop, m_IconSimulate;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
	};
}