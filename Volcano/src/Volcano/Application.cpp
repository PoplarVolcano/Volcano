#include "volpch.h"
#include "Application.h"

#include <glad/glad.h>

namespace Volcano {

// bind������C++11��׼�е�һ������ģ�壬���ڽ�������һ���������һ��
// ����һ���µĿɵ��ö��󡣿�������ʵ�ֺ����������������󶨵ȹ��ܡ�
// bind�����ķ���ֵ��һ���ɵ��ö��󣬿���ͨ�����øö�����ִ�а󶨵ĺ�����
// std::placeholders �� C++ ��׼���е�һ�������ռ䣬��������һ�������ռλ������������ std::bind ����һ��ʹ�á�
// ��Щռλ�������������ڰ󶨺���ʱ��ʾĳЩ������δָ���ģ������Ժ��ṩ��Щ������ֵ��
// std::placeholders ��������ռλ����
// std::placeholders::_1����ʾ��һ��δָ���Ĳ�����
// std::placeholders::_2����ʾ�ڶ���δָ���Ĳ�����
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;
	Application::Application() {
		VOL_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		//�� OnEvent ���� m_Window.m_Data.EventCallback
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
	}

	Application::~Application() {
	}

	void Application::PushLayer(Layer* layer) {
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer) {
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	//�¼�����
	void Application::OnEvent(Event& e) {
		//�����¼�������
		EventDispatcher dispatcher(e);

		//���عرմ����¼��������رմ��ں���
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		//����¼�toString()
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

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}

}