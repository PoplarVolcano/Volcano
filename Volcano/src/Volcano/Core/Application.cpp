#include "volpch.h"
#include "Volcano/Core/Application.h"
#include "Volcano/Core/Log.h"
#include "Volcano/Core/Input.h"
#include "Volcano/Core/MouseBuffer.h"
#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Utils/PlatformUtils.h"
//#include "Volcano/Renderer/Model.h"

namespace Volcano {
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification) 
	{
		VOL_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		// 把工作路径放入ApplicationSpecification
		if (!m_Specification.WorkingDirectory.empty())
			std::filesystem::current_path(m_Specification.WorkingDirectory);

		m_Window = Window::Create(WindowProps(m_Specification.Name));
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer("ImGui");
		PushOverlay(m_ImGuiLayer);

	}
	Application::~Application()
	{
		ScriptEngine::Shutdown();
		Renderer::Shutdown();
	}

	void Application::Run()
	{
		while (m_Running)
		{
			SetMouseActive(MouseBuffer::instance().GetOnActive());

			float time = Time::GetTime();
			m_Timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			ExecuteMainThreadQueue();

			if (!m_Minimized)
			{
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(m_Timestep);

				m_ImGuiLayer->Begin();
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
				m_ImGuiLayer->End();
			}
			m_Window->OnUpdate();

		}
	}

	void Application::Close() 
	{
		m_Running = false;
	}

	void Application::SetMouseActive(bool mouseOnActive)
	{
		m_Window->SetMouseActive(mouseOnActive);
		m_ImGuiLayer->SetMouseActive(mouseOnActive);
	}

	// 把方法提交至主线程队列
	void Application::SubmitToMainThread(const std::function<void()>& function)
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		m_MainThreadQueue.emplace_back(function);
	}

	//事件处理
	void Application::OnEvent(Event& e) 
	{
		//创建事件拦截器
		EventDispatcher dispatcher(e);

		//拦截关闭窗口事件，触发关闭窗口函数
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		//拦截修改窗口尺寸事件，触发修改窗口尺寸函数
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); ) {
			(*--it)->OnEvent(e);
			if (e.m_Handled)
				break;
		}
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		int width = e.GetWidth(), height = e.GetHeight();
		if (width == 0 || height == 0)
		{
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;

		//窗口尺寸改变时，图像跟随改变
		Renderer::OnWindowResize(width, height);
		return false;
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

	void Application::ExecuteMainThreadQueue()
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		for (auto& func : m_MainThreadQueue)
			func();

		m_MainThreadQueue.clear();
	}
}