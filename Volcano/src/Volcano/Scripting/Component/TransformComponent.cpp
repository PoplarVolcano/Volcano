#include "volpch.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Scripting/ComponentRegister.h"

#include "mono/metadata/object.h"

namespace Volcano {

#define VOL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volcano.TransformComponent::" #Name, Name)

	static void TransformComponent_GetTranslation(UUID entityID, glm::vec3* outTranslation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outTranslation = entity.GetComponent<TransformComponent>().Translation;
	}

	static void TransformComponent_SetTranslation(UUID entityID, glm::vec3* translation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		entity.GetComponent<TransformComponent>().Translation = *translation;
	}

	static void TransformComponent_GetRotation(UUID entityID, glm::vec3* outRotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outRotation = entity.GetComponent<TransformComponent>().Rotation;
	}

	static void TransformComponent_SetRotation(UUID entityID, glm::vec3* rotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		entity.GetComponent<TransformComponent>().Rotation = *rotation;
	}

	static void TransformComponent_GetScale(UUID entityID, glm::vec3* outScale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outScale = entity.GetComponent<TransformComponent>().Scale;
	}

	static void TransformComponent_SetScale(UUID entityID, glm::vec3* scale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		entity.GetComponent<TransformComponent>().Scale = *scale;
	}

	void ComponentRegister::TransformComponent_RegisterFunctions()
	{
		VOL_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		VOL_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		VOL_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		VOL_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		VOL_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		VOL_ADD_INTERNAL_CALL(TransformComponent_SetScale);
	}

}