#pragma once

#include "Volcano.h"

namespace Volcano {

	// 场景层级面板
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(Ref<Scene>& scene);

		void SetScene(Ref<Scene>& scene);
		Ref<Scene>& GetScene() { return m_Scene; }

		void OnImGuiRender();
		Ref<Entity> GetSelectedEntity() const { return m_SelectionContext; }
		void SetSelectedEntity(Ref<Entity> entity);
	private:
		template<typename Func = std::nullptr_t>
		static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f, Func&& func = nullptr);

		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);

		void DrawEntityNode(Ref<Entity> entity);
		void DrawComponents(Ref<Entity> entity);

		void DrawTagComponent(Ref<Entity> entity);
		void DrawCameraComponent(CameraComponent& component);
		void DrawLightComponent(LightComponent& component);
		void DrawMeshRendererComponent(MeshRendererComponent& component);
		void DrawScriptComponent(Ref<Entity> entity, ScriptComponent& component);
		void DrawParticleSystemComponent(ParticleSystemComponent& component);
		void DrawSkyboxComponent(SkyboxComponent& component);
		void DrawHDRComponent(HDRComponent& component);
	private:
		Ref<Scene> m_Scene;
		Ref<Entity> m_SelectionContext;
	};
}
#include "SceneHierarchyPanelDrawVec3Control.h"