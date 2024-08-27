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

	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			VOL_CORE_ASSERT(index < Count);
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::string Name = "Volcano Application";
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void Close();

		virtual void OnUpdate(Timestep ts) {}

		virtual void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }
		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		static inline Application& Get() { return *s_Instance; }
		const ApplicationSpecification& GetSpecification() const { return m_Specification; }
		void Run();
	private:
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowClose(WindowCloseEvent& e);
	private:
		ApplicationSpecification m_Specification;
		Scope<Window> m_Window;
		bool m_Running = true, m_Minimized = false;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;
		Timestep m_Timestep;

		float m_LastFrameTime = 0.0f;

		static Application* s_Instance;
	};

	// 在客户端定义
	Application* CreateApplication(ApplicationCommandLineArgs args);
}