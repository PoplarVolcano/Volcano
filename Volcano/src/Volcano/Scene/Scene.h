#pragma once

#include "entt.hpp"
#include "Volcano/Core/UUID.h"
#include "Volcano/Core/Timestep.h"
#include "Volcano/Renderer/EditorCamera.h"

class b2World;

namespace Volcano {

	enum class SceneState
	{
		Edit = 0, Play = 1, Simulate = 2
	};

	enum class RenderType
	{
		NORMAL,
		SHADOW_DIRECTIONALLIGHT,
		SHADOW_POINTLIGHT,
		SHADOW_SPOTLIGHT,
		G_BUFFER,
		LIGHT_SHADING,
		DEFERRED_SHADING,
		SKYBOX
	};

	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		void InitializeUniform();

		static Ref<Scene> Copy(Ref<Scene> other);
		Ref<Entity> DuplicateEntity(Ref<Entity> entity);

		Ref<Entity> FindEntityByName(std::string_view name);
		Ref<Entity> GetEntityByUUID(UUID uuid);

		static std::string NewName(std::map<std::string, Ref<Entity>> entityNameMap, std::string name);
		Ref<Entity> CreateEntity(const std::string& name = std::string(), Ref<Entity> entity = nullptr);
		Ref<Entity> CreateEntityWithUUID(UUID uuid, const std::string& name = std::string(), Ref<Entity> entity = nullptr);
		
		void DestroyEntityChild(Ref<Entity> entity);
		void DestroyEntity(Ref<Entity> entity);

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

		Ref<Entity> GetPrimaryCameraEntity();
		Ref<Entity> GetDirectionalLightEntity();
		std::vector<Ref<Entity>> GetPointLightEntities();
		std::vector<Ref<Entity>> GetSpotLightEntities();

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

		entt::registry& GetRegistry() { return m_Registry; }
		std::unordered_map<UUID, Ref<Entity>>& GetEntityIDMap() { return m_EntityIDMap; }
		std::unordered_map<entt::entity, Ref<Entity>>& GetEntityEnttMap() { return m_EntityEnttMap; }
		std::map<std::string, Ref<Entity>>& GetEntityNameMap() { return m_EntityNameMap; }
		

		void UpdateLight(uint32_t i = 0);
	private:
		template<typename T>
		void OnComponentAdded(Entity& entity, T& component);

		void OnPhysics2DStart();
		void OnPhysics2DStop();

		void UpdateCameraData(Camera& camera, glm::mat4 transform, glm::vec3 position);
		void UpdateScene();
		void RenderScene(Camera& camera, const glm::mat4& transform, const glm::vec3& position, const glm::vec3& direction);

	private:
		entt::registry m_Registry;                                     // 注册表，存所有entt::entity
		std::unordered_map<UUID, Ref<Entity>> m_EntityIDMap;           // ID表，存id和Ref<Entity>的对应关系，包括子节点Entity
		std::unordered_map<entt::entity, Ref<Entity>> m_EntityEnttMap; // Entt表，存entt::entity和Ref<Entity>的对应关系，包括子节点Entity
		std::map<std::string, Ref<Entity>> m_EntityNameMap;            // Name表，存Ref<Entity>，不包括子节点Entity

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		bool m_IsRunning = false;
		bool m_IsPaused = false;
		int m_StepFrames = 0;


		b2World* m_PhysicsWorld = nullptr;

		RenderType m_RenderType = RenderType::NORMAL;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}