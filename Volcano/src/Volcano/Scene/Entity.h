#pragma once

#include "Scene.h"
#include "Volcano/Core/UUID.h"
#include "Volcano/Scene/Components.h"

#include "entt.hpp"

namespace  Volcano {

	class Entity
	{
	public:
		static Ref<Entity> Create(Scene& scene, UUID uuid, const std::string& name);

		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

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

		UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		const std::string& GetName() { return GetComponent<TagComponent>().Tag; }
		const entt::entity& GetEntityHandle() { return m_EntityHandle; }

		std::map<std::string, Ref<Entity>>& GetEntityChildren() { return m_Children; }
		Scene* GetScene() { return m_Scene; }
		Entity* GetEntityParent() { return m_Parent; }

		Ref<Entity> AddEntityChild(UUID uuid, const std::string& name);
		void SetEntityParent(Entity* entity);

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

		glm::mat4 GetTransform() { return m_Transform; }
		void SetTransform(glm::mat4 transform) { m_Transform = transform; }

	private:
		Entity* m_Parent = nullptr;
		std::map<std::string, Ref<Entity>> m_Children;
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;

		glm::mat4 m_Transform;

	};
}