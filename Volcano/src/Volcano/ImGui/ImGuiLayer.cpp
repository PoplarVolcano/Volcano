#include "volpch.h"
#include "ImGuiLayer.h"

#include "imgui.h"
//#include "ImGuizmo.h"

#define IMGUI_IMPL_API
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Volcano/Core/Application.h"
#include "GLFW/glfw3.h"


namespace Volcano {
	ImGuiLayer::ImGuiLayer()
	{
	}

	ImGuiLayer::ImGuiLayer(const std::string& name)
	{
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::OnAttach() {
		//����ImGui������
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();(void)io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;    // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
		//io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;	 // �����ô˱�־ʱ��ImGui��ʹ�ú�˵�����ꡣ���磬����긡���ڿɵ������Ŀ��ʱ���ù����ܻ����Ϊ���Ρ�
		//io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;	 // λ��������ƿ������ö����־������Ҫ�����������״ʱ��Ӧ��ʹ��Ӧ�ó����ԭ����ꡣ�����ʹ�ô˱�־��ImGui ����ʹ�����Դ��Ļ������

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;        // �����Զ�����
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;      // ���ö��Ӵ�/ƽ̨�Ӵ����ܳ�opengl�Ŀ��ӣ�

		//������ɫ
		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		//����ƽ̨/��Ⱦ����
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 130");

	}

	void ImGuiLayer::OnDetach() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnImGuiRender()
	{
	}

	void ImGuiLayer::Begin()
	{
		// ����imgui����֡
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());
		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		//����������ö��Ӵ�/ƽ̨�Ӵ�
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}
}