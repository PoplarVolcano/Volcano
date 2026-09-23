#pragma once

#include "Volcano/Core/Core.h"
#include "Volcano/Core/Events/Event.h"
#include "Volcano/Core/Events/KeyEvent.h"
#include "Volcano/Core/Events/MouseEvent.h"
#include "Volcano/Core/Events/ApplicationEvent.h"
#include "Volcano/Core/Window.h"

#include "Volcano/Core/LayerStack.h"
#include "Volcano/ImGui/ImGuiLayer.h"

#include "Volcano/Core/Time.h"
#include "Volcano/Renderer/VertexArray.h"

int main(int argc, char** argv);

namespace Volcano
{
	// 应用程序指令行参数
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

	// 应用程序规格
	struct ApplicationSpecification
	{
		std::string Name = "Volcano Application";
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};

	class VOL_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Close();

		virtual void OnEvent(Event& e);

		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowClose(WindowCloseEvent& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }
		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		static inline Application& GetInstance() { return *s_Instance; }
		const ApplicationSpecification& GetSpecification() const { return m_Specification; }

		void SetMouseActive(bool mouseActive);

		void SubmitToMainThread(const std::function<void()>& function);
		void ClearConsole();

	private:
		void Run();

		// 执行主线程方法队列
		void ExecuteMainThreadQueue();
	private:
		ApplicationSpecification m_Specification;
		bool m_Running = true;
		bool m_Minimized = false;

		Scope<Window> m_Window;

		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex; // 主线程队列互斥锁

	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// 在客户端定义
	Application* CreateApplication();
}
