#pragma once

#include "Volcano/Core/Base.h"
#include "Volcano/Core/Timestep.h"
#include "Volcano/Core/Window.h"
#include "Volcano/Core/LayerStack.h"

#include "Volcano/Core/Events/ApplicationEvent.h"

#include "Volcano/ImGui/ImGuiLayer.h"
#include <Volcano/Renderer/VertexArray.h>
#include <Volcano/Renderer/Framebuffer.h>
#include <Volcano/Renderer/Texture.h>

namespace Volcano {

	struct ApplicationProps
	{
		std::string Name;
		uint32_t WindowWidth, WindowHeight;
	};

	class Application
	{
	public:
		Application(const ApplicationProps& props = { "Hazel Engine", 1280, 720 });
		virtual ~Application();

		void Run();

		void Close();

		virtual void OnInit() {}
		virtual void OnShutdown() {}
		virtual void OnUpdate(Timestep ts) {}

		virtual void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void RenderImGui();

		inline Window& GetWindow() { return *m_Window; }
		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		float GetTime() const;
		static inline Application& Get() { return *s_Instance; }
	private:
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowClose(WindowCloseEvent& e);
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true, m_Minimized = false;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;
		Timestep m_Timestep;

		float m_LastFrameTime = 0.0f;

		static Application* s_Instance;
	};

	// 在客户端定义
	Application* CreateApplication();
}