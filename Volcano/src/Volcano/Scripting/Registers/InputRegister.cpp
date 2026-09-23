#include "volpch.h"
#include "InputRegister.h"

#include "Volcano/Core/Application.h"
#include "Volcano/Core/Input.h"
#include "Volcano/Core/KeyCodes.h"
#include "Volcano/Core/Window.h"
#include "Volcano/Scripting/ScriptEngine.h"

#include <mono/metadata/object.h>


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

	static void Input_GetMouseX(float* outMouseX)
	{
		uint32_t windowWidth = Application::GetInstance().GetWindow().GetWidth();
		float mouseX = Input::GetMouseX();
		*outMouseX = 2.0f * mouseX / (float)windowWidth - 1.0f;
	}

	static void Input_GetMouseY(float* outMouseY)
	{
		uint32_t windowHeight = Application::GetInstance().GetWindow().GetHeight();
		float mouseY = Input::GetMouseY();
		*outMouseY = 1.0f - 2.0f * mouseY / (float)windowHeight;
	}


	static void Input_GetHorizontal(float* outHorizontal)
	{
		float& horizontal = *outHorizontal;
		horizontal = 0.0f;
		if (Input::IsKeyPressed(Key::A))
			horizontal -= 1.0f;
		if (Input::IsKeyPressed(Key::D))
			horizontal += 1.0f;
	}

	static void Input_GetVertical(float* outVertical)
	{
		float& vertical = *outVertical;
		vertical = 0.0f;
		if (Input::IsKeyPressed(Key::W))
			vertical += 1.0f;
		if (Input::IsKeyPressed(Key::S))
			vertical -= 1.0f;
	}

	void InputRegister::RegisterFunctions()
	{
		VOL_ADD_INTERNAL_CALL(Input_IsKeyClicked);
		VOL_ADD_INTERNAL_CALL(Input_IsMouseButtonClicked);
		VOL_ADD_INTERNAL_CALL(Input_IsKeyPressed);
		VOL_ADD_INTERNAL_CALL(Input_IsMouseButtonPressed);
		VOL_ADD_INTERNAL_CALL(Input_GetMousePosition);
		VOL_ADD_INTERNAL_CALL(Input_GetMouseX);
		VOL_ADD_INTERNAL_CALL(Input_GetMouseY);
		VOL_ADD_INTERNAL_CALL(Input_GetHorizontal);
		VOL_ADD_INTERNAL_CALL(Input_GetVertical);
	}
}
