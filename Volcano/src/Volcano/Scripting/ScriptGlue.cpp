#include "volpch.h"
#include "ScriptGlue.h"

#include "ScriptEngine.h"

#include "Volcano/Core/UUID.h"
#include "Volcano/Core/Timer.h"

#include "Volcano/Scene/Scene.h"
#include "Volcano/Scene/Entity.h"

#include "Volcano/Core/Application.h"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

#include <Volcano/Core/MouseBuffer.h>

#include "Volcano/Scripting/ComponentRegister.h"
#include "Volcano/Scripting/MathFRegister.h"
#include "Volcano/Scripting/InputRegister.h"

namespace Volcano {

	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

	// mono_add_internal_call 添加内部调用 
	// C#的Volcano命名空间下的InternalCalls类的#Name函数  被C++的Name给定义
#define VOL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volcano.InternalCalls::" #Name, Name)

	static MonoObject* GetScriptInstance(UUID entityID)
	{
		return ScriptEngine::GetManagedInstance(entityID);
	}

	// ==========================================Entity=============================================================

	static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		VOL_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end());
		return s_EntityHasComponentFuncs.at(managedType)(*entity.get());
	}

	static uint64_t Entity_FindEntityByName(MonoString* name)
	{
		char* nameCStr = mono_string_to_utf8(name);

		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->FindEntityByName(nameCStr);
		mono_free(nameCStr);

		if (!entity)
			return 0;

		return entity->GetUUID();
	}

	static uint64_t Entity_GetParent(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		if (entity->GetEntityParent() == nullptr)
			return 0;

		return entity->GetEntityParent()->GetUUID();
	}

	static void Entity_GetChildren(UUID entityID, MonoArray** outChildren)
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
		for (auto& [name, child] : entity->GetEntityChildren())
		{
			mono_array_set(monoArray, uint64_t, i, child->GetUUID()); // 设置第i个元素为i + 1
			i++;
		}
		*outChildren = monoArray;
	}

	static UUID Entity_GetChild(UUID entityID, uint64_t index)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);
		if (index >= entity->GetEntityChildren().size())
			return 0;
		auto it = entity->GetEntityChildren().begin();
		std::advance(it, index);
		return it->second->GetUUID();
	}

	static uint64_t Entity_GetChildrenCount(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(entityID);
		VOL_CORE_ASSERT(entity);

		return entity->GetEntityChildren().size();
	}

	static bool Entity_IsEntityExist(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		return scene->GetEntityByUUID(entityID) != nullptr;
	}

	// =======================================MouseBuffer================================================

	static bool MouseBuffer_GetMouseOnActive()
	{
		return MouseBuffer::instance().GetOnActive();
	}

	static void MouseBuffer_SetMouseOnActive(float* onActive)
	{
		return MouseBuffer::instance().SetOnActive(*onActive);
	}

	// ======================================Debug======================================================

	static void Debug_Trace(MonoString* message)
	{
		const char* messageCStr = mono_string_to_utf8(message);
		VOL_TRACE(messageCStr);
		//std::cout << messageCStr << std::endl;
		// 记得释放Mono字符串所占的内存
		mono_free((void*)messageCStr);
	}

	static void Debug_Info(MonoString* message)
	{
		const char* messageCStr = mono_string_to_utf8(message);
		VOL_INFO(messageCStr);
		mono_free((void*)messageCStr);
	}

	static void Debug_Warn(MonoString* message)
	{
		const char* messageCStr = mono_string_to_utf8(message);
		VOL_WARN(messageCStr);
		mono_free((void*)messageCStr);
	}

	static void Debug_Error(MonoString* message)
	{
		const char* messageCStr = mono_string_to_utf8(message);
		VOL_ERROR(messageCStr);
		mono_free((void*)messageCStr);
	}

	// ======================================Debug======================================================

	static void Application_GetTargetFrameRate(int* outTargetFrameRate)
	{
		*outTargetFrameRate = Application::Get().GetTargetFrameRate();
	}

	static void Application_SetTargetFrameRate(int targetFrameRate)
	{
		Application::Get().SetTargetFrameRate(targetFrameRate);
	}

	//====================================Invoke==========================================================

	static void InvokeDelayed(UUID entityID, MonoString* methodName, float time, float repeatRate)
	{
		Ref<ScriptInstance> instance = ScriptEngine::GetEntityScriptInstance(entityID);
		const char* methodNameCStr = mono_string_to_utf8(methodName);
		if (instance->GetScriptClass()->HasMethod(methodNameCStr))
		{
			auto& method = instance->GetScriptClass()->GetMethod(methodNameCStr);
			auto& list = ScriptEngine::GetEntityInvokeDelayedList();
			list.push_back({ entityID, instance, method, time, repeatRate, Timer() });
		}
	}

	static bool IsInvoking(UUID entityID, MonoString* methodName)
	{
		auto& list = ScriptEngine::GetEntityInvokeDelayedList();
		const char* methodNameCStr = mono_string_to_utf8(methodName);
		for (auto& data : list)
		{
			if (data.id = entityID)
				if (data.method.Name == methodNameCStr)
					return true;
		}
		return false;
	}

	static bool IsInvokingAll(UUID entityID)
	{
		auto& list = ScriptEngine::GetEntityInvokeDelayedList();
		for (auto& data : list)
			if (data.id = entityID)
				return true;
		return false;
	}

	static void CancelInvoke(UUID entityID, MonoString* methodName)
	{
		auto& list = ScriptEngine::GetEntityInvokeDelayedList();
		const char* methodNameCStr = mono_string_to_utf8(methodName);
		for (auto it = list.begin(); it != list.end(); )
		{
			if (it->id = entityID && it->method.Name == methodNameCStr)
				it = list.erase(it);
			else
				it++;
		}
	}

	static void CancelInvokeAll(UUID entityID)
	{
		auto& list = ScriptEngine::GetEntityInvokeDelayedList();
		for (auto it = list.begin(); it != list.end(); )
		{
			if (it->id = entityID)
				it = list.erase(it);
			else
				it++;
		}
	}

	// ============================================Instantiate==========================================

	// 在Scene目录下复制实例
	// original: C#Entity实例
	static MonoObject* InstantiateSingle(MonoObject* original, glm::vec3 position, glm::quat rotation)
	{
		ScriptInstance instance = ScriptInstance(original);
		UUID id = instance.GetFieldValue<UUID>("ID");

		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(id);
		VOL_CORE_ASSERT(entity);
		;
		Ref<Entity> resultEntity = scene->DuplicateEntity(entity);
		if (resultEntity != nullptr)
		{
			resultEntity->GetComponent<TransformComponent>().Translation = position;
			resultEntity->GetComponent<TransformComponent>().Rotation = glm::eulerAngles(rotation);
			return ScriptInstance(ScriptEngine::GetEntityClass(), *resultEntity.get()).GetManagedObject();
		}
		else
			return nullptr;
	}

	// 在parent目录下复制实例
	// parent: C#TransformComponent实例
	// original: C#Entity实例
	static MonoObject* InstantiateSingleWithParent(MonoObject* original, MonoObject* parent, glm::vec3 position, glm::quat rotation)
	{
		ScriptInstance instance = ScriptInstance(original);
		UUID id = instance.GetFieldValue<UUID>("ID");

		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(id);
		VOL_CORE_ASSERT(entity);

		ScriptInstance parentInstance = ScriptInstance(parent);
		UUID parentId = instance.GetFieldValue<UUID>("Entity");

		Ref<Entity> parentEntity = scene->GetEntityByUUID(parentId);
		VOL_CORE_ASSERT(entity);

		Ref<Entity> resultEntity = scene->DuplicateEntity(entity, parentEntity);
		if (resultEntity != nullptr)
		{
			resultEntity->GetComponent<TransformComponent>().Translation = position;
			resultEntity->GetComponent<TransformComponent>().Rotation = glm::eulerAngles(rotation);
			return ScriptInstance(ScriptEngine::GetEntityClass(), *resultEntity.get()).GetManagedObject();
		}
		else
			return nullptr;
	}

	static MonoObject* CloneSingle(MonoObject* original)
	{
		ScriptInstance instance = ScriptInstance(original);
		UUID id = instance.GetFieldValue<UUID>("ID");

		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(id);
		VOL_CORE_ASSERT(entity);

		Ref<Entity> resultEntity = scene->DuplicateEntity(entity);
		if (resultEntity != nullptr)
			return ScriptInstance(ScriptEngine::GetEntityClass(), *resultEntity.get()).GetManagedObject();
		else
			return nullptr;
	}

	static MonoObject* CloneSingleWithParent(MonoObject* original, MonoObject* parent, bool instantiateInWorldSpace)
	{
		ScriptInstance instance = ScriptInstance(original);
		UUID id = instance.GetFieldValue<UUID>("ID");

		Scene* scene = ScriptEngine::GetSceneContext();
		VOL_CORE_ASSERT(scene);
		Ref<Entity> entity = scene->GetEntityByUUID(id);
		VOL_CORE_ASSERT(entity);

		ScriptInstance parentInstance = ScriptInstance(parent);
		UUID parentId = instance.GetFieldValue<UUID>("Entity");

		Ref<Entity> parentEntity = scene->GetEntityByUUID(parentId);
		VOL_CORE_ASSERT(entity);

		Ref<Entity> resultEntity = scene->DuplicateEntity(entity, parentEntity);
		if (resultEntity != nullptr)
			return ScriptInstance(ScriptEngine::GetEntityClass(), *resultEntity.get()).GetManagedObject();
		else
			return nullptr;
	}

	// ================================================Register=========================================
	template<typename... Component>
	static void RegisterComponent()
	{
		([]()
			{
				std::string_view typeName = typeid(Component).name();
				size_t pos = typeName.find_last_of(':');
				std::string_view structName = typeName.substr(pos + 1);
				std::string managedTypename = fmt::format("Volcano.{}", structName);

				MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
				if (!managedType)
				{
					VOL_CORE_ERROR("Could not find component type {}", managedTypename);
					return;
				}
				s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };
			}(), ...);
	}

	template<typename... Component>
	static void RegisterComponent(ComponentGroup<Component...>)
	{
		s_EntityHasComponentFuncs.clear();
		RegisterComponent<Component...>();
	}

	void ScriptGlue::RegisterComponents()
	{
		RegisterComponent(AllComponents{});
	}

	void ScriptGlue::RegisterFunctions()
	{
		VOL_ADD_INTERNAL_CALL(GetScriptInstance);
		VOL_ADD_INTERNAL_CALL(Entity_HasComponent);
		VOL_ADD_INTERNAL_CALL(Entity_FindEntityByName);
		VOL_ADD_INTERNAL_CALL(Entity_GetParent);
		VOL_ADD_INTERNAL_CALL(Entity_GetChildren);
		VOL_ADD_INTERNAL_CALL(Entity_GetChildrenCount);
		VOL_ADD_INTERNAL_CALL(Entity_IsEntityExist);
		
		ComponentRegister::TransformComponent_RegisterFunctions();
		ComponentRegister::CameraComponent_RegisterFunctions();
		ComponentRegister::Rigidbody2DComponent_RegisterFunctions();
		MathFRegister::MathF_RegisterFunctions();
		InputRegister::Input_RegisterFunctions();

		VOL_ADD_INTERNAL_CALL(MouseBuffer_GetMouseOnActive);
		VOL_ADD_INTERNAL_CALL(MouseBuffer_SetMouseOnActive);

		VOL_ADD_INTERNAL_CALL(Debug_Trace);
		VOL_ADD_INTERNAL_CALL(Debug_Info);
		VOL_ADD_INTERNAL_CALL(Debug_Warn);
		VOL_ADD_INTERNAL_CALL(Debug_Error);

		VOL_ADD_INTERNAL_CALL(Application_GetTargetFrameRate);
		VOL_ADD_INTERNAL_CALL(Application_SetTargetFrameRate);

		VOL_ADD_INTERNAL_CALL(InvokeDelayed);
		VOL_ADD_INTERNAL_CALL(IsInvoking);
		VOL_ADD_INTERNAL_CALL(IsInvokingAll);
		VOL_ADD_INTERNAL_CALL(CancelInvoke);
		VOL_ADD_INTERNAL_CALL(CancelInvokeAll);

		VOL_ADD_INTERNAL_CALL(InstantiateSingle);
		VOL_ADD_INTERNAL_CALL(InstantiateSingleWithParent);
		VOL_ADD_INTERNAL_CALL(CloneSingle);
		VOL_ADD_INTERNAL_CALL(CloneSingleWithParent);
	}

}