#include "volpch.h"
#include "Application.h"
#include "Volcano/Core/Application.h"

#include "Volcano/Renderer/Renderer.h"
#include <imgui.h>
#include <glad/glad.h>

namespace Volcano {
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationProps& props)
	{
		VOL_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create(WindowProps(props.Name, props.WindowWidth, props.WindowHeight)));
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		m_Window->SetVSync(false);

		m_ImGuiLayer = new ImGuiLayer("ImGui");
		PushOverlay(m_ImGuiLayer);

		Renderer::Init();
		//Renderer::WaitAndRender();

		/*
		float vertices[4 * 5] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};
		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };
		m_VA = VertexArray::Create();
		Ref<VertexBuffer> VB = VertexBuffer::Create(vertices, sizeof(vertices));
		VB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
			});
		m_VA->AddVertexBuffer(VB);

		Ref<IndexBuffer> IB = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
		m_VA->SetIndexBuffer(IB);

		Renderer::GetShaderLibrary()->Load("assets/shaders/Square.glsl");
		Ref<Shader> shader = Renderer::GetShaderLibrary()->Get("Square");
		shader->Bind();
		shader->SetInt("u_Texture", 0);

		m_Texture = Texture2D::Create("assets/textures/Mostima.png");

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		// 纹理附件
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachment);
		glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// 附加到帧缓冲
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);

		// 深度附件
		//glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);
		//glBindTexture(GL_TEXTURE_2D, m_DepthAttachment);
		// 同时附加一个深度缓冲和一个模板缓冲为一个单独的纹理。纹理的每32位数值就包含了24位的深度信息和8位的模板信息。
		//glTexStorage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

		// 附加到帧缓冲
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);

		VOL_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		*/

	}
	Application::~Application()
	{
	}

	void Application::RenderImGui()
	{
		//将ImGui的刷新放到APP中，与Update分开
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
				//Renderer::Submit([app]() { app->RenderImGui(); });
				app->RenderImGui();

				Renderer::WaitAndRender();
			}

			/*
			glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

			Renderer::SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			Renderer::Clear();

			Ref<Shader> shader = Renderer::GetShaderLibrary()->Get("Square");
			m_Texture->Bind();
			shader->Bind();
			m_VA->Bind();
			glDrawElements(GL_TRIANGLES, m_VA->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			m_ImGuiLayer->Begin();
			ImGui::Begin("Settings");
			ImGui::Image((void*)m_ColorAttachment, ImVec2{ 320.0f, 180.0f });
			ImGui::End();
			m_ImGuiLayer->End();
			*/

			m_Window->OnUpdate();

			float time = GetTime();
			m_Timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;
		}
		OnShutdown();
	}

	void Application::Close() 
	{
		m_Running = false;
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