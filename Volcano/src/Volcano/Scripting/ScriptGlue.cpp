#include "volpch.h"
#include "ScriptGlue.h"

#include "Volcano/Core/Time.h"
#include "Volcano/Core/MouseBuffer.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Scene/Components.h"

#include "Volcano/Scripting/Registers/ComponentRegister.h"
#include "Volcano/Scripting/Registers/GameObjectRegister.h"
#include "Volcano/Scripting/Registers/InputRegister.h"
#include "Volcano/Scripting/Registers/MathFloatRegister.h"
#include "Volcano/Scripting/Registers/MonoBehaviourRegister.h"
#include "Volcano/Scripting/Registers/TimeRegister.h"

#include <mono/jit/jit.h>
#include <mono/metadata/object.h>

namespace Volcano
{
	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

	// mono_add_internal_call 添加内部调用 
	// C#的 Volcano 命名空间下的 InternalCalls 类的 #Name 函数 被C++的 Name 给定义
#define VOL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volcano.InternalCalls::" #Name, Name)

	// =======================================================================================
	//                                MouseBuffer
	// =======================================================================================

	static bool MouseBuffer_GetMouseOnActive()
	{
		return MouseBuffer::instance().GetOnActive();
	}

	static void MouseBuffer_SetMouseOnActive(bool onActive)
	{
		return MouseBuffer::instance().SetOnActive(onActive);
	}

	// ==============================================================================================
	//                                  Debug
	//==================================================================================================

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

	//===============================================================================================
	//                                  EntityUpdate
	//===============================================================================================


	static void GameObject_Destroy(MonoObject* original, float time)
	{
		ScriptInstance instance(original);
		UUID id = instance.GetFieldValue<UUID>(ScriptEngine::GetIDScriptField()->nameHash);

		ScriptEngine::GetEntityUpdateList().push({ EntityUpdateType::DESTROY, id });
	}

	// =========================================================================================
	//                                Component Register
	// =========================================================================================

	/*
	将 C++ 的组件类型（Component）与 C# 的托管类型建立映射，并注册一个查询函数，
	以便在 C# 脚本中判断一个实体（Entity）是否拥有该组件

	可变参数模板，接受任意多个类型参数（Component 表示参数包）
	内部使用 C++17 的折叠表达式 ( lambda(), ... )，它会依次展开参数包，对每个 Component 类型执行一次 lambda 表达式
	因此，RegisterComponent<Transform, Rigidbody, ...>() 将依次处理 Transform、Rigidbody 等类型
	template<typename... Component>
	static void RegisterComponent()
	{
	    ([]()
	    {
	        // 对每个 Component 类型执行以下操作
	        ...
	    }(), ...);
    }

	typeid(Component).name() 返回编译器修饰后的类型名称（如"class Volcano::Transform"）。
	find_last_of(':') 找到最后一个冒号（命名空间分隔符），取出最后的类名（例如 "Transform"）。
	substr(0, length() - 9) 硬编码去掉末尾 9 个字符。这通常是针对 MSVC 特定修饰（例如 "Transform" 后面可能跟着 " 或空格等）

	*/
	template<typename... Component>
	static void RegisterComponent()
	{
		([]()
			{
				std::string_view typeName = typeid(Component).name();
				size_t pos = typeName.find_last_of(':');
				std::string_view structName = typeName.substr(pos + 1);
				structName = structName.substr(0, structName.length() - 9);
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
		RegisterComponent<Component...>();
	}

	void ScriptGlue::RegisterComponents()
	{
		RegisterComponent(AllComponents{});
	}


	void ScriptGlue::RegisterFunctions()
	{
		VOL_ADD_INTERNAL_CALL(MouseBuffer_GetMouseOnActive);
		VOL_ADD_INTERNAL_CALL(MouseBuffer_SetMouseOnActive);

		VOL_ADD_INTERNAL_CALL(Debug_Trace);
		VOL_ADD_INTERNAL_CALL(Debug_Info);
		VOL_ADD_INTERNAL_CALL(Debug_Warn);
		VOL_ADD_INTERNAL_CALL(Debug_Error);

		VOL_ADD_INTERNAL_CALL(GameObject_Destroy);

		ComponentRegister::TransformComponent_RegisterFunctions();
		GameObjectRegister::RegisterFunctions();
		InputRegister::RegisterFunctions();
		MathFloatRegister::RegisterFunctions();
		MonoBehaviourRegister::RegisterFunctions();
		TimeRegister::RegisterFunctions();
	}

}