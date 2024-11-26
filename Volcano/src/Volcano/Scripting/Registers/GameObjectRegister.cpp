#include "volpch.h"
#include "GameObjectRegister.h"
#include "Volcano/Scripting/ScriptEngine.h"

#include "mono/metadata/object.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"
#include "mono/metadata/attrdefs.h"
#include "mono/metadata/mono-debug.h"
#include "mono/metadata/threads.h"

namespace Volcano {

#define VOL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volcano.GameObject::" #Name, Name)

	static MonoObject* GameObject_GetComponent(UUID entityID, MonoObject* type)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		std::string typeName = ScriptEngine::MonoToString(type);

		auto scriptClass = ScriptEngine::GetClass(typeName);
		if(scriptClass != nullptr)
	        return ScriptEngine::CreateInstance(scriptClass->GetClass(), entityID);
		return nullptr;
	}

	static void GameObject_GetComponentFastPath(UUID entityID, MonoObject* type, MonoObject** outComponent)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		auto scriptClass = ScriptEngine::GetClass(ScriptEngine::MonoToString(type));
		if (scriptClass != nullptr)
		{
			if (mono_class_is_subclass_of(scriptClass->GetClass(), ScriptEngine::GetMonoBehaviourClass()->GetClass(), false))
				*outComponent = ScriptEngine::GetManagedInstance(entity->GetUUID());
			else
				*outComponent = ScriptEngine::CreateInstance(scriptClass->GetClass(), entityID);
		}
		else
			*outComponent = nullptr;

	}

	static bool GameObject_GetActive(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		return entity->GetActive();
	}

	static void GameObject_SetActive(UUID entityID, bool active)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		entity->SetActive(active);

		if (active == true)
		{
			if (entity->HasComponent<ScriptComponent>())
			{
				auto& sc = entity->GetComponent<ScriptComponent>();
				if (!sc.ClassName.empty())
				{
					auto instance = ScriptEngine::GetEntityScriptInstance(entity->GetUUID());
					if (instance == nullptr)
					{
						ScriptEngine::CreateEntity(*entity.get());
						ScriptEngine::AwakeEntity(*entity.get());
					}
					else
						instance->SetEnabled(sc.enabled);
					ScriptEngine::OnEnableEntity(*entity.get());
					ScriptEngine::StartEntity(*entity.get());
				}
			}
		}
		else
		{
			if (entity->HasComponent<ScriptComponent>())
			{
				auto& sc = entity->GetComponent<ScriptComponent>();
				if (!sc.ClassName.empty())
				{
					auto instance = ScriptEngine::GetEntityScriptInstance(entity->GetUUID());
					if (instance == nullptr)
					{
						ScriptEngine::CreateEntity(*entity.get());
						ScriptEngine::AwakeEntity(*entity.get());
					}
					else
						instance->SetEnabled(sc.enabled);
					ScriptEngine::OnDisableEntity(*entity.get());
				}
			}
		}

	}

	static MonoObject* GameObject_Find(MonoString* name)
	{
		char* nameCStr = mono_string_to_utf8(name);

		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = Scene::Find(nameCStr, scene->GetEntityList());
		mono_free(nameCStr);

		if (!entity)
			return nullptr;

		return ScriptEngine::CreateInstance(ScriptEngine::GetGameObjectClass()->GetClass(), entity->GetUUID());
	}

	void GameObjectRegister::GameObject_RegisterFunctions()
	{
		VOL_ADD_INTERNAL_CALL(GameObject_GetComponent);
		VOL_ADD_INTERNAL_CALL(GameObject_GetComponentFastPath);

		VOL_ADD_INTERNAL_CALL(GameObject_GetActive);
		VOL_ADD_INTERNAL_CALL(GameObject_SetActive);
		VOL_ADD_INTERNAL_CALL(GameObject_Find);
	}
}