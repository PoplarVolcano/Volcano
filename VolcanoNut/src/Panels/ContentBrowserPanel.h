#pragma once

#include <filesystem>
#include <Volcano/Renderer/Texture.h>
#include "SceneHierarchyPanel.h"

namespace Volcano {

	class ExampleLayer;

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();
		void SetSceneHierarchyPanel(SceneHierarchyPanel* sceneHierarchyPanel) { m_SceneHierarchyPanel = sceneHierarchyPanel; }
		void SetExampleLayer(ExampleLayer* exampleLayer);
	private:
		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;

		SceneHierarchyPanel* m_SceneHierarchyPanel;
		ExampleLayer* m_ExampleLayer;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;
	};
}