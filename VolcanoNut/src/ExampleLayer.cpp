#include "ExampleLayer.h"

#include "Volcano/Renderer/Renderer2D.h"
#include "Volcano/Renderer/RendererItem/FullQuad.h"
#include "Volcano/Renderer/SceneRenderer.h"
#include "Volcano/Renderer/RendererItem/Skybox.h"

#include <chrono>
#include <imgui/imgui.h>
#include <Volcano/ImGui/ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

#include "Volcano/Scene/SceneSerializer.h"
#include "Volcano/Utils/PlatformUtils.h"
#include "Volcano/Math/Math.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Core/MouseBuffer.h"
#include "Volcano/Core/Window.h"


namespace Volcano{


    template<typename Fn>
    class Timer {
    public:
        Timer(const char* name, Fn&& func)
            :m_Name(name), m_Func(func), m_Stopped(false)
        {
            m_StartTimepoint = std::chrono::high_resolution_clock::now();
        }
        ~Timer() {
            if (!m_Stopped) {
                Stop();
            }
        }
        void Stop() {
            //销毁对象时，如果计时器尚未停止 ，它会记录结束时间点
            auto endTimepoint = std::chrono::high_resolution_clock::now();
            long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
            long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
            m_Stopped = true;
            float duration = (end - start) * 0.001f;// 单位ms
            m_Func({ m_Name, duration });
            //std::cout << "Timer:"<< m_Name << "时差：" << duration << "ms" << std::endl;
        }
    private:
        const char* m_Name;
        std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
        bool m_Stopped;
        Fn m_Func;
    };

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) { m_ProfileResults.push_back(profileResult); })

    ExampleLayer::ExampleLayer() {}

    ExampleLayer::~ExampleLayer() {}

    void ExampleLayer::OnAttach()
    {
        m_IconPlay     = Texture2D::Create("Resources/Icons/PlayButton.png");
        m_IconPause    = Texture2D::Create("Resources/Icons/PauseButton.png");
        m_IconStop     = Texture2D::Create("Resources/Icons/StopButton.png");
        m_IconSimulate = Texture2D::Create("Resources/Icons/SimulateButton.png");
        m_IconStep     = Texture2D::Create("Resources/Icons/StepButton.png");

        FullQuad::Init();

        // =========================================Framebuffer===============================================
        SceneRenderer::Init();
        m_Framebuffer = SceneRenderer::GetDeferredShadingFramebuffer();
        //================================================================================================

        // 初始化场景
        m_EditorScene = CreateRef<Scene>();
        m_ActiveScene = m_EditorScene;

        // 从app获取指令行，如果指令行大于1则读取场景
        auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
        if (commandLineArgs.Count > 1)
        {
            auto projectFilePath = commandLineArgs[1];
            OpenProject(projectFilePath);
        }
        else {
            // 引导用户选择一个项目路径prompt the user to select a directory
            // 如果没有打开项目则关闭VolcanoNut If no project is opened, close VolcanoNut
            if (!OpenProject())
                Application::Get().Close();
        }

        m_EditorCamera = EditorCamera(30.0f, 1.788f, 0.1f, 1000.0f);

        Renderer2D::SetLineWidth(4.0f);
    }

    void ExampleLayer::OnDetach() {}

    void ExampleLayer::OnUpdate(Timestep ts)
    {
        PROFILE_SCOPE("ExampleLayer::OnUpdate");

        m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        if (m_SceneState == SceneState::Play)
            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

        //Resize
        FramebufferSpecification spec = m_Framebuffer->GetSpecification();
        if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized Framebuffer is invalid
            (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            // 将视图的尺寸同步到帧缓冲尺寸
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        }

        // 重置渲染计数
        Renderer2D::ResetStats();

        // 场景更新
        switch (m_SceneState)
        {
            case SceneState::Edit:
            {
                m_EditorCamera.OnUpdate(ts);
                m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
                break;
            }
            case SceneState::Simulate:
            {
                m_EditorCamera.OnUpdate(ts);
                m_ActiveScene->OnUpdateSimulation(ts, m_EditorCamera);
                break;
            }
            case SceneState::Play:
            {
                m_ActiveScene->OnUpdateRuntime(ts);
                break;
            }
        }

        Renderer::SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        //============================================SceneRender=============================================
        SceneRenderer::BeginScene(m_ActiveScene, ts, m_SceneState, m_EditorCamera);
        SceneRenderer::GBuffer();
        SceneRenderer::SSAO();
        if(*SceneRenderer::GetPBR())
            SceneRenderer::PBRDeferredShading();
        else
            SceneRenderer::DeferredShading();


        auto [mx, my] = ImGui::GetMousePos();
        // 鼠标绝对位置减去viewport窗口的左上角绝对位置=鼠标相对于viewport窗口左上角的位置
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;
        // viewport窗口的右下角绝对位置-左上角的绝对位置=viewport窗口的大小
        glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
        // 翻转y,使其左下角开始才是(0,0)
        my = viewportSize.y - my;
        int mouseX = (int)mx;
        int mouseY = (int)my;

        // 设置点击鼠标时选中实体
        if (mouseX >= 0 && mouseY >= 0 && mouseX < viewportSize.x && mouseY < viewportSize.y)
        {
            // 读取帧缓冲第二个缓冲区的数据
            //int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
            int pixelData = SceneRenderer::GetDeferredShadingFramebuffer()->ReadPixel(1, mouseX, mouseY);
            // TODO：若shader中没有输出EntityID，则这里会设置一个有错误ID的Entity并且无法读取会报错
            //VOL_TRACE(pixelData);
            auto entityTemp = m_ActiveScene->GetEntityEnttMap()[(entt::entity)pixelData];
            if (entityTemp != nullptr)
                m_HoveredEntity = entityTemp;
        }

        SceneRenderer::HDR();
        SceneRenderer::EndScene();


        //=============================================================================================================


        Renderer::Clear();
        // 将FrameBuffer的图像放到window上
        // 还原视图尺寸
        Application::Get().GetWindow().ResetViewport();
        // TODO: 帧缓冲画面会被拉伸至视图尺寸，参考ShaderToy代码将画面保持正常尺寸
        Renderer::GetShaderLibrary()->Get("window")->Bind();
        //uint32_t windowTextureID = m_Framebuffer->GetColorAttachmentRendererID(2);
        //uint32_t windowTextureID = m_SpotDepthMapFramebuffer->GetDepthAttachmentRendererID();
        uint32_t windowTextureID = SceneRenderer::GetPointDepthMapFramebuffer()->GetDepthAttachmentRendererID();
        //uint32_t windowTextureID = m_BRDFLUT->GetRendererID();
        //Texture::Bind(windowTextureID, 0);
        Renderer::SetDepthTest(false);
        FullQuad::DrawIndexed();
        //Renderer::GetShaderLibrary()->Get("Skybox")->Bind();
        //Skybox::DrawIndexed();
        Renderer::SetDepthTest(true);

    }

    void ExampleLayer::OnImGuiRender()
    {
        // =============================DockSpace from ImGui::ShowDemoWindow()======================
        static bool p_open = true;

        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        //当使用ImGuiDockNodeFlags_PassthruCentralNode时，DockSpace（）将渲染我们的背景并处理通孔，因此我们要求Begin（）不渲染背景。
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        //重要提示：请注意，即使Begin（）返回false（即窗口折叠），我们也会继续。 
        //这是因为我们想保持DockSpace（）处于活动状态。如果DockSpace（）处于非活动状态， 
        //所有停靠在其中的活动窗口都将失去其父窗口并取消停靠。 
        //我们无法保留活动窗口和非活动停靠之间的停靠关系，否则 
        //停靠空间/设置的任何更改都会导致窗口陷入困境，永远不可见。
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", &p_open, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 370.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        style.WindowMinSize.x = minWinSizeX;

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                // Disabling fullscreen would allow the window to be moved to the front of other windows,
                // which we can't undo at the moment without finer window depth/z control.
                //禁用全屏将允许窗口移动到其他窗口的前面，如果没有更精细的窗口深度/z控制，我们目前无法撤消。
                ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
                ImGui::MenuItem("Padding", NULL, &opt_padding);
                ImGui::Separator();

                if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
					OpenProject();

				ImGui::Separator();

				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
					NewScene();

				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
					SaveScene();

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
					SaveSceneAs();

				ImGui::Separator();

                if (ImGui::MenuItem("Exit")) 
                    Application::Get().Close();

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Script"))
            {
                if (ImGui::MenuItem("Reload assembly", "Ctrl+R"))
                    ScriptEngine::ReloadAssembly();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        // =====================================================Viewport=====================================================
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

        ImGui::Begin("ViewportTemp");
        {
            uint32_t textureID = 0;
            const char* frameBuffers[] = 
            { "None", "DirectionalShadow", "PointShadow", "SpotShadow", "GBuffer1", "GBuffer2", "GBuffer3", 
                "GBuffer4", "SSAO", "SSAOBlur", "DefferedShading", "HDR", "LightShading1", "LightShading2" , 
                "LightShading3", "PBRLightShading"};
            ImGui::Combo("FrameBuffer", &m_ViewportTempIndex, frameBuffers, IM_ARRAYSIZE(frameBuffers));
            switch (m_ViewportTempIndex)
            {
            case 0:
                textureID = 0;
                break;
            case 1:
                textureID = SceneRenderer::GetDirectionalDepthMapFramebuffer()->GetDepthAttachmentRendererID();
                break;
            case 2:
                textureID = SceneRenderer::GetPointDepthMapFramebuffer()->GetDepthAttachmentRendererID();
                break;
            case 3:
                textureID = SceneRenderer::GetSpotDepthMapFramebuffer()->GetDepthAttachmentRendererID();
                break;
            case 4:
                textureID = SceneRenderer::GetGBufferFramebuffer()->GetColorAttachmentRendererID(0);
                break;
            case 5:
                textureID = SceneRenderer::GetGBufferFramebuffer()->GetColorAttachmentRendererID(1);
                break;
            case 6:
                textureID = SceneRenderer::GetGBufferFramebuffer()->GetColorAttachmentRendererID(2);
                break;
            case 7:
                textureID = SceneRenderer::GetGBufferFramebuffer()->GetColorAttachmentRendererID(3);
                break;
            case 8:
                textureID = SceneRenderer::GetSSAOFramebuffer()->GetColorAttachmentRendererID(0);
                break;
            case 9:
                textureID = SceneRenderer::GetSSAOBlurFramebuffer()->GetColorAttachmentRendererID(0);
                break;
            case 10:
                textureID = SceneRenderer::GetDeferredShadingFramebuffer()->GetColorAttachmentRendererID();
                break;
            case 11:
                textureID = SceneRenderer::GetHDRFramebuffer()->GetColorAttachmentRendererID();
                break;
            case 12:
                textureID = SceneRenderer::GetLightShadingFramebuffer(0)->GetColorAttachmentRendererID(0);
                break;
            case 13:
                textureID = SceneRenderer::GetLightShadingFramebuffer(0)->GetColorAttachmentRendererID(1);
                break;
            case 14:
                textureID = SceneRenderer::GetLightShadingFramebuffer(0)->GetColorAttachmentRendererID(2);
                break;
            case 15:
                textureID = SceneRenderer::GetPBRLightShadingFramebuffer(0)->GetColorAttachmentRendererID(0);
                break;
            default:
                textureID = 0;
                break;
            }
            
            ImGui::Image((void*)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
            
            ImGui::End();
        }

        ImGui::Begin("Viewport");

        // 获取Viewport视口左上角与viewport视口标题栏距离的偏移位置（0, 24) - 必须放这，因为标题栏后就是视口的左上角
        auto viewportOffset = ImGui::GetCursorPos();  // Include Tab bar

        // 窗口是否被选中
        m_ViewportFocused = ImGui::IsWindowFocused();
        // 鼠标是否悬浮在窗口上
        m_ViewportHovered = ImGui::IsWindowHovered();
        // 只有同时选中和悬浮才会执行Event
        Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused);

        // 获取视图窗口尺寸
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        // 在视图显示帧缓冲画面
        //uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image((void*)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{0, 1}, ImVec2{1, 0});
        
        
        // 接收在此视口拖放过来的值，On target candidates，拖放目标
        if (ImGui::BeginDragDropTarget()) 
        {
            // 因为接收内容可能为空，需要if判断。 CONTENT_BROWSER_ITEM：拖动携带的内容
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) 
            {
                const wchar_t* path = (const wchar_t*)payload->Data;
                OpenScene(path);
            }
            ImGui::EndDragDropTarget();
        }


        // 获取vieport视口大小 - 包含标题栏的高
        auto windowSize = ImGui::GetWindowSize();
        // 获取当前vieport视口标题栏左上角距离当前整个屏幕左上角（0,0）的位置
        ImVec2 minBound = ImGui::GetWindowPos();
        minBound.x += viewportOffset.x;
        minBound.y += viewportOffset.y;

        ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };
        // 保存左上角和右下角距离整个屏幕左上角的位置
        m_ViewportBounds[0] = { minBound.x, minBound.y };
        m_ViewportBounds[1] = { maxBound.x, maxBound.y };

        //Gizmos
        Ref<Entity> selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity && m_GizmoType != -1 && m_SceneState == SceneState::Edit)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            float windowWidth = (float)ImGui::GetWindowWidth();
            float windowHeight = (float)ImGui::GetWindowHeight();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
            
            //Editor Camera
            const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
            glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

            // Selected Entity transform
            auto& tc = selectedEntity->GetComponent<TransformComponent>();
            glm::mat4 transform = tc.GetTransform();

            // Snapping
            bool snap = Input::IsKeyPressed(Key::LeftControl);
            // Snap to 0.5m for translation/scale
            float snapValue = 0.5f;
            // Snap to 45 defrees for rotation
            if (m_GizmoType == ImGuizmo::OPERATION::ROTATE) 
            {
                snapValue = 45.0f;
            }
            float snapValues[3] = { snapValue, snapValue, snapValue };

            ImGuizmo::Manipulate(
                glm::value_ptr(glm::inverse(cameraView)), 
                glm::value_ptr(cameraProjection),
                (ImGuizmo::OPERATION)m_GizmoType, 
                ImGuizmo::LOCAL, 
                glm::value_ptr(transform),
                nullptr,
                snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                // 预防万向节死锁
                glm::vec3 translation, rotation, scale;
                Math::DecomposeTransform(transform, translation, rotation, scale);

                glm::vec3 deltaRotation = rotation - tc.Rotation;
                tc.Translation = translation;
                tc.Rotation += deltaRotation;
                tc.Scale = scale;
            }
        }

        // Viewport
        ImGui::End();
        ImGui::PopStyleVar();

        m_SceneHierarchyPanel.OnImGuiRender();
        m_ContentBrowserPanel->OnImGuiRender();

        // =====================================================Settings=====================================================
        ImGui::Begin("Stats");

        ImGui::Checkbox("PBR", SceneRenderer::GetPBR());
        // Stats
        ImGui::DragFloat("Exposure", SceneRenderer::GetExposure(), 0.01f);
        ImGui::Checkbox("Bloom", SceneRenderer::GetBloom());

        ImGui::Checkbox("SSAO", SceneRenderer::GetSSAOSwitch());
        ImGui::DragInt("KernelSize", SceneRenderer::GetKernelSize());
        ImGui::DragFloat("Radius", SceneRenderer::GetRadius(), 0.01f);
        ImGui::DragFloat("Bias", SceneRenderer::GetBias(), 0.01f);
        ImGui::DragFloat("Power", SceneRenderer::GetPower(), 0.01f);


        std::string name = "None";
        if (m_HoveredEntity)
        {
            // BUG!!! 悬浮在标题框m_HoveredEntity为空且会调用GetComponent
            //name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
        }
        ImGui::Text("Hovered Entity: %s", name.c_str());

        auto stats = Renderer2D::GetStats();
        ImGui::Text("Renderer2D Stats:");
        ImGui::Text("Draw Calls: %d", stats.DrawCalls);
        ImGui::Text("Quads: %d", stats.QuadCount);
        ImGui::Text("Vertices: %d", stats.GetTotalVartexCount());
        ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

        for (auto& result : m_ProfileResults)
        {
            char label[50];
            strcpy(label, "%.3fms  ");
            strcat(label, result.Name);
            ImGui::Text(label, result.Time);
        }
        m_ProfileResults.clear();



            
            
            
            


        ImGui::End();

        ImGui::Begin("Settings");
        ImGui::Checkbox("Show physics colliders", &m_ShowPhysicsColliders);
        ImGui::End();

        //ImGui::ShowDemoWindow();

        UI_Toolbar();

        // Dockspace
        ImGui::End();

    }

    void ExampleLayer::UI_Toolbar()
    { 
        // padding
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
        // 按钮图片透明背景
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        auto& colors = ImGui::GetStyle().Colors;
        // 按钮针对hover、click有不同效果
        const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
        const auto& buttonActive = colors[ImGuiCol_ButtonActive];
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

        ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        
        bool toolbarEnabled = (bool)m_ActiveScene;

        ImVec4 tintColor = ImVec4(1, 1, 1, 1);
        if (!toolbarEnabled)
            tintColor.w = 0.5f;

        // 按钮适应窗口大小、放大时应使用线性插值保证不那么模糊
        float size = ImGui::GetWindowHeight() - 4.0f;
        // 按钮应该在中间, 设置按钮的x位置
        ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - size);

        bool hasPlayButton     = m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play;
        bool hasSimulateButton = m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate;
        bool hasPauseButton    = m_SceneState != SceneState::Edit;
        
        // 开始暂停按钮
        if(hasPlayButton)
        {
            Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) ? m_IconPlay : m_IconStop;
            if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
            {
                if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate)
                    OnScenePlay();
                else if (m_SceneState == SceneState::Play)
                    OnSceneStop();
            }
        }

        if (hasSimulateButton)
        {
            if(hasPlayButton)
            // 模拟按钮
                ImGui::SameLine();
            Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play) ? m_IconSimulate : m_IconStop;

            if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
            {

                if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play)
                    OnSceneSimulate();
                else if (m_SceneState == SceneState::Simulate)
                    OnSceneStop();
            }
        }

        if (hasPauseButton)
        {
            bool isPaused = m_ActiveScene->IsPaused();
            ImGui::SameLine();
            {
                Ref<Texture2D> icon = isPaused ? m_IconPlay : m_IconPause;
                if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
                {
                    m_ActiveScene->SetPaused(!isPaused);
                }
            }

            // Step button
            if (isPaused)
            {
                ImGui::SameLine();
                {
                    Ref<Texture2D> icon = m_IconStep;
                    if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
                    {
                        m_ActiveScene->Step();
                    }
                }
            }

        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);



        ImGui::End();
    }

    void ExampleLayer::OnEvent(Event& event)
    {
        if (m_SceneState == SceneState::Edit)
        {
            m_EditorCamera.OnEvent(event);
        }
        
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(VOL_BIND_EVENT_FN(ExampleLayer::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(VOL_BIND_EVENT_FN(ExampleLayer::OnMouseButtonPressed));
        //dispatcher.Dispatch<MouseMovedEvent>(VOL_BIND_EVENT_FN(ExampleLayer::OnMouseMoved));

    }

    bool ExampleLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.IsRepeat()) 
        {
            return false;
        }

        bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

        switch (e.GetKeyCode()) 
        {
        case Key::N:
            if (control)
                NewScene();
            break;
        case Key::O:
            if (control)
                OpenProject();
            break;
        case Key::S:
            if (control)
                if (shift)
                    SaveSceneAs();
                else
                    SaveScene();
            break;
        case Key::D:
            if (control)
                OnDuplicateEntity();

        //Gizmos
        case Key::Q:
            m_GizmoType = -1;
            break;
        case Key::W:
            m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
            break;
        case Key::E:
            m_GizmoType = ImGuizmo::OPERATION::ROTATE;
            break;
        case Key::R:
            if (control)
            {
                ScriptEngine::ReloadAssembly();
            }
            else
            {
                if (!ImGuizmo::IsUsing())
                    m_GizmoType = ImGuizmo::OPERATION::SCALE;
            }
            break;

            // TODO: 只有鼠标点击viewport的实体会被选中删除，SceneHierarchyPanel选中实体delete不能删除
        case Key::Delete:
            if (Application::Get().GetImGuiLayer()->GetActiveWidgetID() == 0)
            {
                Ref<Entity> selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
                if (selectedEntity)
                {
                    m_SceneHierarchyPanel.SetSelectedEntity({});
                    m_ActiveScene->DestroyEntity(selectedEntity);
                }
            }
            break;
        }

        return false;
    }

    bool ExampleLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        if (e.GetMouseButton() == Mouse::ButtonLeft)
        {
            // 鼠标悬浮，没按Alt
            if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
                m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
        }
        return false;
    }

    /*
    bool ExampleLayer::OnMouseMoved(MouseMovedEvent& e)
    {
        MouseBuffer& mouse = MouseBuffer::instance();

        if (mouse.GetOnActive()) {
            if (mouse.GetFirstMouse())
            {
                mouse.SetLastX(e.GetX());
                mouse.SetLastY(e.GetY());
                mouse.SetFirstMouse(false);
            }

            float xoffset = e.GetX() - mouse.GetLastX();
            float yoffset = mouse.GetLastY() - e.GetY();
            mouse.SetLastX(e.GetX());
            mouse.SetLastY(e.GetY());

            float sensitivity = 0.05;
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            mouse.SetYaw(mouse.GetYaw() + xoffset);
            mouse.SetPitch(mouse.GetPitch() + yoffset);
        }
    
    }*/

    void ExampleLayer::OnOverlayRender()
    {
        // 播放状态下
        if (m_SceneState == SceneState::Play)
        {
            Ref<Entity> camera = m_ActiveScene->GetPrimaryCameraEntity();
            if (!camera)
                return;  // 找不到就退出
            Renderer2D::BeginScene(camera->GetComponent<CameraComponent>().Camera, camera->GetComponent<TransformComponent>().GetTransform());
        }
        else
        {
            Renderer2D::BeginScene(m_EditorCamera, m_EditorCamera.GetViewMatrix());
        }

        if (m_ShowPhysicsColliders)
        {
            // 包围盒需跟随对应的物体,包围盒的transform需基于物体的平移、旋转、缩放。
            // Box Colliders
            {
                auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
                for (auto entity : view)
                {
                    auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);

                    // 0.001f Z轴偏移量
                    glm::vec3 translation = tc.Translation + glm::vec3(bc2d.Offset, 0.001f);
                    // bc2d.Size需乘以2，以免缩小一半
                    glm::vec3 scale = tc.Scale * glm::vec3(bc2d.Size * 2.0f, 1.0f);

                    glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
                        * glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
                        * glm::scale(glm::mat4(1.0f), scale);

                    // 绿色的包围盒
                    Renderer2D::DrawRect(transform, glm::vec4(0, 1, 0, 1));
                }
            }

            // Circle Colliders
            {
                auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
                for (auto entity : view)
                {
                    auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(entity);

                    glm::vec3 translation = tc.Translation + glm::vec3(cc2d.Offset, 0.001f);
                    // cc2d.Radius需乘以2，以免缩小一半
                    glm::vec3 scale = tc.Scale * glm::vec3(cc2d.Radius * 2.0f);

                    glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
                        * glm::scale(glm::mat4(1.0f), scale);

                    Renderer2D::DrawCircle(transform, glm::vec4(0, 1, 0, 1), 0.01f);
                }
            }
        }

        // Draw selected entity outline 
        if (Ref<Entity> selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity())
        {
            const TransformComponent& transform = selectedEntity->GetComponent<TransformComponent>();
            Renderer2D::DrawRect(transform.GetTransform(), glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)); 
        }

        Renderer2D::EndScene();
    }

    void ExampleLayer::NewProject()
    {
        Project::New();
    }

    void ExampleLayer::OpenProject(const std::filesystem::path& path)
    {
        if (Project::Load(path))
        {
            ScriptEngine::Init();
            auto startScenePath = Project::GetAssetFileSystemPath(Project::GetActive()->GetConfig().StartScene);
            OpenScene(startScenePath);
            m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();
        }
    }

    bool ExampleLayer::OpenProject()
    {
        // 打开.hproj文件读取Project
        std::string filepath = FileDialogs::OpenFile("Volcano Project (*.hproj)\0*.hproj\0");
        if (filepath.empty())
            return false;

        OpenProject(filepath);
        return true;
    }

    void ExampleLayer::SaveProject()
    {
        // Project::SaveActive();
    }

    void ExampleLayer::NewScene()
    {
        m_EditorScene = CreateRef<Scene>();
        m_SceneHierarchyPanel.SetContext(m_EditorScene);
        m_EditorScenePath = std::filesystem::path();
        m_ActiveScene = m_EditorScene;
    }

    void ExampleLayer::OpenScene()
    {
        std::string filepath = FileDialogs::OpenFile("Volcano Scene (*.volcano)\0*.volcano\0");
        if (!filepath.empty())
            OpenScene(filepath);
    }
    
    void ExampleLayer::OpenScene(const std::filesystem::path& path)
    {
        if (m_SceneState != SceneState::Edit)
            OnSceneStop();

        if(path.extension().string() != ".volcano")
        {
            VOL_WARN("Could not load {0} - not a scene file", path.filename().string());
            return;
        }

        Ref<Scene> newScene = CreateRef<Scene>();
        SceneSerializer serializer(newScene);
        if (serializer.Deserialize(path.string()))
        {
            // 编辑器场景获取文件读取场景
            m_EditorScene = newScene;
            m_SceneHierarchyPanel.SetContext(m_EditorScene);
            m_EditorScenePath = path;
            m_ActiveScene = m_EditorScene;
        }
    }

    void ExampleLayer::SaveScene()
    {
        if (!m_EditorScenePath.empty())
        {
            SerializeScene(m_ActiveScene, m_EditorScenePath);
            m_EditorScene = m_ActiveScene;
            if (m_SceneState != SceneState::Edit)
                OnSceneStop();
        }
        else
            SaveSceneAs();
    }

    void ExampleLayer::SaveSceneAs()
    {
        std::string filepath = FileDialogs::SaveFile("Volcano Scene (*.volcano)\0*.volcano\0");
        if (!filepath.empty())
        {
            SerializeScene(m_ActiveScene, filepath);
            m_EditorScenePath = filepath;
            m_EditorScene = m_ActiveScene;
            if (m_SceneState != SceneState::Edit)
                OnSceneStop();
        }
    }

    // 序列化场景
    void ExampleLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& path)
    {
        SceneSerializer serializer(scene);
        serializer.Serialize(path.string());
    }

    // 播放场景
    void ExampleLayer::OnScenePlay()
    {
        // 运行时物理模拟则重置场景
        if (m_SceneState == SceneState::Simulate)
            OnSceneStop();

        // 设置场景状态：播放
        m_SceneState = SceneState::Play;

        // 分割活动场景为单独的Scene
        m_ActiveScene = Scene::Copy(m_EditorScene);

        if (m_SceneHierarchyPanel.GetSelectedEntity())
        {
            UUID selectedEntityID = m_SceneHierarchyPanel.GetSelectedEntity()->GetUUID();
            m_SceneHierarchyPanel.SetSelectedEntity(m_ActiveScene->GetEntityByUUID(selectedEntityID));
        }

        m_ActiveScene->SetRunning(true);

        // 设置活动场景物理效果
        m_ActiveScene->OnRuntimeStart();

        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
    }

    void ExampleLayer::OnSceneSimulate()
    {
        // 物理模拟时运行则重置场景
        if (m_SceneState == SceneState::Play)
            OnSceneStop();

        // 设置场景状态：模拟
        m_SceneState = SceneState::Simulate;

        m_ActiveScene = Scene::Copy(m_EditorScene);

        m_SceneHierarchyPanel.SetContext(m_ActiveScene);

        m_ActiveScene->SetRunning(true);

        m_ActiveScene->OnSimulationStart();

    }

    // 暂停场景
    void ExampleLayer::OnScenePause()
    {
        VOL_CORE_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate);

        if (m_SceneState == SceneState::Edit)
            return;
        m_ActiveScene->SetPaused(true);
        m_ActiveScene->SetRunning(false);
    }

    // 重置场景
    void ExampleLayer::OnSceneStop()
    {
        VOL_CORE_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate);

        // 关闭活动场景物理效果
        if (m_SceneState == SceneState::Play)
            m_ActiveScene->OnRuntimeStop();
        else if (m_SceneState == SceneState::Simulate)
            m_ActiveScene->OnSimulationStop();

        // 设置场景状态：编辑
        m_SceneState = SceneState::Edit;

        m_SceneHierarchyPanel.SetContext(m_EditorScene);

        // 活动场景恢复编辑器场景
        m_ActiveScene = m_EditorScene;

        // edit模式下running必然false，可删除
        m_ActiveScene->SetRunning(false);

        MouseBuffer::instance().SetOnActive(true);
    }

    // 编辑模式可用，如果选中实体，将实体复制
    void ExampleLayer::OnDuplicateEntity()
    {
        if (m_SceneState != SceneState::Edit)
            return;

        Ref<Entity> selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity)
        {
            Ref<Entity> newEntity = m_ActiveScene->DuplicateEntity(selectedEntity);
            m_SceneHierarchyPanel.SetSelectedEntity(newEntity);
        }
    }


}