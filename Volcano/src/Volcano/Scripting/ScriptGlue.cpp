#include "volpch.h"
#include "ScriptGlue.h"

#include "ScriptEngine.h"

#include "Volcano/Core/UUID.h"

#include "Volcano/Scene/Scene.h"
#include "Volcano/Scene/Entity.h"

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

	// =======================================MouseBuffer================================================

	static bool MouseBuffer_GetMouseOnActive()
	{
		return MouseBuffer::instance().GetOnActive();
	}

	static void MouseBuffer_SetMouseOnActive(float* onActive)
	{
		return MouseBuffer::instance().SetOnActive(*onActive);
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

		ComponentRegister::TransformComponent_RegisterFunctions();
		ComponentRegister::CameraComponent_RegisterFunctions();
		ComponentRegister::Rigidbody2DComponent_RegisterFunctions();
		MathFRegister::MathF_RegisterFunctions();
		InputRegister::Input_RegisterFunctions();

		VOL_ADD_INTERNAL_CALL(MouseBuffer_GetMouseOnActive);
		VOL_ADD_INTERNAL_CALL(MouseBuffer_SetMouseOnActive);
	}

}