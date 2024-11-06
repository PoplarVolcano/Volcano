#pragma once

#include "Volcano/Core/Base.h"
#include "Volcano/Scene/Scene.h"
#include "Volcano/Scene/Entity.h"

namespace Volcano {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(Ref<Scene>& scene);

		void SetContext(Ref<Scene>& scene);
		Ref<Scene>& GetContext() { return m_Context; }

		void OnImGuiRender();
		Ref<Entity> GetSelectedEntity() const { return m_SelectionContext; }
		void SetSelectedEntity(Ref<Entity> entity);
	private:
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);

		void DrawEntityNode(Ref<Entity> entity);
		void DrawPrefabCombo(std::filesystem::path folder_path, Ref<Entity> entity, bool& stopDraw);
		void DrawComponents(Ref<Entity> entity);
	private:
		Ref<Scene> m_Context;
		Ref<Entity> m_SelectionContext;
	};
}