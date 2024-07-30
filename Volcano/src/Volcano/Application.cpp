#include "volpch.h"
#include "Application.h"

#include <glad/glad.h>

#include "Input.h"

namespace Volcano {

	// bind函数是C++11标准中的一个函数模板，用于将函数和一组参数绑定在一起，
	// 生成一个新的可调用对象。可以用于实现函数适配器、参数绑定等功能。
	// bind函数的返回值是一个可调用对象，可以通过调用该对象来执行绑定的函数。
	// std::placeholders 是 C++ 标准库中的一个命名空间，它包含了一组特殊的占位符对象，用于与 std::bind 函数一起使用。
	// 这些占位符对象允许你在绑定函数时表示某些参数是未指定的，并在稍后提供这些参数的值。
	// std::placeholders 中有以下占位符：
	// std::placeholders::_1：表示第一个未指定的参数。
	// std::placeholders::_2：表示第二个未指定的参数。
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application() {
		VOL_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		// 回调函数
		// 将 OnEvent 赋给 m_Window.m_Data.EventCallback
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application() {
	}

	//事件处理
	void Application::OnEvent(Event& e) {
		//创建事件拦截器
		EventDispatcher dispatcher(e);

		//拦截关闭窗口事件，触发关闭窗口函数
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		//输出事件toString()
		//VOL_CORE_TRACE("{0}", e);

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); ) {
			(*--it)->OnEvent(e);
			if (e.m_Handled)
				break;
		}
	}

	void Application::Run() {

		{
			WindowResizeEvent e(1280, 720);

			if(e.IsInCategory(EventCategoryApplication)){
				VOL_TRACE(e);
			}

			if (e.IsInCategory(EventCategoryInput)) {
				VOL_TRACE(e);
			}
		}

		while (m_Running) {
			glClearColor(1, 0, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			//将ImGui的刷新放到APP中，与Update分开
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack) {
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}

	void Application::PushLayer(Layer* layer) {
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer) {
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

}