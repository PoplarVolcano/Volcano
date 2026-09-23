#include "volpch.h"

#include "Entity.h"
#include "Volcano//Math/Math.h"
#include "Volcano/Scene/SceneBuffer.h"
#include "Volcano/Scripting/ScriptEngine.h"

namespace Volcano {

	Ref<Entity> Entity::Create(Scene* scene, UUID uuid, const std::string& name)
	{
		Ref<Entity> entity = std::make_shared<Entity>(scene->m_Registry.create(), scene);
		entity->AddComponent<TransformComponent>();
		entity->AddComponent<IDComponent>(uuid); // 使用实参uuid，不创建新的
		auto& Tag = entity->AddComponent<TagComponent>();
		Tag.tag = name.empty() ? "Empty Entity" : name;

		return entity;
	}

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{

	}

	glm::mat4 Entity::GetTransform()
	{
		return GetComponent<TransformComponent>().GetTransform();
	}

	glm::mat4 Entity::GetWorldTransform()
	{
		auto& entityWorldTransformMap = SceneBuffer::GetInstance().entityWorldTransformMap;
		auto it = entityWorldTransformMap.find(this);
		if (it != entityWorldTransformMap.end())
		{
			return it->second;
		}
		else
		{
			if (m_Parent != nullptr)
				return m_Parent->GetWorldTransform() * GetTransform();
			return GetTransform();
		}
	}

	glm::quat Entity::GetWorldRotation()
	{
		auto& entityWorldRotationMap = SceneBuffer::GetInstance().entityWorldRotationMap;
		auto it = entityWorldRotationMap.find(this);
		if (it != entityWorldRotationMap.end())
		{
			return it->second;
		}
		else
		{
			const auto& transform = GetComponent<TransformComponent>();
			if (m_Parent != nullptr)
				return m_Parent->GetWorldRotation() * transform.rotation;
			return transform.rotation;
		}
	}

	// 新建子节点
	Ref<Entity> Entity::AddEntityChild(UUID uuid, const std::string& name, int index)
	{
		if (index > (int)m_Children.size() || index < -1)
			return nullptr;

		std::string newName = Scene::NewName(m_Children, name);

		Ref<Entity> entity = Create(m_Scene, uuid, newName);
		entity->SetEntityParent(this);

		if (index == -1)
			m_Children.push_back(entity);
		else
			m_Children.insert(m_Children.begin() + index, entity);

		return entity;
	}

	void Entity::SetEntityParent(Entity* entity)
	{
		m_Parent = entity;
	}

	template<>
	void Entity::RemoveComponent<ScriptComponent>()
	{
		VOL_CORE_ASSERT(HasComponent<ScriptComponent>(), "RemoveComponent: Entity does not has component!");
		
		auto entityID = GetUUID();

		m_Scene->m_Registry.remove<ScriptComponent>(m_EntityHandle);
	}

}
