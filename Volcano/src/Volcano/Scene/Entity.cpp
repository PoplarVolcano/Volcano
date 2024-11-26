#include "volpch.h"

#include "Entity.h"
#include "Volcano//Math/Math.h"

namespace Volcano {

	Ref<Entity> Entity::Create(Scene& scene, UUID uuid, const std::string& name)
	{
		Ref<Entity> entity = std::make_shared<Entity>(scene.m_Registry.create(), &scene);
		entity->AddComponent<TransformComponent>();
		entity->AddComponent<IDComponent>(uuid); // ʹ��ʵ��uuid���������µ�
		auto& tag = entity->AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Empty Entity" : name;

		return entity;
	}

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
		
	}

	// �½��ӽڵ�
	Ref<Entity> Entity::AddEntityChild(UUID uuid, const std::string& name)
	{
		std::string newName = Scene::NewName(m_Children, name);

		Ref<Entity> entity = Create(*m_Scene, uuid, newName);
		entity->SetEntityParent(this);
		m_Children.push_back(entity);

		return entity;
	}

	// ��entity���뵽�ӽڵ�(������entityԭ���ڵ���ӽڵ��б�)
	Ref<Entity> Entity::AddEntityChild(Ref<Entity> entity)
	{
		std::string newName = Scene::NewName(m_Children, entity->GetName());

		entity->SetName(newName);
		entity->SetEntityParent(this);
		m_Children.push_back(entity);

		return entity;
	}

	void Entity::SetEntityParent(Entity* entity)
	{
		m_Parent = entity;
	}

	glm::mat4 Entity::GetTransform()
	{
		return GetComponent<TransformComponent>().GetTransform();
	}

	glm::mat4 Entity::GetParentTransform()
	{
		glm::mat4 parentTransform = glm::mat4(1.0f);
		Entity* parent = m_Parent;
		while (parent != nullptr)
		{
			parentTransform = parent->GetTransform() * parentTransform;
			parent = parent->GetEntityParent();
		}
		return parentTransform;
	}

	void Entity::SetPosition(const glm::vec3& position, bool isWorldSpace)
	{
		if(isWorldSpace)
			GetComponent<TransformComponent>().Translation = inverse(GetParentTransform()) * glm::vec4(position, 1.0f);
		else
			GetComponent<TransformComponent>().Translation = position;
	}

	void Entity::SetRotation(const glm::vec3& rotation, bool isWorldSpace)
	{
		if (isWorldSpace)
		{
			glm::vec3 parentPosition, parentRotation, parentScale;
			Math::DecomposeTransform(GetParentTransform(), parentPosition, parentRotation, parentScale);
			GetComponent<TransformComponent>().Rotation = rotation - parentRotation;
		}
		else
			GetComponent<TransformComponent>().Rotation = rotation;
	}

}
