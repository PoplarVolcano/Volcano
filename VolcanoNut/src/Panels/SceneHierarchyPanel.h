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

		void SetContext(const Ref<Scene>& scene);

		void OnImGuiRender();
		Ref<Entity> GetSelectedEntity() const { return m_SelectionContext; }
		void SetSelectedEntity(Ref<Entity> entity);
	private:
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);

		void DrawEntityNode(Ref<Entity> entity);
		void DrawComponents(Ref<Entity> entity);
	private:
		Ref<Scene> m_Context;
		Ref<Entity> m_SelectionContext;
	};
}