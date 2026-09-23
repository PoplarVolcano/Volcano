#include "volpch.h"
#include "MonoBehaviourRegister.h"

#include "Volcano/Scripting/ScriptEngine.h"

#include <mono/metadata/object.h>

namespace Volcano
{

#define VOL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volcano.MonoBehaviour::" #Name, Name)



	//==================================================================================================
	//                                    Invoke
	//==================================================================================================

	static void MonoBehaviour_InvokeDelayed(UUID entityID, MonoString* methodName, float time, float repeatRate)
	{
		Ref<ScriptInstanceMonoBehaviour> instance = ScriptEngine::GetEntityScriptInstance(entityID);
		const char* methodNameCStr = mono_string_to_utf8(methodName);
		if (instance->GetScriptClass()->HasMethod(methodNameCStr))
		{
			auto& method = instance->GetScriptClass()->GetMethod(methodNameCStr);
			auto& list = ScriptEngine::GetEntityInvokeDelayedListBuffer();
			list.push_back({ entityID, instance, method, time, repeatRate, Timer() });
		}
	}

	static bool MonoBehaviour_IsInvoking(UUID entityID, MonoString* methodName)
	{
		auto& list = ScriptEngine::GetEntityInvokeDelayedList();
		const char* methodNameCStr = mono_string_to_utf8(methodName);
		for (auto& data : list)
		{
			if (data.id == entityID)
				if (data.method.Name == methodNameCStr)
					return true;
		}
		return false;
	}

	static bool MonoBehaviour_IsInvokingAll(UUID entityID)
	{
		auto& list = ScriptEngine::GetEntityInvokeDelayedList();
		for (auto& data : list)
			if (data.id == entityID)
				return true;
		return false;
	}

	static void MonoBehaviour_CancelInvoke(UUID entityID, MonoString* methodName)
	{
		const char* methodNameCStr = mono_string_to_utf8(methodName);
		ScriptEngine::RemoveEntityInvokeDelayed(entityID, methodNameCStr);
	}

	static void MonoBehaviour_CancelInvokeAll(UUID entityID)
	{
		auto& list = ScriptEngine::GetEntityInvokeDelayedList();
		for (auto it = list.begin(); it != list.end(); )
		{
			if (it->id == entityID)
				it = list.erase(it);
			else
				it++;
		}
	}


	void MonoBehaviourRegister::RegisterFunctions()
	{
		VOL_ADD_INTERNAL_CALL(MonoBehaviour_InvokeDelayed);
		VOL_ADD_INTERNAL_CALL(MonoBehaviour_IsInvoking);
		VOL_ADD_INTERNAL_CALL(MonoBehaviour_IsInvokingAll);
		VOL_ADD_INTERNAL_CALL(MonoBehaviour_CancelInvoke);
		VOL_ADD_INTERNAL_CALL(MonoBehaviour_CancelInvokeAll);

	}
}