#pragma once

#include "Volcano.h"
#include "Panel/ContentBrowserPanel.h"

namespace Volcano
{
	class VolcanoEditorLayer : public Volcano::Layer
	{
	public:
		VolcanoEditorLayer();
		virtual ~VolcanoEditorLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate() override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

	private:
		void NewProject(std::filesystem::path newProjectAbsolutePath, const std::string projectName);
		void OpenProject(const std::filesystem::path& projectFileAbsolutePath);
		bool OpenProject();
		void SaveProject();

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& sceneFileAbsolutePath);
		void SaveScene();
		void SaveSceneAs();

		void SerializeScene(Ref<Scene> scene, const std::filesystem::path& sceneFileAbsolutePath);

		void OnScenePlay();
		void OnSceneSimulate();
		void OnScenePause();
		void OnSceneStop();

		void OnDuplicateEntity();

		void ImGuiPlayButton();
		void OnOverlayRender();


		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void ClearConsole();
	private:

		EditorCamera m_EditorCamera;
		Ref<Entity> m_HoveredEntity;

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];
		bool m_ViewportFocused = false, m_ViewportHovered = false;

		int m_GizmoType = -1;
		bool m_ShowPhysicsColliders = false;

		// 是否新建项目
		bool m_NewProject = false;
		bool m_ProjectLoaded = false;

		// 活动场景
		Ref<Scene> m_ActiveScene;
		// 编辑器场景
		Ref<Scene> m_EditorScene;
		// 用于切换到临时场景时暂存m_EditorScene
		Ref<Scene> m_BackupScene;
		// 编辑器场景文件路径
		std::filesystem::path m_EditorSceneFileAbsolutePath;

		SceneState m_SceneState = SceneState::Edit;

		// Play
		bool m_Play = false;

		// Editor resources
		Ref<Texture2D> m_IconPlay, m_IconPause, m_IconStep, m_IconStop, m_IconSimulate;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		Scope<ContentBrowserPanel> m_ContentBrowserPanel;

		bool m_ViewportTempEnabled = false;

		// 性能分析
		struct ProfileResult
		{
			const char* Name;
			float Time;
		};
		std::vector<ProfileResult> m_ProfileResults;

	};
}