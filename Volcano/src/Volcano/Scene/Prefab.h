#pragma once

#include "Volcano/Scene/Entity.h"

namespace  Volcano {

	class Prefab 
	{
	public:
		static Ref<Scene>& GetScene();
		static Ref<Entity>& Load(std::filesystem::path prefabPath);
		static Ref<Entity> Get(const std::string& prefabPath);
		static std::unordered_map<std::string, Ref<Entity>>& GetEntityPathMap();
		static std::unordered_map<std::string, std::unordered_map<UUID, Ref<Entity>>>& GetTargetEntityPathMap();
		static const Ref<Scene>& Reset();
		static bool IsPrefabHasTarget(std::string& prefabPath);
		static void ReloadPrefabTarget(Ref<Entity> entity, Ref<Entity> targetEntity = nullptr);
		static void AddPrefabTarget(std::string& prefabPath, Ref<Entity> targetEntity);
		static void RemovePrefabTarget(Ref<Entity> targetEntity);
		static void RemovePrefabTarget(std::string& prefabPath, UUID targetEntity);
	private:
		static Ref<Scene> m_Scene;
		static std::unordered_map<std::string, Ref<Entity>> m_EntityPathMap;
		static std::unordered_map<std::string, std::unordered_map<UUID, Ref<Entity>>> m_TargetEntityPathMap;
	};

}