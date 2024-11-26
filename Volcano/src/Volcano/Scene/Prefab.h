#pragma once

#include "Volcano/Scene/Entity.h"

namespace  Volcano {

	class Prefab 
	{
	public:
		static Ref<Scene>& GetScene();
		static Ref<Entity> Load(std::filesystem::path prefabPath);
		static Ref<Entity> Get(const std::string& prefabPath);
		static std::unordered_map<std::string, Ref<Entity>>& GetEntityPathMap();
		static std::unordered_map<UUID, std::string>& GetEntityPrefabMap();
		static const Ref<Scene>& Reset();
		static bool IsPrefabHasTarget(std::string& prefabPath);
		static void ReloadPrefabTarget(Ref<Entity> entity, Scene* scene, UUID entityID = 0);
		static void RemovePrefabTarget(Ref<Entity> targetEntity);
		static void RemovePrefab(Ref<Entity> prefabEntity);
		static std::string GetPrefabPath(Ref<Entity> prefabEntity);
		static void Clear();
	private:
		static Ref<Scene> m_Scene;
		static std::unordered_map<std::string, Ref<Entity>> m_EntityPathMap;
		static std::unordered_map<UUID, std::string> m_EntityPrefabMap;
	};

}