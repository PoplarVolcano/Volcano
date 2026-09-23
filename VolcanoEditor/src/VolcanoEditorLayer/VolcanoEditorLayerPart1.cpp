#include "VolcanoEditorLayer.h"

#include "EditorTimer.h"

#include "Volcano/Core/AppPath.h"

namespace Volcano
{
    Volcano::VolcanoEditorLayer::VolcanoEditorLayer()
        : Layer("VolcanoEditorLayer")
    {
    }

    VolcanoEditorLayer::~VolcanoEditorLayer()
    {
    }

    void VolcanoEditorLayer::OnAttach()
    {
        ScriptEngine::Init();

        // 从app获取指令行，如果指令行大于1则读取场景
        auto commandLineArgs = Application::GetInstance().GetSpecification().CommandLineArgs;
        if (commandLineArgs.Count > 1)
        {
            auto projectFilePath = commandLineArgs[1];
            OpenProject(projectFilePath);
        }
        else {
            // 引导用户选择一个项目路径，如果没有打开项目则关闭编辑器
            if (OpenProject())
                m_ProjectLoaded = true;
            else
                m_NewProject = true;
        }

        Volcano::ImGuiContextManager::MakeCurrent();

        m_IconPlay     = Texture2D::Create((AppPath::GetInstance().GetExePath() / "Resources/Icons/PlayButton.png").string());
        m_IconPause    = Texture2D::Create((AppPath::GetInstance().GetExePath() / "Resources/Icons/PauseButton.png").string());
        m_IconStop     = Texture2D::Create((AppPath::GetInstance().GetExePath() / "Resources/Icons/StopButton.png").string());
        m_IconSimulate = Texture2D::Create((AppPath::GetInstance().GetExePath() / "Resources/Icons/SimulateButton.png").string());
        m_IconStep     = Texture2D::Create((AppPath::GetInstance().GetExePath() / "Resources/Icons/StepButton.png").string());

        m_EditorCamera = EditorCamera(30.0f, 1.788f, 0.1f, 1000.0f);

        m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();
        m_ContentBrowserPanel->SetSceneHierarchyPanel(&m_SceneHierarchyPanel);
        m_ContentBrowserPanel->SetLayer(this);

    }

    void VolcanoEditorLayer::OnDetach()
    {
    }

    void VolcanoEditorLayer::OnUpdate()
    {
        PROFILE_SCOPE("VolcanoEditorLayer::OnUpdate");

        if (!m_ProjectLoaded)
            return;

        m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        if (m_SceneState == SceneState::Play)
            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        
        Ref<Volcano::Framebuffer> framebuffer = SceneRenderer::GetPostProcessingFramebuffer();

        //Resize
        FramebufferSpecification spec = framebuffer->GetSpecification();
        if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized Framebuffer is invalid
            (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            // 将视图的尺寸同步到帧缓冲尺寸
            framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        }


        // 重置渲染计数
        Renderer2D::ResetStats();

        // 场景更新
        switch (m_SceneState)
        {
        case SceneState::Edit:
            m_EditorCamera.OnUpdate();
            m_ActiveScene->OnUpdateEditor(m_EditorCamera);
            break;
        case SceneState::Simulate:
            m_EditorCamera.OnUpdate();
            m_ActiveScene->OnUpdateSimulation(m_EditorCamera);
            break;
        case SceneState::Play:
            m_ActiveScene->OnUpdateRuntime();
            break;
        }

        Renderer::SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        Renderer::Clear();

        //============================================SceneRender=============================================
        SceneRenderer::BeginScene(m_ActiveScene, m_SceneState, m_EditorCamera);
        SceneRenderer::PreProcessing();
        SceneRenderer::GBuffer();
        SceneRenderer::MidProcessing();
        SceneRenderer::DeferredShading();
        SceneRenderer::PostProcessing();
        SceneRenderer::EndScene();
        framebuffer->Bind();
        OnOverlayRender();
        framebuffer->Unbind();
    }


}