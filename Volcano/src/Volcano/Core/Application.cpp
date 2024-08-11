#include "volpch.h"
#include "Application.h"
#include "Volcano/Core/Application.h"

#include "Volcano/Renderer/Renderer.h"
#include <imgui.h>

namespace Volcano {
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationProps& props)
	{
		VOL_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create(WindowProps(props.Name, props.WindowWidth, props.WindowHeight)));
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		m_Window->SetVSync(true);

		m_ImGuiLayer = new ImGuiLayer("ImGui");
		PushOverlay(m_ImGuiLayer);

		Renderer::Init();
		Renderer::WaitAndRender();

	}
	Application::~Application()
	{
	}

	void Application::RenderImGui()
	{
		//��ImGui��ˢ�·ŵ�APP�У���Update�ֿ�
		m_ImGuiLayer->Begin();

		for (Layer* layer : m_LayerStack)
			layer->OnImGuiRender();

		m_ImGuiLayer->End();

	}

	void Application::Run()
	{
		OnInit();
		while (m_Running)
		{
			if (!m_Minimized)
			{
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(m_Timestep);

				// Render ImGui on render thread
				Application* app = this;
				Renderer::Submit([app]() { app->RenderImGui(); });

				Renderer::WaitAndRender();
			}
			m_Window->OnUpdate();

			float time = GetTime();
			m_Timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;
		}
		OnShutdown();
	}

	//�¼�����
	void Application::OnEvent(Event& e) {
		//�����¼�������
		EventDispatcher dispatcher(e);

		//���عرմ����¼��������رմ��ں���
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		//�����޸Ĵ��ڳߴ��¼��������޸Ĵ��ڳߴ纯��
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

		//���ڳߴ�ı�ʱ��ͼ�����ı�
		Renderer::OnWindowResize(width, height);
		return false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}

	float Application::GetTime() const
	{
		return m_Window->GetTime();
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