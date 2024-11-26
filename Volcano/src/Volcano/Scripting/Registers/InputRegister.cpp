#include "volpch.h"
#include "InputRegister.h"
#include "Volcano/Core/KeyCodes.h"
#include "Volcano/Core/Input.h"

#include "mono/metadata/object.h"

namespace Volcano {

#define VOL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volcano.Input::" #Name, Name)

	static bool Input_IsKeyClicked(KeyCode keycode)
	{
		return Input::IsClicked(keycode);
	}

	static bool Input_IsMouseButtonClicked(int mouseButton)
	{
		return Input::IsClicked(mouseButton);
	}

	static bool Input_IsKeyPressed(KeyCode keycode)
	{
		return Input::IsKeyPressed(keycode);
	}

	static bool Input_IsMouseButtonPressed(int mouseButton)
	{
		return Input::IsMouseButtonPressed(mouseButton);
	}

	static void Input_GetMousePosition(std::pair<float, float>* outMousePosition)
	{
		*outMousePosition = Input::GetMousePosition();
	}

	static float Input_GetMouseX()
	{
		return Input::GetMouseX();
	}

	static float Input_GetMouseY()
	{
		return Input::GetMouseY();
	}


	void InputRegister::Input_RegisterFunctions()
	{
		VOL_ADD_INTERNAL_CALL(Input_IsKeyClicked);
		VOL_ADD_INTERNAL_CALL(Input_IsMouseButtonClicked);
		VOL_ADD_INTERNAL_CALL(Input_IsKeyPressed);
		VOL_ADD_INTERNAL_CALL(Input_IsMouseButtonPressed);
		VOL_ADD_INTERNAL_CALL(Input_GetMousePosition);
		VOL_ADD_INTERNAL_CALL(Input_GetMouseX);
		VOL_ADD_INTERNAL_CALL(Input_GetMouseY);
	}
}
