#pragma once

#include "entt.hpp"
#include "Volcano/Core/UUID.h"
#include "Volcano/Core/Timestep.h"
#include "Volcano/Renderer/EditorCamera.h"
#include "Volcano/Physics/Physic/b3_WorldCallbacks.h"

class b2World;

namespace Volcano {

	class Scene;
	class b3_World;
	class b3_Contact;
	struct b3_Manifold;

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
		PBRLIGHT_SHADING,
		LIGHT_SHADING,
		PBRDEFERRED_SHADING,
		DEFERRED_SHADING,
		SKYBOX,
		COLLIDER
	};

	class Entity;

	class Listener : public b3_ContactListener
	{
	public:
		Listener(Scene* scene) { m_scene = scene; }
		void SetScene(Scene* scene) { m_scene = scene; }
		virtual void BeginContact(b3_Contact* contact) override;
		virtual void EndContact(b3_Contact* contact) override {}
		virtual void PreSolve(b3_Contact* contact, const b3_Manifold* oldManifold) override {}
	private:
		Scene* m_scene;
	};

	class Scene
	{
	public:
		static std::vector<Ref<Entity>>::iterator FindIteratorInList(std::string name, std::vector<Ref<Entity>>& entityList);
		static std::string NewName(std::vector<Ref<Entity>>& entityList, std::string name);
		static Ref<Scene> Copy(Ref<Scene> other);
		static Ref<Entity> Find(std::string name, std::vector<Ref<Entity>>& entityList);
		static void RemoveEntityFromList(Ref<Entity> entity, std::vector<Ref<Entity>>& entityList);
		static void UpdateEntityParent(Ref<Entity> entity, Entity* parent, Scene* targetScene = nullptr);

		Scene();
		~Scene();

		std::string GetName() { return m_Name; }
		void SetName(std::string name) { m_Name = name; }
		std::string GetFilePath() { return m_FilePath; }
		void SetFilePath(std::string filePath) { m_FilePath = filePath; }
		std::string GetPath() { return m_Path; }
		void SetPath(std::string path) { m_Path = path; }

		void InitializeUniform();

		Ref<Entity> DuplicateEntity(Ref<Entity> entity, Entity* parent = nullptr, UUID id = UUID());

		Ref<Entity> GetEntityByUUID(UUID uuid);

		Ref<Entity> CreateEntity(const std::string& name = std::string(), Entity* parent = nullptr);
		Ref<Entity> CreateEntityWithUUID(UUID uuid, const std::string& name = std::string(), Entity* parent = nullptr);
		
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
		Ref<Entity> GetPrimarySkyboxEntity();
		bool& GetShowBone() { return m_ShowBone; }

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
		std::vector<Ref<Entity>>& GetEntityList() { return m_EntityList; }
		
		void UpdateLight(uint32_t i = 0);
	private:
		template<typename T>
		void OnComponentAdded(Entity& entity, T& component);

		void OnPhysics2DStart();
		void OnPhysics2DStop();
		void OnPhysics3DStart();
		void OnPhysics3DStop();

		void UpdateCameraData(Camera& camera, glm::mat4 transform, glm::vec3 position);
		void UpdateScene(Timestep ts);
		void RenderScene(Camera& camera, const glm::mat4& transform, const glm::vec3& position, const glm::vec3& direction);

	private:
		std::string m_Name = "Untitled";
		std::string m_FilePath;
		std::string m_Path;

		entt::registry m_Registry;                                     // 注册表，存所有entt::entity
		std::unordered_map<UUID, Ref<Entity>> m_EntityIDMap;           // ID表，存id和Ref<Entity>的对应关系，包括子节点Entity
		std::unordered_map<entt::entity, Ref<Entity>> m_EntityEnttMap; // Entt表，存entt::entity和Ref<Entity>的对应关系，包括子节点Entity
		std::vector<Ref<Entity>> m_EntityList;            // Name表，存Ref<Entity>，不包括子节点Entity

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		bool m_ShowBone = false;
		bool m_IsRunning = false;
		bool m_IsPaused = false;
		int m_StepFrames = 0;


		b2World* m_PhysicsWorld = nullptr;

		b3_World* m_Physics3DWorld = nullptr;

		RenderType m_RenderType = RenderType::NORMAL;

		b3_ContactListener m_physic3DListener;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};

}