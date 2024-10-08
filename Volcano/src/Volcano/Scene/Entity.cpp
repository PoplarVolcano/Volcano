#include "volpch.h"
#include "Entity.h"

namespace Volcano {

	Ref<Entity> Entity::Create(Scene& scene, UUID uuid, const std::string& name)
	{
		Ref<Entity> entity = std::make_shared<Entity>(scene.m_Registry.create(), &scene);
		entity->AddComponent<TransformComponent>();
		entity->AddComponent<IDComponent>(uuid); // 使用实参uuid，不创建新的
		auto& tag = entity->AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		return entity;
	}

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
		
	}

	Ref<Entity> Entity::SetEntityChild(UUID uuid, const std::string& name)
	{
		std::string newName = Scene::NewName(m_Children, name);

		Ref<Entity> entity = Create(*m_Scene, uuid, newName);
		entity->SetEntityParent(this);
		m_Children[newName] = entity;

		return entity;
	}
	void Entity::SetEntityParent(Entity* entity)
	{
		m_Parent = entity;
	}
}
