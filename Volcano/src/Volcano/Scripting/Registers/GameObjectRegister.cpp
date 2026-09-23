#include "volpch.h"
#include "GameObjectRegister.h"

#include "Volcano/Scripting/ScriptEngine.h"

#include <mono/metadata/object.h>

namespace Volcano
{

#define VOL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volcano.InternalCalls::" #Name, Name)


	static void GameObject_GetName(uint64_t entityID, MonoString** outName)
	{
		std::string name = ScriptEngine::GetSceneContext()->GetEntityByUUID(entityID)->GetName();
		*outName = mono_string_new(ScriptEngine::GetAppAssemblyDomain(), name.c_str());
	}

	static void GameObject_GetGameObject(UUID entityID, MonoObject** outGameObject)
	{
		*outGameObject = ScriptInstance(ScriptEngine::GetScriptClassGameObject(), entityID).GetManagedInstance();
	}

	void GameObjectRegister::RegisterFunctions()
	{
		VOL_ADD_INTERNAL_CALL(GameObject_GetName);
		VOL_ADD_INTERNAL_CALL(GameObject_GetGameObject);
	}
}