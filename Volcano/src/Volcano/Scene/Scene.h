#pragma once

#include "entt.hpp"
#include "Volcano/Core/UUID.h"
#include "Volcano/Core/Timestep.h"
#include "Volcano/Renderer/EditorCamera.h"

class b2World;

namespace Volcano {

	enum class RenderType
	{
		NORMAL,
		SHADOW_DIRECTIONALLIGHT,
		SHADOW_POINTLIGHT,
		SHADOW_SPOTLIGHT
	};

	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		void InitializeUniform();

		static Ref<Scene> Copy(Ref<Scene> other);
		Entity DuplicateEntity(Entity entity);

		Entity FindEntityByName(std::string_view name);
		Entity GetEntityByUUID(UUID uuid);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnUpdateRuntime(Timestep ts);
		void OnRenderRuntime(Timestep ts);
		void OnUpdateSimulation(Timestep ts, EditorCamera& camera);
		void OnRenderSimulation(Timestep ts, EditorCamera& camera);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnRenderEditor(Timestep ts, EditorCamera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void Physics(Timestep ts);

		Entity GetPrimaryCameraEntity();
		Entity GetDirectionalLightEntity();
		Entity GetPointLightEntity();
		Entity GetSpotLightEntity();

		bool IsRunning() const { return m_IsRunning; }
		bool IsPaused() const { return m_IsPaused; }

		void SetRunning(bool running) { m_IsRunning = running; }
		void SetPaused(bool paused) { m_IsPaused = paused; }

		void SetRenderType(RenderType type) { m_RenderType = type; }

		void Step(int frames = 1);

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		void OnPhysics2DStart();
		void OnPhysics2DStop();

		void RenderScene(Camera& camera, const glm::mat4& transform, const glm::vec3& position, const glm::vec3& direction);
		void UpdateLight();
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		bool m_IsRunning = false;
		bool m_IsPaused = false;
		int m_StepFrames = 0;

		b2World* m_PhysicsWorld = nullptr;

		std::unordered_map<UUID, entt::entity> m_EntityMap;

		RenderType m_RenderType = RenderType::NORMAL;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}