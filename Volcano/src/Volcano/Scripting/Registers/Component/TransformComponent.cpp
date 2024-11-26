#include "volpch.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Scripting/Registers/ComponentRegister.h"
#include "Volcano/Math/Math.h"

#include "mono/metadata/object.h"

#include "glm/gtx/euler_angles.hpp"

namespace Volcano {

#define VOL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volcano.Transform::" #Name, Name)

	static void Transform_GetLocalPosition(UUID entityID, glm::vec3* outPosition)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outPosition = entity->GetComponent<TransformComponent>().Translation;
	}

	static void Transform_SetLocalPosition(UUID entityID, glm::vec3* position)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		entity->GetComponent<TransformComponent>().Translation = *position;
	}

	static void Transform_GetPosition(UUID entityID, glm::vec3* outPosition)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outPosition = entity->GetParentTransform() * glm::vec4(entity->GetComponent<TransformComponent>().Translation, 1.0f);
	}

	static void Transform_SetPosition(UUID entityID, glm::vec3* position)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		entity->GetComponent<TransformComponent>().Translation = inverse(entity->GetParentTransform()) * glm::vec4(*position, 1.0f);
	}

	static void Transform_GetLocalEulerAngles(UUID entityID, glm::vec3* outEulerAngles)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outEulerAngles = entity->GetComponent<TransformComponent>().Rotation;
	}

	static void Transform_SetLocalEulerAngles(UUID entityID, glm::vec3* eulerAngles)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		entity->GetComponent<TransformComponent>().Rotation = *eulerAngles;
	}

	static void Transform_GetEulerAngles(UUID entityID, glm::vec3* outEulerAngles)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		glm::vec3 parentPosition, parentRotation, parentScale;
		Math::DecomposeTransform(entity->GetParentTransform(), parentPosition, parentRotation, parentScale);
		*outEulerAngles = parentRotation + entity->GetComponent<TransformComponent>().Rotation;
	}

	static void Transform_SetEulerAngles(UUID entityID, glm::vec3* eulerAngles)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		glm::vec3 parentPosition, parentRotation, parentScale;
		Math::DecomposeTransform(entity->GetParentTransform(), parentPosition, parentRotation, parentScale);
		entity->GetComponent<TransformComponent>().Rotation = *eulerAngles - parentRotation;
	}

	static void Transform_GetLocalRotation(UUID entityID, glm::quat* outRotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outRotation = entity->GetComponent<TransformComponent>().Rotation;
	}

	static void Transform_SetLocalRotation(UUID entityID, glm::quat* rotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		auto rotations = glm::eulerAngles(*rotation);
		entity->GetComponent<TransformComponent>().Rotation = glm::eulerAngles(*rotation);
	}

	static void Transform_GetRotation(UUID entityID, glm::quat* outRotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		glm::vec3 parentPosition, parentRotation, parentScale;
		Math::DecomposeTransform(entity->GetParentTransform(), parentPosition, parentRotation, parentScale);
		*outRotation = parentRotation + entity->GetComponent<TransformComponent>().Rotation;
	}

	static void Transform_SetRotation(UUID entityID, glm::quat* rotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		glm::vec3 parentPosition, parentRotation, parentScale;
		Math::DecomposeTransform(entity->GetParentTransform(), parentPosition, parentRotation, parentScale);
		entity->GetComponent<TransformComponent>().Rotation = glm::eulerAngles(*rotation) - parentRotation;
	}

	static void Transform_GetScale(UUID entityID, glm::vec3* outScale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outScale = entity->GetComponent<TransformComponent>().Scale;
	}

	static void Transform_SetScale(UUID entityID, glm::vec3* scale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		entity->GetComponent<TransformComponent>().Scale = *scale;
	}

	// Transforms /direction/ from local space to world space.
	static void Transform_TransformSpace(UUID entityID, glm::vec3 vector, glm::vec3* outVector)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		glm::vec3 parentPosition, parentRotation, parentScale;
		Math::DecomposeTransform(entity->GetParentTransform(), parentPosition, parentRotation, parentScale);
		*outVector = glm::quat(parentRotation) * vector;
	}

	// Transforms /direction/ from world space to local space.
	static void Transform_InverseTransformSpace(UUID entityID, glm::vec3 vector, glm::vec3* outVector)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		glm::vec3 parentPosition, parentRotation, parentScale;
		Math::DecomposeTransform(entity->GetParentTransform(), parentPosition, parentRotation, parentScale);
		*outVector = glm::quat(-parentRotation) * vector;
	}

	static void Transform_LookAt(UUID entityID, glm::vec3 worldPosition, glm::vec3 worldUp)
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

	static MonoObject* Transform_GetParent(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		if (entity->GetEntityParent() != nullptr)
			return ScriptInstance(ScriptEngine::GetClass("System.Transform"), *entity->GetEntityParent()).GetManagedObject();
		else
			return nullptr;

	}

	static void Transform_SetParent(UUID entityID, MonoObject* parent)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);


		if (parent == nullptr)
			ScriptEngine::GetEntityUpdateList().push({ EntityUpdateType::MOVE, entityID });
		else
		{
			ScriptInstance parentInstance(parent);
			UUID parentID = parentInstance.GetFieldValue<UUID>("ID");
			ScriptEngine::GetEntityUpdateList().push({ EntityUpdateType::MOVE, entityID , parentID});
		}
		/*
			Scene::UpdateEntityParent(entity, nullptr);
		else
		{
			ScriptInstance parentInstance(parent);
			UUID parentID = parentInstance.GetFieldValue<UUID>("ID");
			Ref<Entity> parentEntity = scene->GetEntityByUUID(parentID);
			Scene::UpdateEntityParent(entity, parentEntity.get());
		}
		*/
	}


	static void Transform_GetChildren(UUID entityID, MonoArray** outChildren)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		auto& children = entity->GetEntityChildren();
		auto* image = ScriptEngine::GetAppAssemblyImage();

		//MonoClass* monoClass = mono_class_from_name(ScriptEngine::GetMscorlibAssemblyImage(), "System", "UInt64");
		MonoArray* monoArray = mono_array_new(ScriptEngine::GetCoreAssemblyDomain(), ScriptEngine::GetClass(ScriptFieldType::ULong), children.size());

		int i = 0;
		for (auto& child : entity->GetEntityChildren())
		{
			mono_array_set(monoArray, uint64_t, i, child->GetUUID()); // 设置第i个元素为i + 1
			i++;
		}
		*outChildren = monoArray;
	}

	static MonoObject* Transform_GetChild(UUID entityID, uint64_t index)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);
		if (index >= entity->GetEntityChildren().size())
			return 0;
		auto it = entity->GetEntityChildren().begin();
		std::advance(it, index);
		
		return ScriptInstance(ScriptEngine::GetClass("Volcano.Transform"), *(*it).get()).GetManagedObject();
	}

	static uint64_t Transform_GetChildrenCount(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		return entity->GetEntityChildren().size();
	}

	static void Transform_DetachChildren(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		for (auto& child : entity->GetEntityChildren())
			ScriptEngine::GetEntityUpdateList().push({ EntityUpdateType::MOVE, child->GetUUID()});
			//Scene::UpdateEntityParent(child, nullptr);
	}



	void ComponentRegister::TransformComponent_RegisterFunctions()
	{
		VOL_ADD_INTERNAL_CALL(Transform_GetLocalPosition);
		VOL_ADD_INTERNAL_CALL(Transform_SetLocalPosition);
		VOL_ADD_INTERNAL_CALL(Transform_GetPosition);
		VOL_ADD_INTERNAL_CALL(Transform_SetPosition);
		VOL_ADD_INTERNAL_CALL(Transform_GetLocalEulerAngles);
		VOL_ADD_INTERNAL_CALL(Transform_SetLocalEulerAngles);
		VOL_ADD_INTERNAL_CALL(Transform_GetEulerAngles);
		VOL_ADD_INTERNAL_CALL(Transform_SetEulerAngles);
		VOL_ADD_INTERNAL_CALL(Transform_GetLocalRotation);
		VOL_ADD_INTERNAL_CALL(Transform_SetLocalRotation);
		VOL_ADD_INTERNAL_CALL(Transform_GetRotation);
		VOL_ADD_INTERNAL_CALL(Transform_SetRotation);
		VOL_ADD_INTERNAL_CALL(Transform_GetScale);
		VOL_ADD_INTERNAL_CALL(Transform_SetScale);
		VOL_ADD_INTERNAL_CALL(Transform_TransformSpace);
		VOL_ADD_INTERNAL_CALL(Transform_InverseTransformSpace);
		VOL_ADD_INTERNAL_CALL(Transform_LookAt);

		VOL_ADD_INTERNAL_CALL(Transform_GetParent);
		VOL_ADD_INTERNAL_CALL(Transform_SetParent);
		VOL_ADD_INTERNAL_CALL(Transform_GetChildren);
		VOL_ADD_INTERNAL_CALL(Transform_GetChild);
		VOL_ADD_INTERNAL_CALL(Transform_GetChildrenCount);
		VOL_ADD_INTERNAL_CALL(Transform_DetachChildren);
	}

}