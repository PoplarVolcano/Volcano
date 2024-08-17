#include "ExampleLayer.h"

#include <glm/ext/matrix_transform.hpp>
#include "Volcano/Renderer/Renderer2D.h"

#include <chrono>
#include <imgui/imgui.h>
#include <Volcano/ImGui/ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

#include "Volcano/Scene/SceneSerializer.h"
#include "Volcano/Utils/PlatformUtils.h"
#include "Volcano/Math/Math.h"

static const uint32_t s_MapWidth = 24;
static const char* s_MapTiles = 
"WWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWDDDDDDWWWWWWWWWWW"
"WWWWWDDDDDDDDDDWWWWWWWWW"
"WWWWDDDDDDDDDDDDDDWWWWWW"
"WWWDDDDDDDDDDDDDBDDDWWWW"
"WWDDDDDDDDDDDDDDDDDDWWWW"
"WDDDDDWWWDDDDDDDDDDDDWWW"
"WWDDDDWWWDDDDDDDDDDDWWWW"
"WWWDDDDDDDDDDDDDDDDWWWWW"
"WWWWDDDDDDDDDDDDDDWWWWWW"
"WWWWWDDDDDDDDDDDDWWWWWWW"
"WWWWWWWDDDDDDDDWWWWWWWWW"
"WWWWWWWWWDDDDWWWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWW";

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
            //���ٶ���ʱ�������ʱ����δֹͣ �������¼����ʱ���
            auto endTimepoint = std::chrono::high_resolution_clock::now();
            long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
            long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
            m_Stopped = true;
            float duration = (end - start) * 0.001f;// ��λms
            m_Func({ m_Name, duration });
            //std::cout << "Timer:"<< m_Name << "ʱ�" << duration << "ms" << std::endl;
        }
    private:
        const char* m_Name;
        std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
        bool m_Stopped;
        Fn m_Func;
    };

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) { m_ProfileResults.push_back(profileResult); })

    ExampleLayer::ExampleLayer()
        : m_CameraController(1280.0f / 720.0f, true)
    {
    }

    ExampleLayer::~ExampleLayer()
    {
    }

    void ExampleLayer::OnAttach()
    {
        m_Texture = Texture2D::Create("assets/textures/Mostima.png");
        m_AlterTexture = Texture2D::Create("assets/textures/Ī˹����.png");
        m_SpriteSheet = Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");

        FramebufferSpecification fbSpec;
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        m_TextureStairs = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 7, 6 },  { 128, 128 });
        m_TextureTree = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2, 1 }, { 128, 128 }, { 1, 2 });

        m_MapWidth = s_MapWidth;
        m_MapHeight = strlen(s_MapTiles) / s_MapWidth;
        s_TextureMap['D'] = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 6, 11}, {128, 128});
        s_TextureMap['W'] = SubTexture2D::CreateFromCoords(m_SpriteSheet, { 11, 11 }, { 128, 128 });

        // Flames
        m_Particle.Position = { 0.0f, 0.0f };
        m_Particle.Velocity = { 0.0f, 0.0f }, m_Particle.VelocityVariation = { 1.0f, 1.0f };
        m_Particle.SizeBegin = 0.2f, m_Particle.SizeEnd = 0.0f, m_Particle.SizeVariation = 0.3f;
        m_Particle.ColorBegin = { 176 / 255.0f, 243 / 255.0f, 253 / 255.0f, 1.0f };
        m_Particle.ColorEnd = { 22 / 255.0f, 58 / 255.0f, 142 / 255.0f , 1.0f };
        m_Particle.LifeTime = 1.0f;

        //m_CameraController.SetZoomLevel(5.5f);

        m_ActiveScene = CreateRef<Scene>();

        /*
        //Entity
        auto square = m_ActiveScene->CreateEntity("Square");
        square.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.2f, 0.2f, 0.8f, 1.0f });

        auto redquare = m_ActiveScene->CreateEntity("RedSquare");
        redquare.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.8f, 0.2f, 0.2f, 1.0f });

        m_SquareEntity = square;

        m_CameraEntity = m_ActiveScene->CreateEntity("Camera A");
        m_CameraEntity.AddComponent<CameraComponent>();

        m_SecondCamera = m_ActiveScene->CreateEntity("Camera B");
        auto& cc = m_SecondCamera.AddComponent<CameraComponent>();
        cc.Primary = false;

        class CameraController : public ScriptableEntity
        {
        public:
            void OnCreate()
            {
                auto& translation = GetComponent<TransformComponent>().Translation;
                //translation.x = rand() % 10 - 5.0f;
            }

            void OnDestroy()
            {

            }

            void OnUpdate(Timestep ts)
            {
                auto& translation = GetComponent<TransformComponent>().Translation;
                auto& rotation = GetComponent<TransformComponent>().Rotation;
                float speed = 5.0f;

                if (Input::IsKeyPressed(VOL_KEY_A))
                    translation.x -= speed * ts;
                if (Input::IsKeyPressed(VOL_KEY_D))
                    translation.x += speed * ts;
                if (Input::IsKeyPressed(VOL_KEY_W))
                    translation.y += speed * ts;
                if (Input::IsKeyPressed(VOL_KEY_S))
                    translation.y -= speed * ts;

                if (Input::IsKeyPressed(VOL_KEY_Q))
                    rotation.z += speed * ts;
                if (Input::IsKeyPressed(VOL_KEY_E))
                    rotation.z -= speed * ts;


            }

        };
        m_CameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
        */
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);

    }

    void ExampleLayer::OnDetach()
    {
    }

    void ExampleLayer::OnUpdate(Timestep ts)
    {
        PROFILE_SCOPE("ExampleLayer::OnUpdate");

        if(m_ViewportFocused)
        {
            m_CameraController.OnUpdate(ts);
        }

        //Resize
        FramebufferSpecification spec = m_Framebuffer->GetSpecification();
        if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized Framebuffer is invalid
            (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            // ����ͼ�ĳߴ�ͬ����֡����ߴ�
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);

            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }

        Renderer2D::ResetStats();
        m_Framebuffer->Bind();
        Renderer::SetClearColor(0.2f, 0.2f, 0.2f, 1);
        Renderer::Clear();

        static float rotation = 0.0f;
        rotation += ts * 20.0f;

        // Background Scene
        /*
        Renderer2D::BeginScene(m_CameraController.GetCamera());
        {
            for (float y = -5.0f; y < 5.0f; y += 0.5f)
            {
                for (float x = -5.0f; x < 5.0f; x += 0.5f)
                {
                    glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 1.0f };
                    Renderer2D::DrawQuad({ x, y, -0.2f }, { 0.45f, 0.45f }, color);

                }
            }
            Renderer2D::EndScene();
        }
        */
        // SpriteSheet Scene
        /*
        Renderer2D::BeginScene(m_CameraController.GetCamera());
        {
            for (uint32_t y = 0; y < m_MapHeight; y++)
            {
                for (uint32_t x = 0; x < m_MapWidth; x++)
                {
                    char tileType = s_MapTiles[x + y * m_MapWidth];
                    Ref<SubTexture2D> texture;
                    if (s_TextureMap.find(tileType) != s_TextureMap.end())
                        texture = s_TextureMap[tileType];
                    else 
                        texture = m_TextureStairs;
                    Renderer2D::DrawQuad({ x - m_MapWidth / 2.0f, y - m_MapHeight / 2.0f, -0.1f }, { 1.0f, 1.0f }, texture);
                }

            }
            //Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 1.0f, 1.0f }, m_TextureStairs);
            //Renderer2D::DrawQuad({ -1.0f, 0.0f, -0.1f }, { 1.0f, 2.0f }, m_TextureTree);
            Renderer2D::EndScene();
        }
        */
        // Test Scene
        /*
        Renderer2D::BeginScene(m_SecondCamera.GetComponent<CameraComponent>().Camera, m_SecondCamera.GetComponent<CameraComponent>().Camera.GetProjection());
        {
            glm::vec4  redColor(0.8f, 0.3f, 0.3f, 1.0f);
            glm::vec4  blueColor(0.2f, 0.3f, 0.8f, 1.0f);

            Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.2f }, { 10.0f, 10.0f }, m_Texture);
            Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_AlterTexture);
            Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, redColor);
            Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, blueColor);
            Renderer2D::DrawRotatedQuad({ 1.0f, 0.0f }, { 0.8f,0.8f }, glm::radians(rotation), redColor);
            Renderer2D::DrawRotatedQuad({ -2.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, glm::radians(rotation), m_AlterTexture, 10.0f);
            Renderer2D::EndScene();
        }
        */



        m_ActiveScene->OnUpdate(ts);


        // Particle Scene
        /*
        Renderer2D::BeginScene(m_CameraEntity.GetComponent<CameraComponent>().Camera, m_CameraEntity.GetComponent<CameraComponent>().Camera.GetProjection());
        {
            //������¿ո�
            if (Input::IsMouseButtonPressed(VOL_MOUSE_BUTTON_LEFT))
            {
                auto [x, y] = Input::GetMousePosition();
                auto width = Application::Get().GetWindow().GetWidth();
                auto height = Application::Get().GetWindow().GetHeight();

                auto bounds = m_CameraController.GetBounds();
                auto pos = m_CameraController.GetCamera().GetPosition();
                x = (x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
                y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();
                m_Particle.Position = { x + pos.x, y + pos.y };
                for (int i = 0; i < 5; i++)
                    m_ParticleSystem.Emit(m_Particle);
            }
            m_ParticleSystem.OnUpdate(ts);
            m_ParticleSystem.OnRender();
            Renderer2D::EndScene();
        }
        */
        m_Framebuffer->Unbind();
    }

    void ExampleLayer::OnImGuiRender()
    {
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
        //��ʹ��ImGuiDockNodeFlags_PassthruCentralNodeʱ��DockSpace��������Ⱦ���ǵı���������ͨ�ף��������Ҫ��Begin��������Ⱦ������
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        //��Ҫ��ʾ����ע�⣬��ʹBegin��������false���������۵���������Ҳ������� 
        //������Ϊ�����뱣��DockSpace�������ڻ״̬�����DockSpace�������ڷǻ״̬�� 
        //����ͣ�������еĻ���ڶ���ʧȥ�丸���ڲ�ȡ��ͣ���� 
        //�����޷���������ںͷǻͣ��֮���ͣ����ϵ������ 
        //ͣ���ռ�/���õ��κθ��Ķ��ᵼ�´���������������Զ���ɼ���
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
                //����ȫ�����������ƶ����������ڵ�ǰ�棬���û�и���ϸ�Ĵ������/z���ƣ�����Ŀǰ�޷�������
                ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
                ImGui::MenuItem("Padding", NULL, &opt_padding);
                ImGui::Separator();

                if (ImGui::MenuItem("New", "Ctrl+N"))
                    NewScene();

                if (ImGui::MenuItem("Open...", "Ctrl+O"))
                    OpenScene();

                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                    SaveSceneAs();

                if (ImGui::MenuItem("Exit")) 
                    Application::Get().Close();

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // =====================================================Viewport=====================================================
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Viewport");

        // �����Ƿ�ѡ��
        m_ViewportFocused = ImGui::IsWindowFocused();
        // ����Ƿ������ڴ�����
        m_ViewportHovered = ImGui::IsWindowHovered();
        // ֻ��ͬʱѡ�к������Ż�ִ��Event
        Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image((void*)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{0, 1}, ImVec2{1, 0});

        //Gizmos
        Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity && m_GizmoType != -1)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            float windowWidth = (float)ImGui::GetWindowWidth();
            float windowHeight = (float)ImGui::GetWindowHeight();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
            
            //Camera
            auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
            const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
            const glm::mat4& cameraProjection = camera.GetProjection();
            glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

            //Entity transform
            auto& tc = selectedEntity.GetComponent<TransformComponent>();
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
                glm::value_ptr(cameraView), 
                glm::value_ptr(cameraProjection),
                (ImGuizmo::OPERATION)m_GizmoType, 
                ImGuizmo::LOCAL, 
                glm::value_ptr(transform),
                nullptr,
                snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                // Ԥ�����������
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

        // =====================================================Settings=====================================================
        ImGui::Begin("Settings");

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

        // Settings
        ImGui::End();

        //ImGui::ShowDemoWindow();

        // Dockspace
        ImGui::End();

    }

    void ExampleLayer::OnEvent(Event& event)
    {
        m_CameraController.OnEvent(event);
        
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(VOL_BIND_EVENT_FN(ExampleLayer::OnKeyPressed));

    }
    bool ExampleLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.GetRepeatCount() > 0) 
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
                OpenScene();
            break;
        case Key::S:
            if (control && shift)
                SaveSceneAs();
            break;

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
            m_GizmoType = ImGuizmo::OPERATION::SCALE;
            break;
        }
        return false;
    }

    void ExampleLayer::NewScene()
    {
        m_ActiveScene = CreateRef<Scene>();
        m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
    }

    void ExampleLayer::OpenScene()
    {
        std::string filepath = FileDialogs::OpenFile("Volcano Scene (*.volcano)\0*.volcano\0");
        if (!filepath.empty())
        {
            m_ActiveScene = CreateRef<Scene>();
            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_SceneHierarchyPanel.SetContext(m_ActiveScene);

            SceneSerializer serializer(m_ActiveScene);
            serializer.Deserialize(filepath);
        }
    }

    void ExampleLayer::SaveSceneAs()
    {
        std::string filepath = FileDialogs::SaveFile("Volcano Scene (*.volcano)\0*.volcano\0");
        if (!filepath.empty())
        {
            SceneSerializer serializer(m_ActiveScene);
            serializer.Serialize(filepath);
        }
    }
}