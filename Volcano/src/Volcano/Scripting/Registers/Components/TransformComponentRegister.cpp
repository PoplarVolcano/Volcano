#include "volpch.h"
#include "Volcano/Scripting/Registers/ComponentRegister.h"

#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Math/Math.h"

#include <mono/metadata/object.h>
#include <glm/gtx/matrix_decompose.hpp>

namespace Volcano {

#define VOL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volcano.Transform::" #Name, Name)

	static void Transform_GetPosition(UUID entityID, glm::vec3* outPosition)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		Entity* parent = entity->GetEntityParent();
		if (parent == nullptr)
		    *outPosition = entity->GetComponent<TransformComponent>().translation;
		else
			*outPosition = parent->GetWorldTransform() * glm::vec4(entity->GetComponent<TransformComponent>().translation, 1.0f);
	}

	static void Transform_SetPosition(UUID entityID, glm::vec3* position)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		Entity* parent = entity->GetEntityParent();
		if (parent == nullptr)
	        entity->GetComponent<TransformComponent>().translation = *position;
		else
		{
			glm::mat4 invParent = glm::inverse(parent->GetWorldTransform());
			glm::vec3 localPos = invParent * glm::vec4(*position, 1.0f);
			entity->GetComponent<TransformComponent>().translation = localPos;
		}

	}

	static void Transform_GetLocalPosition(UUID entityID, glm::vec3* outPosition)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outPosition = entity->GetComponent<TransformComponent>().translation;
	}

	static void Transform_SetLocalPosition(UUID entityID, glm::vec3* position)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		entity->GetComponent<TransformComponent>().translation = *position;
	}


	static void Transform_GetRotation(UUID entityID, glm::quat* outRotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		// 多级乘法会累积误差
		*outRotation = glm::normalize(entity->GetWorldRotation());
	}

	static void Transform_SetRotation(UUID entityID, glm::quat* rotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		if (Entity* parent = entity->GetEntityParent())
		{
			glm::quat parentWorldRotation = glm::normalize(parent->GetWorldRotation());

			// localRotation = inv(parentWorldRotation) * worldRotation
			glm::quat localRotation = glm::normalize(glm::inverse(parentWorldRotation) * (*rotation));

			entity->GetComponent<TransformComponent>().SetRotation(localRotation);
		}
		else
		{
			entity->GetComponent<TransformComponent>().SetRotation(*rotation);
		}
	}

	static void Transform_GetLocalRotation(UUID entityID, glm::quat* outRotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outRotation = entity->GetComponent<TransformComponent>().rotation;
	}

	static void Transform_SetLocalRotation(UUID entityID, glm::quat* rotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		entity->GetComponent<TransformComponent>().SetRotation(*rotation);
	}

	// 注：glm::quat(vec3) 和 glm::eulerAngles(quat) 用的是 XYZ 顺序，
	//     C# 端的 EulerRadians / ToEulerRadians 用的是 ZXY 顺序。
	//     两边不一致，欧拉角的读写会乱掉。
	//     现在主要在 C# 端做所有角度运算，C++ 端的 GetEulerAngles / SetEulerAngles 可以暂缓处理。
	static void Transform_GetEulerAngles(UUID entityID, glm::vec3* outEulerAngles)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outEulerAngles = glm::eulerAngles(glm::normalize(entity->GetWorldRotation()));
	}

	static void Transform_SetEulerAngles(UUID entityID, glm::vec3* eulerAngles)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		glm::quat quat = glm::normalize(glm::quat(*eulerAngles));
		if (Entity* parent = entity->GetEntityParent())
		{
			glm::quat parentWorldRotation = glm::normalize(parent->GetWorldRotation());

			// 计算局部四元数：local = inv(parent) * world
			glm::quat localRotation = glm::normalize(glm::inverse(parentWorldRotation) * quat);

			entity->GetComponent<TransformComponent>().SetRotation(localRotation);
		}
		else
		{
			entity->GetComponent<TransformComponent>().rotation = quat;
			entity->GetComponent<TransformComponent>().inspectorEulerHint = glm::degrees(*eulerAngles);
		}
	}

	static void Transform_GetLocalEulerAngles(UUID entityID, glm::vec3* outEulerAngles)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outEulerAngles = glm::radians(entity->GetComponent<TransformComponent>().inspectorEulerHint);
	}

	static void Transform_SetLocalEulerAngles(UUID entityID, glm::vec3* eulerAngles)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		entity->GetComponent<TransformComponent>().rotation = glm::quat(*eulerAngles);
		entity->GetComponent<TransformComponent>().inspectorEulerHint = glm::degrees(*eulerAngles);
	}

	static void Transform_GetLocalScale(UUID entityID, glm::vec3* outScale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		*outScale = entity->GetComponent<TransformComponent>().scale;
	}

	static void Transform_SetLocalScale(UUID entityID, glm::vec3* scale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		entity->GetComponent<TransformComponent>().scale = *scale;
	}

	static void Transform_GetScale(UUID entityID, glm::vec3* outScale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		glm::mat4 worldTransform = entity->GetWorldTransform();
		// 世界矩阵三列的长度就是三个轴的缩放
		*outScale = glm::vec3(
			glm::length(glm::vec3(worldTransform[0])),
			glm::length(glm::vec3(worldTransform[1])),
			glm::length(glm::vec3(worldTransform[2]))
		);
	}



	// Transforms /direction/ from local space to world space.
	static void Transform_VectorTransformSpace(UUID entityID, glm::vec3 vector, glm::vec3* outVector)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		// w = 1：变换点（会吃平移，还会吃缩放）
		// w = 0：变换方向（不吃平移）
		if (Entity* parent = entity->GetEntityParent())
			*outVector = parent->GetWorldRotation() * vector;
		else
			*outVector = vector;
	}

	// Transforms /point/ from local space to world space.
	static void Transform_PointTransformSpace(UUID entityID, glm::vec3 point, glm::vec3* outPoint)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		// w = 1：变换点（会吃平移，还会吃缩放）
		// w = 0：变换方向（不吃平移）
		if (Entity* parent = entity->GetEntityParent())
			*outPoint = entity->GetEntityParent()->GetWorldTransform()* glm::vec4(point, 1.0f);
		else
			*outPoint = point;
	}

	static MonoObject* Transform_GetParent(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		if (entity->GetEntityParent() != nullptr)
			return ScriptInstanceMonoBehaviour(ScriptEngine::GetScriptClass("System.Transform", true), entity->GetEntityParent()->GetUUID()).GetManagedObject();
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
			UUID parentID = parentInstance.GetFieldValue<UUID>(ScriptEngine::GetIDScriptField()->nameHash);
			ScriptEngine::GetEntityUpdateList().push({ EntityUpdateType::MOVE, entityID , parentID });
		}
	}


	static void Transform_GetChildren(UUID entityID, MonoArray** outChildren)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		auto& children = entity->GetEntityChildrenList();
		auto* image = ScriptEngine::GetAppAssemblyImage();

		//MonoClass* monoClass = mono_class_from_name(ScriptEngine::GetMscorlibAssemblyImage(), "System", "UInt64");
		MonoArray* monoArray = mono_array_new(ScriptEngine::GetCoreAssemblyDomain(), ScriptEngine::GetScriptClass(ScriptFieldType::ULong), children.size());

		int i = 0;
		for (auto& child : entity->GetEntityChildrenList())
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
		if (index >= entity->GetEntityChildrenList().size())
			return 0;

		auto it = entity->GetEntityChildrenList().begin();
		std::advance(it, index);

		return ScriptInstanceMonoBehaviour(ScriptEngine::GetScriptClass("Volcano.Transform", true), (*it)->GetUUID()).GetManagedObject();
	}

	static uint64_t Transform_GetChildrenCount(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		return entity->GetEntityChildrenList().size();
	}

	// 销毁所有子节点
	static void Transform_DetachChildren(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		for (auto& child : entity->GetEntityChildrenList())
			ScriptEngine::GetEntityUpdateList().push({ EntityUpdateType::MOVE, child->GetUUID() });
	}


	static void Transform_GetTransform(UUID entityID, MonoObject** outTransform)
	{
		*outTransform = ScriptInstance(ScriptEngine::GetScriptClass("Volcano.Transform", true), entityID).GetManagedInstance();
	}

	void ComponentRegister::TransformComponent_RegisterFunctions()
	{
		VOL_ADD_INTERNAL_CALL(Transform_GetPosition);
		VOL_ADD_INTERNAL_CALL(Transform_SetPosition);
		VOL_ADD_INTERNAL_CALL(Transform_GetLocalPosition);
		VOL_ADD_INTERNAL_CALL(Transform_SetLocalPosition);
		VOL_ADD_INTERNAL_CALL(Transform_GetRotation);
		VOL_ADD_INTERNAL_CALL(Transform_SetRotation);
		VOL_ADD_INTERNAL_CALL(Transform_GetLocalRotation);
		VOL_ADD_INTERNAL_CALL(Transform_SetLocalRotation);
		VOL_ADD_INTERNAL_CALL(Transform_GetEulerAngles);
		VOL_ADD_INTERNAL_CALL(Transform_SetEulerAngles);
		VOL_ADD_INTERNAL_CALL(Transform_GetLocalEulerAngles);
		VOL_ADD_INTERNAL_CALL(Transform_SetLocalEulerAngles);
		VOL_ADD_INTERNAL_CALL(Transform_GetLocalScale);
		VOL_ADD_INTERNAL_CALL(Transform_SetLocalScale);;
		VOL_ADD_INTERNAL_CALL(Transform_GetScale);
		VOL_ADD_INTERNAL_CALL(Transform_VectorTransformSpace);
		VOL_ADD_INTERNAL_CALL(Transform_PointTransformSpace);

		VOL_ADD_INTERNAL_CALL(Transform_GetTransform);
		VOL_ADD_INTERNAL_CALL(Transform_GetParent);
		VOL_ADD_INTERNAL_CALL(Transform_SetParent);
		VOL_ADD_INTERNAL_CALL(Transform_GetChildren);
		VOL_ADD_INTERNAL_CALL(Transform_GetChild);
		VOL_ADD_INTERNAL_CALL(Transform_GetChildrenCount);
		VOL_ADD_INTERNAL_CALL(Transform_DetachChildren);
	}

}