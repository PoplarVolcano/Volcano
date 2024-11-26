#include "volpch.h"

#include "Volcano/Core/Input.h"
#include "WindowsWindow.h"
#include "GLFW/glfw3.h"

#include "Volcano/Core/Application.h"

namespace Volcano {

	std::unordered_set<int> Input::m_ClickedMap;
	std::unordered_set<int> Input::m_ClickedMapBuffer;

	bool Input::IsClicked(int code)
	{
		return m_ClickedMap.find(code) != m_ClickedMap.end();
	}

	bool Input::IsKeyPressed(int keycode)
	{
		auto& window = static_cast<WindowsWindow&>(Application::Get().GetWindow());
		auto state = glfwGetKey(static_cast<GLFWwindow*>(window.GetNativeWindow()), keycode);
		return state == GLFW_PRESS;
	}

	bool Input::IsMouseButtonPressed(int mouseButton) 
	{
		auto& window = static_cast<WindowsWindow&>(Application::Get().GetWindow());
		auto state = glfwGetMouseButton(static_cast<GLFWwindow*>(window.GetNativeWindow()), mouseButton);
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

	void Input::UpdateClickMap()
	{
		m_ClickedMap.clear();
		m_ClickedMap = m_ClickedMapBuffer;
		m_ClickedMapBuffer.clear();
	}

	void Input::Click(int code)
	{
		m_ClickedMapBuffer.insert(code);
	}

}