#pragma once

#include "Core.h"

#include "Window.h"
#include "Volcano/LayerStack.h"
//#include "Events/Event.h"
#include "Volcano/Events/ApplicationEvent.h"

#include "Volcano/Core/Timestep.h"

#include "Volcano/ImGui/ImGuiLayer.h"

namespace Volcano {

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void OnEvent(Event& e);
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
	private:

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;
	private:
		static Application* s_Instance;
	};

	// 在客户端定义
	Application* CreateApplication();
}