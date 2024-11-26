#include "volpch.h"
#include "Prefab.h"
#include "SceneSerializer.h"
#include "Volcano/Project/Project.h"

namespace Volcano{

	Ref<Scene> Prefab::m_Scene;
	std::unordered_map<std::string, Ref<Entity>> Prefab::m_EntityPathMap;
	std::unordered_map<UUID, std::string> Prefab::m_EntityPrefabMap;

	Ref<Scene>& Prefab::GetScene()
	{
		return m_Scene;
	}

	// prefabPath绝对路径
	Ref<Entity> Prefab::Load(std::filesystem::path prefabPath)
	{
		SceneSerializer serializer(m_Scene);
		Ref<Entity> entity = serializer.DeserializePrefab(prefabPath.string(), prefabPath.stem().string());

		std::string prefabPathTemp = Project::GetRelativeAssetDirectory(prefabPath).string();
		m_EntityPathMap[prefabPathTemp] = entity;
		m_EntityPrefabMap[entity->GetUUID()] = prefabPathTemp;
		return entity;
	}

	Ref<Entity> Prefab::Get(const std::string& prefabPath)
	{
		if (m_EntityPathMap.find(prefabPath) != m_EntityPathMap.end())
			return m_EntityPathMap[prefabPath];
		else
			return nullptr;
	}

	std::unordered_map<std::string, Ref<Entity>>& Prefab::GetEntityPathMap() { return m_EntityPathMap; }

	std::unordered_map<UUID, std::string>& Prefab::GetEntityPrefabMap() { return m_EntityPrefabMap; }

	const Ref<Scene>& Prefab::Reset()
	{
		m_EntityPathMap.clear();
		m_EntityPrefabMap.clear();
		m_Scene.reset(new Scene());
		m_Scene->SetName("Prefab");
		return m_Scene;
	}

	bool Prefab::IsPrefabHasTarget(std::string& prefabPath)
	{
		for (auto& [id, path] : m_EntityPrefabMap)
			if (path == prefabPath)
				return true;
		return false;
	}

	// targetEntity为空时重置scene下Prefab /entity/ 有联系的所有实体，否则重置仅重置targetEntity
	void Prefab::ReloadPrefabTarget(Ref<Entity> entity, Scene* scene, UUID entityID)
	{
		if (entity == nullptr || scene == nullptr)
			return;

		if (entityID == 0)
		{
			std::string prefabPath = Prefab::GetPrefabPath(entity);
			for (auto& [id, prefabPathTemp] : m_EntityPrefabMap)
			{
				if (prefabPathTemp == prefabPath)
				{
					Ref<Entity> targetEntity = scene->GetEntityByUUID(id);
					Ref<Entity> targetEntityParent;
					if (targetEntity->GetEntityParent() != nullptr)
						targetEntityParent = scene->GetEntityByUUID(targetEntity->GetEntityParent()->GetUUID());

					scene->DestroyEntity(targetEntity);
					scene->DuplicateEntity(entity, targetEntityParent.get(), id);
				}
			}
		}
		else
		{
			Ref<Entity> targetEntity = scene->GetEntityByUUID(entityID);
			if (targetEntity == nullptr)
				return;

			Ref<Entity> targetEntityParent;
			if (targetEntity->GetEntityParent() != nullptr)
				targetEntityParent = scene->GetEntityByUUID(targetEntity->GetEntityParent()->GetUUID());

			scene->DestroyEntity(targetEntity);
			scene->DuplicateEntity(entity, targetEntityParent.get(), entityID);
		}

	}
	
	// 移除Prefab实体的联系
	void Prefab::RemovePrefabTarget(Ref<Entity> targetEntity)
	{
		if (targetEntity == nullptr)
			return;

		m_EntityPrefabMap.erase(targetEntity->GetUUID());
	}

	void Prefab::RemovePrefab(Ref<Entity> prefabEntity)
	{
		if (prefabEntity == nullptr)
			return;

		std::string prefabPath = Prefab::GetPrefabPath(prefabEntity);
		if (prefabPath.empty())
			return;

		for (auto it = m_EntityPrefabMap.begin(); it != m_EntityPrefabMap.end(); )
		{
			if (it->second == prefabPath)
				it = m_EntityPrefabMap.erase(it);
			else
				it++;
		}
		m_EntityPathMap.erase(prefabPath);
	}

	// 获取prefab原型的路径
	std::string Prefab::GetPrefabPath(Ref<Entity> prefabEntity)
	{
		if (prefabEntity == nullptr)
			return std::string();

		auto it = m_EntityPrefabMap.find(prefabEntity->GetUUID());
		if (it == m_EntityPrefabMap.end())
			return std::string();

		return it->second;
	}
}