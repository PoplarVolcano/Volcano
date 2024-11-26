#include "volpch.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Scripting/Registers/ComponentRegister.h"

#include "mono/metadata/object.h"
namespace Volcano {

#define VOL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volcano.CameraComponent::" #Name, Name)
	
	void ComponentRegister::CameraComponent_RegisterFunctions()
	{

	}
}