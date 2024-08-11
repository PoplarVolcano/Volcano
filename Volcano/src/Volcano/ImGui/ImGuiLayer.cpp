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
		//创建ImGui上下文
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();(void)io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;    // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
		//io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;	 // 当设置此标志时，ImGui会使用后端的鼠标光标。例如，当鼠标浮动在可点击的项目上时，该光标可能会更改为手形。
		//io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;	 // 位运算的优势可以设置多个标志，当需要更改鼠标光标形状时，应该使用应用程序的原生光标。如果不使用此标志，ImGui 将会使用它自带的基础光标

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;        // 启用自动布局
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;      // 启用多视窗/平台视窗（能出opengl的框子）

		//设置颜色
		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		//设置平台/渲染器绑定
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
		// 创建imgui窗口帧
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
		//如果启动启用多视窗/平台视窗
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}
}