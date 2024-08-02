#include "volpch.h"
#include "Application.h"

#include <glad/glad.h>

#include "Input.h"

namespace Volcano {

	// bind函数是C++11标准中的一个函数模板，用于将函数和一组参数绑定在一起，
	// 生成一个新的可调用对象。可以用于实现函数适配器、参数绑定等功能。
	// bind函数的返回值是一个可调用对象，可以通过调用该对象来执行绑定的函数。
	// std::placeholders 是 C++ 标准库中的一个命名空间，它包含了一组特殊的占位符对象，用于与 std::bind 函数一起使用。
	// 这些占位符对象允许你在绑定函数时表示某些参数是未指定的，并在稍后提供这些参数的值。
	// std::placeholders 中有以下占位符：
	// std::placeholders::_1：表示第一个未指定的参数。
	// std::placeholders::_2：表示第二个未指定的参数。
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
		// 回调函数
		// 将 OnEvent 赋给 m_Window.m_Data.EventCallback
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

		//着色器
		//顶点布局
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
		//绘制颜色
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
		//绘制颜色
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

	//事件处理
	void Application::OnEvent(Event& e) {
		//创建事件拦截器
		EventDispatcher dispatcher(e);

		//拦截关闭窗口事件，触发关闭窗口函数
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		//输出事件toString()
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
			//如何绘制索引， 多少个索引， 索引类型， 偏移量
			glDrawElements(GL_TRIANGLES, m_VertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			//将ImGui的刷新放到APP中，与Update分开
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