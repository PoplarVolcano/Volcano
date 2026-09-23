#include "VolcanoEditorLayer.h"
#include <glm/gtc/type_ptr.hpp>

#include "ImGuizmo.h"

#include "Volcano/Renderer/PostProcessing.h"

#include "Volcano/Core/AppPath.h"

namespace Volcano
{

    void VolcanoEditorLayer::OnImGuiRender()
    {
        if (Project::GetActive() != nullptr && Project::GetActive()->GetPlayGame())
            return;

        if (m_NewProject)
        {
            static bool use_work_area = true;
            static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
            ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

            ImGui::Begin("NewProject", &m_NewProject, flags);
            static std::string newProjectName = "Project";
            ImGui::Text((const char*)u8"项目名称");
            char buffer[MAX_PATH];
            memset(buffer, 0, sizeof(buffer));
            strncpy_s(buffer, sizeof(buffer), newProjectName.c_str(), sizeof(buffer));
            if (ImGui::InputText("##NewProjectName", buffer, sizeof(buffer)))
            {
                newProjectName = buffer;
            }
            static std::string newProjectParentAbsolutePath = AppPath::GetInstance().GetExePath().append("Projects").string();
            ImGui::Text((const char*)u8"位置(如果路径不存在，将新建路径。)");
            ImGui::InputText("##newProjectParentAbsolutePath", newProjectParentAbsolutePath.data(), MAX_PATH);
            ImGui::SameLine();
            if (ImGui::Button("..."))
                newProjectParentAbsolutePath = FileDialogs::OpenFolder(newProjectParentAbsolutePath);

            std::filesystem::path newProjectAbsolutePath = std::filesystem::path(newProjectParentAbsolutePath) / std::filesystem::path(newProjectName);
            ImGui::Text(((const char*)u8"项目将在" + newProjectAbsolutePath.string() + (const char*)u8"中创建").c_str());
            if (ImGui::Button("Create Project"))
            {
                if (!std::filesystem::exists(newProjectAbsolutePath))
                {
                    NewProject(newProjectAbsolutePath, newProjectName);
                    auto projectAbsolutePathTemp = newProjectAbsolutePath;
                    OpenProject(projectAbsolutePathTemp.append(newProjectName + ".proj"));
                    m_NewProject = false;
                    m_ProjectLoaded = true;
                }

            }
            if (std::filesystem::exists(newProjectAbsolutePath))
            {
                if (ImGui::BeginItemTooltip())
                {
                    ImGui::Text("Project already exist.");
                    ImGui::EndTooltip();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                if (m_ProjectLoaded)
                    m_NewProject = false;
            }
            if (!m_ProjectLoaded)
            {
                if (ImGui::BeginItemTooltip())
                {
                    ImGui::Text("No project loaded.");
                    ImGui::EndTooltip();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Open Project"))
            {
                if (OpenProject())
                {
                    m_NewProject = false;
                    m_ProjectLoaded = true;
                }
            }
            ImGui::End();
        }

        if (!m_ProjectLoaded)
            return;

        // =============================DockSpace from ImGui::ShowDemoWindow()======================
        static bool p_open = true;

        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // ImGuiWindowFlags_NoDocking标志使父窗口不可停靠
        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
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

        //当使用ImGuiDockNodeFlags_PassthruCentralNode时，DockSpace（）将渲染我们的背景并处理通孔，因此我们要求Begin（）不渲染背景。
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

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
            bool enable = m_EditorScene->GetName() != "Prefab";

            if (ImGui::BeginMenu("File"))
            {
                // Disabling fullscreen would allow the window to be moved to the front of other windows,
                // which we can't undo at the moment without finer window depth/z control.
                //禁用全屏将允许窗口移动到其他窗口的前面，如果没有更精细的窗口深度/z控制，我们目前无法撤消。
                ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
                ImGui::MenuItem("Padding", NULL, &opt_padding);
                ImGui::Separator();

                if (ImGui::MenuItem("New Project..."))
                    m_NewProject = true;
                if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
                    OpenProject();
                if (ImGui::MenuItem("Save Project...", "Ctrl+O"))
                    SaveProject();

                ImGui::Separator();

                if (ImGui::MenuItem("New Scene", "Ctrl+N", false, enable))
                    NewScene();

                if (ImGui::MenuItem("Save Scene", "Ctrl+S", false, enable))
                    SaveScene();

                if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S", false, enable))
                    SaveSceneAs();

                if (ImGui::MenuItem("Set Scene As StartScene", "Ctrl+Shift+S", false, enable))
                    Project::GetActive()->GetConfig().startScene = m_ActiveScene->GetFileRelativePath();

                if (ImGui::MenuItem("Exit"))
                    Application::GetInstance().Close();

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Script"))
            {
                if (ImGui::MenuItem("Set assembly", "Ctrl+R", false, enable))
                {
                    std::string assemblyPath = FileDialogs::OpenFile("C# Assembly (*.dll)\0*.dll\0 ", Project::GetActive()->GetProjectAbsolutePath().string());
                    Project::GetActive()->GetConfig().scriptModulePath = std::filesystem::relative(assemblyPath, Project::GetActive()->GetAssetsAbsolutePath()).string();
                    ScriptEngine::ReloadAssembly();
                }
                if (ImGui::MenuItem("Reload assembly", "Ctrl+R", false, enable))
                    ScriptEngine::ReloadAssembly();
            
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }


        if (m_ViewportTempEnabled)
        {
            ImGui::Begin("ViewportTemp");
            {
                uint32_t textureID = 0;
                static int viewportTempIndex;
                const char* frameBuffers[] = {
                    "None",
                    "DirectionalLightShadow",
                    "PointLightShadow",
                    "SpotLightShadow",
                    "GBufferPositionAndDepth",
                    "GBufferAlbedo",
                    "GBufferNormal",
                    "GBufferRoughnessAndAO",
                    "SSAO",
                    "SSAOBlur",
                    "DefferedShading",
                    "HDR",
                    "LightShadingAmbient",
                    "LightShadingDiffuse",
                    "LightShadingSpecular",
                    "PBRLightShading",
                    "BRDFLUT"
                };
                ImGui::Combo("FrameBuffer", &viewportTempIndex, frameBuffers, IM_ARRAYSIZE(frameBuffers));
                switch (viewportTempIndex)
                {
                case 1:
                    textureID = SceneRenderer::GetDirectionalLightDepthMapFramebuffer()->GetDepthAttachmentRendererID();
                    break;
                case 2:
                    textureID = SceneRenderer::GetPointLightDepthMapFramebuffer()->GetDepthAttachmentRendererID();
                    break;
                case 3:
                    textureID = SceneRenderer::GetSpotLightDepthMapFramebuffer()->GetDepthAttachmentRendererID();
                    break;
                case 4:
                    textureID = SceneRenderer::GetResolvedGBufferFramebuffer()->GetColorAttachmentRendererID(0);
                    break;
                case 5:
                    textureID = SceneRenderer::GetResolvedGBufferFramebuffer()->GetColorAttachmentRendererID(1);
                    break;
                case 6:
                    textureID = SceneRenderer::GetResolvedGBufferFramebuffer()->GetColorAttachmentRendererID(2);
                    break;
                case 7:
                    textureID = SceneRenderer::GetResolvedGBufferFramebuffer()->GetColorAttachmentRendererID(3);
                    break;
                case 8:
                    textureID = SceneRenderer::GetSSAOFramebuffer()->GetColorAttachmentRendererID();
                    break;
                case 9:
                    textureID = SceneRenderer::GetSSAOBlurFramebuffer()->GetColorAttachmentRendererID();
                    break;
                case 10:
                    textureID = SceneRenderer::GetDeferredShadingFramebuffer()->GetColorAttachmentRendererID(0);
                    break;
                case 12:
                    textureID = SceneRenderer::GetLightShadingFramebuffer()->GetColorAttachmentRendererID(0);
                    break;
                case 13:
                    textureID = SceneRenderer::GetLightShadingFramebuffer()->GetColorAttachmentRendererID(1);
                    break;
                case 14:
                    textureID = SceneRenderer::GetLightShadingFramebuffer()->GetColorAttachmentRendererID(2);
                    break;
                case 15:
                    textureID = SceneRenderer::GetPBRLightShadingFramebuffer()->GetColorAttachmentRendererID(0);
                    break;
                case 16:
                    textureID = SceneRenderer::GetBRDFLUT()->GetRendererID();
                    break;
                default:
                    textureID = 0;
                    break;
                }

                if (textureID != 0)
                {
                    ImGui::Image((ImTextureID)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
                }

                ImGui::End();
            }
        }

        //Volcano::ImGuiContextManager::ShowDemoWindow();

        ImGuiWindowFlags flag = ImGuiWindowFlags_AlwaysAutoResize;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Viewport");

        // 获取Viewport视口左上角与viewport视口标题栏距离的偏移位置（0, 24) - 必须放这，因为标题栏后就是视口的左上角
        auto viewportOffset = ImGui::GetCursorPos();  // Include Tab bar

        // 窗口是否被选中
        m_ViewportFocused = ImGui::IsWindowFocused();
        // 鼠标是否悬浮在窗口上
        m_ViewportHovered = ImGui::IsWindowHovered();
        // 只有同时选中和悬浮才会执行Event
        // 注：Event包括编辑器摄像头的移动和各种快捷键操作
        Application::GetInstance().GetImGuiLayer()->BlockEvents(!(m_ViewportFocused && m_ViewportHovered));


        // 获取内容区域（排除标题栏和菜单栏）
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        uint32_t textureID = SceneRenderer::GetPostProcessingFramebuffer()->GetColorAttachmentRendererID();
        ImGui::Image((void*)(uint64_t)textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0, 1), ImVec2(1, 0));

        // 接收在此视口拖放过来的值，On target candidates，拖放目标
        if (ImGui::BeginDragDropTarget())
        {
            // 因为接收内容可能为空，需要if判断。 CONTENT_BROWSER_ITEM：拖动携带的内容
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* path = (const wchar_t*)payload->Data;
                std::filesystem::path scenePath = path;

                if (scenePath.extension() == ".scene")
                    OpenScene(path);
            }
            ImGui::EndDragDropTarget();
        }

        //Gizmos

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

        Ref<Entity> selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity && m_GizmoType != -1 && m_SceneState == SceneState::Edit)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            float windowWidth = (float)ImGui::GetWindowWidth();
            float windowHeight = (float)ImGui::GetWindowHeight();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

            const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
            glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

            glm::mat4 worldTransform = selectedEntity->GetWorldTransform();

            bool snap = Input::IsKeyPressed(Key::LeftControl);
            float snapValue = 0.5f;
            if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
            {
                snapValue = 45.0f;
            }
            float snapValues[3] = { snapValue, snapValue, snapValue };

            /*
            在 3D 视口中渲染一个可交互的操纵杆（Gizmo），让用户通过鼠标拖拽来移动、旋转或缩放场景中的对象。
            ImGuizmo::Manipulate只负责两件事：
            渲染：在屏幕上绘制出 Gizmo 的线条（箭头、圆环等）。
            计算：检测鼠标是否在 Gizmo 上拖拽，并计算出新的变换值。
            相机视图矩阵：注意传的是摄像机视图矩阵的逆矩阵！Gizmo需要世界空间下的数据
            相机投影矩阵：Gizmo 需要知道是透视投影还是正交投影，来正确计算鼠标与 3D 物体的交互点。
            Gizmo 操作类型：决定 Gizmo 是平移（TRANSLATE）、旋转（ROTATE）还是缩放（SCALE）模式。
            Gizmo 空间：Gizmo 的朝向是基于物体的局部坐标轴（LOCAL）还是世界坐标轴（WORLD）。
            变换矩阵：传入和传出数据的管道。传入时，它是物体的当前世界变换矩阵。函数执行后，如果用户拖拽了 Gizmo，这个矩阵会被直接修改（更新）。
            增量矩阵：可选的 delta 矩阵，用于更精细的变换控制，通常不填。
            捕捉值：用于实现“步进”效果，例如让物体每次移动 0.5 单位，或每次旋转 15 度。传入 nullptr 则关闭捕捉。
            */

            ImGuizmo::Manipulate(
                glm::value_ptr(glm::inverse(cameraView)),
                glm::value_ptr(cameraProjection),
                (ImGuizmo::OPERATION)m_GizmoType,
                ImGuizmo::LOCAL,
                glm::value_ptr(worldTransform),
                nullptr,
                snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                glm::vec3 euler;
                auto& tc = selectedEntity->GetComponent<TransformComponent>();
                if (Entity* parent = selectedEntity->GetEntityParent())
                {
                    glm::mat4 parentWorld = parent->GetWorldTransform();
                    glm::mat4 localMatrix = glm::inverse(parentWorld) * worldTransform;
                    Math::DecomposeTransform(localMatrix, tc.translation, euler, tc.scale);
                }
                else
                {
                    Math::DecomposeTransform(worldTransform, tc.translation, euler, tc.scale);
                }
                tc.rotation = glm::quat(euler);
                tc.inspectorEulerHint = glm::degrees(euler);

            }
        }

        ImGui::End(); // Viewport
        ImGui::PopStyleVar();


        ImGui::Begin("Stats");

        auto stats = Renderer2D::GetStats();
        ImGui::Text("Renderer2D Stats:");
        ImGui::Text("Draw Calls: %d", stats.DrawCalls);
        ImGui::Text("Quads: %d", stats.QuadCount);
        ImGui::Text("Vertices: %d", stats.GetTotalVartexCount());
        ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

        for (auto& result : m_ProfileResults)
        {
            char label[50];
            strcpy_s(label, "%.3fms  ");
            strcat_s(label, result.Name);
            ImGui::Text(label, result.Time);
        }
        m_ProfileResults.clear();

        ImGui::Separator();

        ImGui::Checkbox("ViewportTemp", &m_ViewportTempEnabled);

        ImGui::Separator();

        ImGui::Checkbox("Show physics colliders", &m_ShowPhysicsColliders);

        ImGui::Separator();

        bool HDREnabled = m_ActiveScene->GetHDREnabled();
        if (ImGui::Checkbox("HDR", &HDREnabled))
        {
            m_ActiveScene->SetHDREnabled(HDREnabled);
        }

        if (HDREnabled)
        {
            float exposure = m_ActiveScene->GetExposure();
            if (ImGui::DragFloat("Exposure", &exposure, 0.1f, 0.0f, 100.0f))
            {
                m_ActiveScene->SetExposure(exposure);
            }

            bool bloomEnabled = m_ActiveScene->GetBloomEnabled();
            if (ImGui::Checkbox("Bloom", &bloomEnabled))
            {
                m_ActiveScene->SetBloomEnabled(bloomEnabled);
            }

        }

        ImGui::Separator();
        ImGui::Text("Post Processing");
        uint32_t& postProcessingFlags = PostProcessing::GetInstance().GetPostProcessingFlags();
        uint32_t& kernelFlags = PostProcessing::GetInstance().GetKernelFlags();
        ImGui::CheckboxFlags("INVERSION", &postProcessingFlags, (uint32_t)PostProcessingFlags::INVERSION);
        ImGui::CheckboxFlags("GRAYSCALE", &postProcessingFlags, (uint32_t)PostProcessingFlags::GRAYSCALE);
        ImGui::CheckboxFlags("KERNEL", &postProcessingFlags, (uint32_t)PostProcessingFlags::KERNEL);
        ImGui::CheckboxFlags("SHARPEN", &kernelFlags, (uint32_t)KernelFlags::SHARPEN);
        ImGui::CheckboxFlags("BLUR", &kernelFlags, (uint32_t)KernelFlags::BLUR);

        ImGui::Separator();


        auto ssao = m_ActiveScene->GetSSAO();
        if (ImGui::DragInt("Kernel Size", &ssao.kernelSize, 1.0f, 0.0f, 1000.0f))
        {
            m_ActiveScene->SetSSAO(ssao);
        }

        if (ImGui::DragFloat("Radius", &ssao.radius, 0.001f, 0.0f, 100.0f))
        {
            m_ActiveScene->SetSSAO(ssao);
        }

        if (ImGui::DragFloat("Bias", &ssao.bias, 0.01f, 0.0f, 100.0f))
        {
            m_ActiveScene->SetSSAO(ssao);
        }

        if (ImGui::DragFloat("Power", &ssao.power, 0.1f, 0.0f, 100.0f))
        {
            m_ActiveScene->SetSSAO(ssao);
        }

        bool ssaoEnabled = ssao.ssaoEnabled == 0 ? false : true;
        if (ImGui::Checkbox("SSAO Enabled", &ssaoEnabled))
        {
            ssao.ssaoEnabled = ssaoEnabled ? true : false;
            m_ActiveScene->SetSSAO(ssao);
        }




        ImGui::End();

        m_SceneHierarchyPanel.OnImGuiRender();
        m_ContentBrowserPanel->OnImGuiRender();

        ImGuiPlayButton();

        // DockSpace
        ImGui::End();
    }

}