#pragma once

#include <filesystem>
#include "SceneHierarchyPanel.h"

namespace Volcano {

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();
		void SetSceneHierarchyPanel(SceneHierarchyPanel* sceneHierarchyPanel) { m_SceneHierarchyPanel = sceneHierarchyPanel; }
		void SetLayer(Layer* layer);
	private:
		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;

		SceneHierarchyPanel* m_SceneHierarchyPanel;
		Layer* m_Layer;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;
	};
}