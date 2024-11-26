#include "volpch.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Scripting/Registers/ComponentRegister.h"

#include "mono/metadata/object.h"
namespace Volcano {

#define VOL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volcano.Component::" #Name, Name)

	static MonoObject* Component_GetComponent(UUID entityID, MonoObject* type)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		std::string typeName = ScriptEngine::MonoToString(type);

		auto scriptClass = ScriptEngine::GetClass(typeName);
		if (scriptClass != nullptr)
			return ScriptEngine::CreateInstance(scriptClass->GetClass(), entityID);
		return nullptr;
	}

	static void Component_GetComponentFastPath(UUID entityID, MonoObject* type, MonoObject** outComponent)
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

	static MonoObject* Component_GetGameObject(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		auto gameObjectClass = ScriptEngine::GetGameObjectClass();
		if (gameObjectClass != nullptr)
			return ScriptEngine::CreateInstance(gameObjectClass->GetClass(), entityID);
		return nullptr;
	}


	void ComponentRegister::Component_RegisterFunctions()
	{
		VOL_ADD_INTERNAL_CALL(Component_GetComponent);
		VOL_ADD_INTERNAL_CALL(Component_GetComponentFastPath);
		VOL_ADD_INTERNAL_CALL(Component_GetGameObject);
	}
}