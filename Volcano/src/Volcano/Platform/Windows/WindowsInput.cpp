#include "volpch.h"

#include "Volcano/Core/Input.h"
#include "WindowsWindow.h"
#include "GLFW/glfw3.h"

#include "Volcano/Core/Application.h"

namespace Volcano {

	bool Input::IsKeyPressed(int keycode) 
	{
		auto& window = static_cast<WindowsWindow&>(Application::Get().GetWindow());
		auto state = glfwGetKey(static_cast<GLFWwindow*>(window.GetNativeWindow()), keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonPressed(int button) 
	{
		auto& window = static_cast<WindowsWindow&>(Application::Get().GetWindow());
		auto state = glfwGetMouseButton(static_cast<GLFWwindow*>(window.GetNativeWindow()), button);
		return state == GLFW_PRESS;
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		auto& window = static_cast<WindowsWindow&>(Application::Get().GetWindow());
		double x, y;
		glfwGetCursorPos(static_cast<GLFWwindow*>(window.GetNativeWindow()), &x, &y);
		return { (float)x, (float)y };
	}

	float Input::GetMouseX()
	{
		auto [x, y] = GetMousePosition();
		return (float)x;
	}

	float Input::GetMouseY()
	{
		auto [x, y] = GetMousePosition();
		return (float)y;
	}

}