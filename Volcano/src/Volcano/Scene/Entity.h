#pragma once

#include "Scene.h"
#include "Volcano/Core/UUID.h"
#include "Volcano/Scene/Components.h"

#include "entt.hpp"

namespace  Volcano {

	class VOL_API Entity
	{
	public:
		static Ref<Entity> Create(Scene* scene, UUID uuid, const std::string& name);

		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		UUID& GetUUID() { return GetComponent<IDComponent>().ID; }
		const std::string& GetName() { return GetComponent<TagComponent>().tag; }
		void SetName(std::string name) { GetComponent<TagComponent>().tag = name; }
		const entt::entity& GetEntityHandle() { return m_EntityHandle; }

		std::vector<Ref<Entity>>& GetEntityChildrenList() { return m_Children; }
		Scene* GetScene() { return m_Scene; }
		Entity* GetEntityParent() { return m_Parent; }

		glm::mat4 GetTransform();
		glm::mat4 GetWorldTransform();
		glm::quat GetWorldRotation();

		bool& GetActive() { return m_Active; }
		void SetActive(bool active) { m_Active = active; }

		Ref<Entity> AddEntityChild(UUID uuid, const std::string& name, int index = -1);

		void SetEntityParent(Entity* entity);


		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			VOL_CORE_ASSERT(!HasComponent<T>(), "AddComponent: Entity already has component!");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		// 添加组件，若已存在则覆盖
		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			VOL_CORE_ASSERT(HasComponent<T>(), "GetComponent: Entity does not has component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.try_get<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			VOL_CORE_ASSERT(HasComponent<T>(), "RemoveComponent: Entity does not has component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		// 不能删除 TransformComponent
		template<>
		void RemoveComponent<TransformComponent>()
		{
		}

		template<>
		void RemoveComponent<ScriptComponent>();

		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }
		bool operator==(const Entity& other) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}

	private:
		bool m_Active = true;
		Entity* m_Parent = nullptr;
		std::vector<Ref<Entity>> m_Children;
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	};
}