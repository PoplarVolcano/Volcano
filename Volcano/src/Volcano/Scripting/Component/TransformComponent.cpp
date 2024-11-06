#include "volpch.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Scripting/ComponentRegister.h"
#include "Volcano/Math/Math.h"

#include "mono/metadata/object.h"

#include "glm/gtx/euler_angles.hpp"

namespace Volcano {

#define VOL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volcano.TransformComponent::" #Name, Name)

	static void TransformComponent_GetLocalPosition(UUID entityID, glm::vec3* outPosition)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outPosition = entity->GetComponent<TransformComponent>().Translation;
	}

	static void TransformComponent_SetLocalPosition(UUID entityID, glm::vec3* position)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		entity->GetComponent<TransformComponent>().Translation = *position;
	}

	static void TransformComponent_GetPosition(UUID entityID, glm::vec3* outPosition)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outPosition = entity->GetParentTransform() * glm::vec4(entity->GetComponent<TransformComponent>().Translation, 1.0f);
	}

	static void TransformComponent_SetPosition(UUID entityID, glm::vec3* position)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		entity->GetComponent<TransformComponent>().Translation = inverse(entity->GetParentTransform()) * glm::vec4(*position, 1.0f);
	}

	static void TransformComponent_GetLocalEulerAngles(UUID entityID, glm::vec3* outEulerAngles)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outEulerAngles = entity->GetComponent<TransformComponent>().Rotation;
	}

	static void TransformComponent_SetLocalEulerAngles(UUID entityID, glm::vec3* eulerAngles)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		entity->GetComponent<TransformComponent>().Rotation = *eulerAngles;
	}

	static void TransformComponent_GetEulerAngles(UUID entityID, glm::vec3* outEulerAngles)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		glm::vec3 parentPosition, parentRotation, parentScale;
		Math::DecomposeTransform(entity->GetParentTransform(), parentPosition, parentRotation, parentScale);
		*outEulerAngles = parentRotation + entity->GetComponent<TransformComponent>().Rotation;
	}

	static void TransformComponent_SetEulerAngles(UUID entityID, glm::vec3* eulerAngles)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		glm::vec3 parentPosition, parentRotation, parentScale;
		Math::DecomposeTransform(entity->GetParentTransform(), parentPosition, parentRotation, parentScale);
		entity->GetComponent<TransformComponent>().Rotation = *eulerAngles - parentRotation;
	}

	static void TransformComponent_GetLocalRotation(UUID entityID, glm::quat* outRotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outRotation = entity->GetComponent<TransformComponent>().Rotation;
	}

	static void TransformComponent_SetLocalRotation(UUID entityID, glm::quat* rotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		auto rotations = glm::eulerAngles(*rotation);
		entity->GetComponent<TransformComponent>().Rotation = glm::eulerAngles(*rotation);
	}

	static void TransformComponent_GetRotation(UUID entityID, glm::quat* outRotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		glm::vec3 parentPosition, parentRotation, parentScale;
		Math::DecomposeTransform(entity->GetParentTransform(), parentPosition, parentRotation, parentScale);
		*outRotation = parentRotation + entity->GetComponent<TransformComponent>().Rotation;
	}

	static void TransformComponent_SetRotation(UUID entityID, glm::quat* rotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		glm::vec3 parentPosition, parentRotation, parentScale;
		Math::DecomposeTransform(entity->GetParentTransform(), parentPosition, parentRotation, parentScale);
		entity->GetComponent<TransformComponent>().Rotation = glm::eulerAngles(*rotation) - parentRotation;
	}

	static void TransformComponent_GetScale(UUID entityID, glm::vec3* outScale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outScale = entity->GetComponent<TransformComponent>().Scale;
	}

	static void TransformComponent_SetScale(UUID entityID, glm::vec3* scale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		entity->GetComponent<TransformComponent>().Scale = *scale;
	}

	// Transforms /direction/ from local space to world space.
	static void TransformComponent_TransformDirection(UUID entityID, glm::vec3 direction, glm::vec3* outDirection)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		glm::vec3 parentPosition, parentRotation, parentScale;
		Math::DecomposeTransform(entity->GetParentTransform(), parentPosition, parentRotation, parentScale);
		*outDirection = glm::quat(parentRotation) * direction;
	}

	// Transforms /direction/ from world space to local space.
	static void TransformComponent_InverseTransformDirection(UUID entityID, glm::vec3 direction, glm::vec3* outDirection)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		glm::vec3 parentPosition, parentRotation, parentScale;
		Math::DecomposeTransform(entity->GetParentTransform(), parentPosition, parentRotation, parentScale);
		*outDirection = glm::quat(-parentRotation) * direction;
	}

	static void TransformComponent_LookAt(UUID entityID, glm::vec3 worldPosition, glm::vec3 worldUp)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		glm::vec3 parentPosition, parentRotation, parentScale;
		Math::DecomposeTransform(entity->GetParentTransform(), parentPosition, parentRotation, parentScale);
		auto& transformComponent = entity->GetComponent<TransformComponent>();
		glm::vec3 eyeWorldPosition = entity->GetParentTransform() * glm::vec4(transformComponent.Translation, 1.0f);
		glm::mat4 lookAt = glm::lookAt(eyeWorldPosition, worldPosition, worldUp);
		transformComponent.Rotation = glm::eulerAngles(glm::quat(glm::inverse(lookAt))) - parentRotation;
	}

	void ComponentRegister::TransformComponent_RegisterFunctions()
	{
		VOL_ADD_INTERNAL_CALL(TransformComponent_GetLocalPosition);
		VOL_ADD_INTERNAL_CALL(TransformComponent_SetLocalPosition);
		VOL_ADD_INTERNAL_CALL(TransformComponent_GetPosition);
		VOL_ADD_INTERNAL_CALL(TransformComponent_SetPosition);
		VOL_ADD_INTERNAL_CALL(TransformComponent_GetLocalEulerAngles);
		VOL_ADD_INTERNAL_CALL(TransformComponent_SetLocalEulerAngles);
		VOL_ADD_INTERNAL_CALL(TransformComponent_GetEulerAngles);
		VOL_ADD_INTERNAL_CALL(TransformComponent_SetEulerAngles);
		VOL_ADD_INTERNAL_CALL(TransformComponent_GetLocalRotation);
		VOL_ADD_INTERNAL_CALL(TransformComponent_SetLocalRotation);
		VOL_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		VOL_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		VOL_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		VOL_ADD_INTERNAL_CALL(TransformComponent_SetScale);
		VOL_ADD_INTERNAL_CALL(TransformComponent_TransformDirection);
		VOL_ADD_INTERNAL_CALL(TransformComponent_InverseTransformDirection);
		VOL_ADD_INTERNAL_CALL(TransformComponent_LookAt);
	}

}