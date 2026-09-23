#include "volpch.h"
#include "Application.h"
#include "Volcano/Core/Events/ApplicationEvent.h"
#include "Volcano/Core/Events/MouseEvent.h"
#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Core/Input.h"
#include "Volcano/Core/MouseBuffer.h"
#include "Volcano/Core/AppPath.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include <imgui_internal.h>

namespace Volcano
{

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		if (s_Instance) throw std::runtime_error("Application already exists!");
		s_Instance = this;

		VOL_CORE_TRACE("Window create.");
		m_Window = Window::Create(WindowProperties());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		VOL_CORE_TRACE("Renderer init.");
		Renderer::Init();

		VOL_CORE_TRACE("ImGui create.");
		m_ImGuiLayer = new ImGuiLayer("ImGui");
		PushOverlay(m_ImGuiLayer);

#ifdef _WIN32
		// 把 CWD 设为 exe 所在目录
		wchar_t buf[MAX_PATH];
		GetModuleFileNameW(nullptr, buf, MAX_PATH);
		auto exeAbsolutePath = std::filesystem::path(buf).parent_path();
		AppPath::GetInstance().SetExePath(exeAbsolutePath);
		//std::filesystem::current_path(exeAbsolutePath);
#endif

	}

	Application::~Application() 
	{
		ScriptEngine::Shutdown();
		Renderer::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::SetMouseActive(bool mouseActive)
	{
		m_Window->SetMouseActive(mouseActive);
		m_ImGuiLayer->SetMouseActive(mouseActive);
	}

	// 把方法提交至主线程队列
	void Application::SubmitToMainThread(const std::function<void()>& function)
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		m_MainThreadQueue.push_back(function);
	}

	void Application::ClearConsole()
	{
		m_Window->ClearConsole();
	}

	void Application::Run()
	{
		Time::Init();

		const float fixedDelta = 1.0f / 60.0f; // 固定60帧，0.01666…

		while (m_Running)
		{
			float time = Time::GetSeconds();
			float deltaTime = time - Time::GetLastFrameTime();   // 上一帧的时间到这一帧的间隔时间
			deltaTime = deltaTime > 0.1f ? 0.1f : (float)deltaTime;// 防止 m_Timestep 过大（比如调试断点导致的跳帧）

			if (!m_Minimized && deltaTime >= fixedDelta)
			{
				Time::SetLastFrameTime(time);// 上一帧的时间
				Time::SetDeltaTime(deltaTime);

				SetMouseActive(MouseBuffer::instance().GetOnActive());

				ExecuteMainThreadQueue();

				Renderer::Clear(0.0f, 0.0f, 0.0f, 0.0f);

				for (Layer* layer : m_LayerStack)
					layer->OnUpdate();

				m_ImGuiLayer->Begin();
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
				m_ImGuiLayer->End();

			}
			m_Window->OnUpdate();
		}
	}

	void Application::ExecuteMainThreadQueue()
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		if (m_MainThreadQueue.size() == 0)
			return;

		for (auto& func : m_MainThreadQueue)
			func();

		m_MainThreadQueue.clear();
	}


	void Application::Close()
	{
		m_Running = false;
	}

	// 事件处理
	void Application::OnEvent(Event& e)
	{
		// 创建事件拦截器
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

		// 遍历层，执行各层的OnEvent()，如果某次OnEvent执行结束后event标记已完成，则结束遍历，不在执行后续层的OnEvent
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

	bool Application::OnWindowClose(WindowCloseEvent& e) 
	{
		m_Running = false;
		return true;
	}

}