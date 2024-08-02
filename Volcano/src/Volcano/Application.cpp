#include "volpch.h"
#include "Application.h"

#include <glad/glad.h>

#include "Input.h"

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

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case Volcano::ShaderDataType::Float:     return GL_FLOAT;
			case Volcano::ShaderDataType::Float2:    return GL_FLOAT;
			case Volcano::ShaderDataType::Float3:    return GL_FLOAT;
			case Volcano::ShaderDataType::Float4:    return GL_FLOAT;
			case Volcano::ShaderDataType::Mat3:      return GL_FLOAT;
			case Volcano::ShaderDataType::Mat4:      return GL_FLOAT;
			case Volcano::ShaderDataType::Int:       return GL_INT;
			case Volcano::ShaderDataType::Int2:      return GL_INT;
			case Volcano::ShaderDataType::Int3:      return GL_INT;
			case Volcano::ShaderDataType::Int4:      return GL_INT;
			case Volcano::ShaderDataType::Bool:      return GL_BOOL;
		}

		VOL_CORE_ASSERT(false, "Unknow ShaderDataType!");
		return 0;
	}
	
	Application::Application() {
		VOL_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		// �ص�����
		// �� OnEvent ���� m_Window.m_Data.EventCallback
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);


		m_VertexArray.reset(VertexArray::Create());

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
		};

		std::shared_ptr<VertexBuffer> vertexBuffer;
		vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color" }
		};

		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<IndexBuffer> indexBuffer;
		indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		//��ɫ��
		//���㲼��
		std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			out vec3 v_Position;
			out vec4 v_Color;

			void main(){
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = vec4( a_Position,1.0);
			}
		)";
		//������ɫ
		std::string fragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main(){
				color = vec4(v_Position*0.5+0.5,1.0);
				color = v_Color;
			}
		)";
		m_Shader.reset(new Shader(vertexSrc, fragmentSrc));
		//shader



		m_SquareVA.reset(VertexArray::Create());

		float squareVertices[4 * 3] = {
			-0.75f, -0.75f, 0.0f,
			 0.75f, -0.75f, 0.0f,
			 0.75f,  0.75f, 0.0f,
			-0.75f,  0.75f, 0.0f
		};

		std::shared_ptr<VertexBuffer> squareVB;
		squareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		
		squareVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" }
		});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<IndexBuffer> squareIB;
		squareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);


		std::string blueShaderVertexSrc2 = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;

			out vec3 v_Position;

			void main(){
				v_Position = a_Position;
				gl_Position = vec4( a_Position,1.0);
			}
		)";
		//������ɫ
		std::string blueShaderFragmentSrc2 = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			void main(){
				color = vec4(0.2, 0.3, 0.8, 1.0);
			}
		)";
		m_BlueShader.reset(new Shader(blueShaderVertexSrc2, blueShaderFragmentSrc2));

	}

	Application::~Application() {
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
		/*
		{
			WindowResizeEvent e(1280, 720);

			if(e.IsInCategory(EventCategoryApplication)){
				VOL_TRACE(e);
			}

			if (e.IsInCategory(EventCategoryInput)) {
				VOL_TRACE(e);
			}
		}
		*/
		while (m_Running) {
			glClearColor(0.1f, 0.1f, 0.1f, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			m_BlueShader->Bind();
			m_SquareVA->Bind();
			glDrawElements(GL_TRIANGLES, m_SquareVA->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

			m_Shader->Bind();
			m_VertexArray->Bind();
			//��λ��������� ���ٸ������� �������ͣ� ƫ����
			glDrawElements(GL_TRIANGLES, m_VertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			//��ImGui��ˢ�·ŵ�APP�У���Update�ֿ�
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