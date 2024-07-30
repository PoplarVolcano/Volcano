#pragma once

#include "Core.h"

#include "Window.h"
#include "Volcano/LayerStack.h"
#include "Events/Event.h"
#include "Volcano/Events/ApplicationEvent.h"

#include "Volcano/ImGui/ImGuiLayer.h"

namespace Volcano {

	class VOLCANO_API Application
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
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;

		static Application* s_Instance;
	};

	// �ڿͻ��˶���
	Application* CreateApplication();
}