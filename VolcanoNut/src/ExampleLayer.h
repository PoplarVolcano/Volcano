#pragma once

#include "Volcano.h"
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
		bool OnMouseMoved(MouseMovedEvent& e);
		//bool OnMouseMoved(MouseMovedEvent& e);

		void OnOverlayRender();

		void NewProject();
		void OpenProject(const std::filesystem::path& path);
		bool OpenProject();
		void SaveProject();

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene();
		void SaveSceneAs();
		void RenderScene(Timestep ts);

		void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneSimulate();
		void OnScenePause();
		void OnSceneStop();

		void OnDuplicateEntity();

		// UI Panels
		void UI_Toolbar();
	private:
		Ref<VertexArray> m_VertexArray;
		Ref<Texture2D> m_Texture, m_AlterTexture;
		Ref<Texture2D> m_SpriteSheet;
		Ref<SubTexture2D> m_TextureStairs, m_TextureTree;
		Ref<Framebuffer> m_Framebuffer;
		Ref<Framebuffer> m_DirectionalDepthMapFramebuffer;
		Ref<Framebuffer> m_PointDepthMapFramebuffer;
		Ref<Framebuffer> m_SpotDepthMapFramebuffer;

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];
		bool m_ViewportFocused = false, m_ViewportHovered = false;

		// �����
		Ref<Scene> m_ActiveScene;
		// �༭������
		Ref<Scene> m_EditorScene;
		// �༭�������ļ�·��
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
		
		// Editor resources
		Ref<Texture2D> m_IconPlay, m_IconPause, m_IconStep, m_IconStop, m_IconSimulate;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		Scope<ContentBrowserPanel> m_ContentBrowserPanel;

		struct WindowVertex 
		{
			glm::vec3 Position;
			glm::vec2 TextureCoords;
		};
		WindowVertex m_WindowVertex[4];
		Ref<VertexArray> windowVa;
		Ref<Shader> m_WindowShader;
	};
}