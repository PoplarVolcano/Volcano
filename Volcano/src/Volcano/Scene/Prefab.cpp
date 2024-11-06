#include "volpch.h"
#include "Prefab.h"
#include "SceneSerializer.h"
#include "Volcano/Project/Project.h"

namespace Volcano{

	Ref<Scene> Prefab::m_Scene;
	std::unordered_map<std::string, Ref<Entity>> Prefab::m_EntityPathMap;
	std::unordered_map<std::string, std::unordered_map<UUID, Ref<Entity>>> Prefab::m_TargetEntityPathMap;

	Ref<Scene>& Prefab::GetScene()
	{
		return m_Scene;
	}

	// prefabPath¾ø¶ÔÂ·¾¶
	Ref<Entity>& Prefab::Load(std::filesystem::path prefabPath)
	{
		SceneSerializer serializer(m_Scene);
		Ref<Entity> entity = serializer.DeserializePrefab(prefabPath.string(), prefabPath.stem().string());
		m_EntityPathMap[entity->GetPrefabPath()] = entity;
		return m_EntityPathMap[entity->GetPrefabPath()];
	}
	Ref<Entity> Prefab::Get(const std::string& prefabPath)
	{
		if (m_EntityPathMap.find(prefabPath) != m_EntityPathMap.end())
			return m_EntityPathMap[prefabPath];
		else
			return nullptr;
	}

	std::unordered_map<std::string, Ref<Entity>>& Prefab::GetEntityPathMap() { return m_EntityPathMap; }

	std::unordered_map<std::string, std::unordered_map<UUID, Ref<Entity>>>& Prefab::GetTargetEntityPathMap() { return m_TargetEntityPathMap; }

	const Ref<Scene>& Prefab::Reset()
	{
		m_EntityPathMap.clear();
		m_TargetEntityPathMap.clear();
		m_Scene.reset(new Scene());
		m_Scene->SetName("Prefab");
		return m_Scene;
	}

	bool Prefab::IsPrefabHasTarget(std::string& prefabPath)
	{
		for (auto& [path, entityMap] : m_TargetEntityPathMap)
			if (path == prefabPath)
				if(entityMap.empty())
				    return false;
				else
					return true;
	}

	void Prefab::ReloadPrefabTarget(Ref<Entity> entity, Ref<Entity> targetEntity)
	{
		if (targetEntity == nullptr)
		{
			for (auto& [id, targetEntity] : Prefab::GetTargetEntityPathMap()[entity->GetPrefabPath()])
			{
				Scene* scene = targetEntity->GetScene();
				Ref<Entity> targetEntityParent = scene->GetEntityByUUID(targetEntity->GetEntityParent()->GetUUID());
				scene->DestroyEntity(targetEntity);
				Ref<Entity> resultEntity = scene->DuplicateEntity(entity, targetEntityParent, id);
				targetEntity = resultEntity;
			}
		}
		else
		{
			Scene* scene = targetEntity->GetScene();
			Ref<Entity> targetEntityParent = scene->GetEntityByUUID(targetEntity->GetEntityParent()->GetUUID());
			UUID id = targetEntity->GetUUID();
			scene->DestroyEntity(targetEntity);
			Ref<Entity> resultEntity = scene->DuplicateEntity(entity, targetEntityParent, id);
			targetEntity = resultEntity;
		}

	}
	void Prefab::AddPrefabTarget(std::string& prefabPath, Ref<Entity> targetEntity)
	{
		if (targetEntity != nullptr)
			m_TargetEntityPathMap[prefabPath][targetEntity->GetUUID()] = targetEntity;
	}
	void Prefab::RemovePrefabTarget(Ref<Entity> targetEntity)
	{
		if (targetEntity != nullptr)
			if (!targetEntity->GetPrefabPath().empty())
			{
				m_TargetEntityPathMap[targetEntity->GetPrefabPath()].erase(targetEntity->GetUUID());
			}
	}
	void Prefab::RemovePrefabTarget(std::string& prefabPath, UUID targetEntity)
	{
		if(targetEntity != 0)
			m_TargetEntityPathMap[prefabPath].erase(targetEntity);
	}
}