#include "VolcanoEditorLayer.h"

#include "Volcano/Core/AppPath.h"

namespace Volcano
{
    void VolcanoEditorLayer::NewProject(std::filesystem::path newProjectAbsolutePath, const std::string projectName)
    {
        Project::New(newProjectAbsolutePath, projectName);
    }

    void VolcanoEditorLayer::OpenProject(const std::filesystem::path& projectFileAbsolutePath)
    {
        if (Project::Load(projectFileAbsolutePath))
        {
            ScriptEngine::ReloadAssembly();
            auto startScenePath = Project::GetAssetFileSystemPath(Project::GetActive()->GetConfig().startScene);
            // 场景路径为空 或 找不到文件 则 新建场景 或 文件选择框选择场景
            if (Project::GetActive()->GetConfig().startScene.empty() || !std::filesystem::exists(startScenePath))
                OpenScene();
            else
                OpenScene(startScenePath);
            m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();
            m_ContentBrowserPanel->SetSceneHierarchyPanel(&m_SceneHierarchyPanel);
            m_ContentBrowserPanel->SetLayer(this);
        }
    }

    bool VolcanoEditorLayer::OpenProject()
    {
        // 打开.proj文件读取Project
        std::string projectFileAbsolutePath = FileDialogs::OpenFile("Volcano Project (*.proj)\0*.proj\0", AppPath::GetInstance().GetExePath().string());
        if (projectFileAbsolutePath.empty())
            return false;

        OpenProject(projectFileAbsolutePath);
        return true;
    }

    void VolcanoEditorLayer::SaveProject()
    {
        Project::SaveActive();
    }

    void VolcanoEditorLayer::NewScene()
    {
        m_EditorScene = CreateRef<Scene>();
        m_SceneHierarchyPanel.SetScene(m_EditorScene);
        m_EditorSceneFileAbsolutePath = std::filesystem::path();
        m_ActiveScene = m_EditorScene;
    }

    void VolcanoEditorLayer::OpenScene()
    {
        std::string filepath = FileDialogs::OpenFile("Volcano Scene (*.scene)\0*.scene\0", Project::GetAssetsAbsolutePath().string());
        if (!filepath.empty())
            OpenScene(filepath);
        else
        {
            if (m_SceneState != SceneState::Edit)
                OnSceneStop();
            NewScene();
        }
    }

    void VolcanoEditorLayer::OpenScene(const std::filesystem::path& sceneFileAbsolutePath)
    {
        if (m_SceneState != SceneState::Edit)
            OnSceneStop();

        if (sceneFileAbsolutePath.extension().string() != ".scene")
        {
            VOL_WARN("Could not load {0} - not a scene file", sceneFileAbsolutePath.filename().string());
            return;
        }

        Ref<Scene> newScene = CreateRef<Scene>();
        SceneSerializer serializer(newScene);
        if (serializer.Deserialize(sceneFileAbsolutePath.string()))
        {
            // 编辑器场景获取文件读取场景
            m_EditorScene = newScene;
            m_SceneHierarchyPanel.SetScene(m_EditorScene);
            m_EditorSceneFileAbsolutePath = sceneFileAbsolutePath;
            m_ActiveScene = m_EditorScene;
        }
    }

    void VolcanoEditorLayer::SaveScene()
    {
        if (!m_EditorSceneFileAbsolutePath.empty())
        {
            SerializeScene(m_ActiveScene, m_EditorSceneFileAbsolutePath);
            m_EditorScene = m_ActiveScene;
            if (m_SceneState != SceneState::Edit)
                OnSceneStop();
        }
        else
            SaveSceneAs();
    }

    void VolcanoEditorLayer::SaveSceneAs()
    {
        std::string filepath = FileDialogs::SaveFile("Volcano Scene (*.scene)\0*.scene\0", Project::GetAssetsAbsolutePath().string());
        if (!filepath.empty())
        {
            // 检查是否以 .scene 结尾,自动补后缀
            if (filepath.size() < 6)
                filepath += ".scene";
            else if (filepath.substr(filepath.size() - 6) != ".scene")
                filepath += ".scene";

            SerializeScene(m_ActiveScene, filepath);
            m_EditorSceneFileAbsolutePath = filepath;
            m_EditorScene = m_ActiveScene;
            if (m_SceneState != SceneState::Edit)
                OnSceneStop();
        }
    }

    // 序列化场景
    void VolcanoEditorLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& sceneFileAbsolutePath)
    {
        SceneSerializer serializer(scene);
        scene->SetName(FileUtils::GetFileNameFromPath(sceneFileAbsolutePath.string()));
        scene->SetFileRelativePath(sceneFileAbsolutePath.string());
        serializer.Serialize(sceneFileAbsolutePath.string());
    }

    // 播放场景
    void VolcanoEditorLayer::OnScenePlay()
    {
        // 场景状态为模拟时，重置场景
        if (m_SceneState == SceneState::Simulate)
            OnSceneStop();

        // 设置场景状态播放
        m_SceneState = SceneState::Play;

        // 复制编辑器场景，得到一个新的Scene并赋给活动场景
        m_ActiveScene = Scene::Copy(m_EditorScene);

        // 设置新的Scene为运行状态
        m_ActiveScene->SetRunning(true);

        // 设置活动场景物理效果
        m_ActiveScene->OnRuntimeStart();

        m_SceneHierarchyPanel.SetScene(m_ActiveScene);
    }

    void VolcanoEditorLayer::OnSceneSimulate()
    {
        // 物理模拟时运行则重置场景
        if (m_SceneState == SceneState::Play)
            OnSceneStop();

        // 设置场景状态：模拟
        m_SceneState = SceneState::Simulate;

        m_ActiveScene = Scene::Copy(m_EditorScene);

        m_SceneHierarchyPanel.SetScene(m_ActiveScene);

        m_ActiveScene->SetRunning(true);

        m_ActiveScene->OnSimulationStart();

    }

    // 暂停场景
    void VolcanoEditorLayer::OnScenePause()
    {
        VOL_CORE_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate);

        if (m_SceneState == SceneState::Edit)
            return;
        m_ActiveScene->SetPaused(true);
        m_ActiveScene->SetRunning(false);
    }

    // 重置场景
    void VolcanoEditorLayer::OnSceneStop()
    {
        VOL_CORE_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate);

        // 关闭活动场景物理效果
        if (m_SceneState == SceneState::Play)
            m_ActiveScene->OnRuntimeStop();
        else if (m_SceneState == SceneState::Simulate)
            m_ActiveScene->OnSimulationStop();

        // 设置场景状态：编辑
        m_SceneState = SceneState::Edit;

        m_SceneHierarchyPanel.SetScene(m_EditorScene);

        // 活动场景恢复编辑器场景
        m_ActiveScene = m_EditorScene;

        // edit模式下running必然false，可删除
        m_ActiveScene->SetRunning(false);

        MouseBuffer::instance().SetOnActive(true);
    }

    // 编辑模式可用，如果选中实体，则复制一个新实体
    void VolcanoEditorLayer::OnDuplicateEntity()
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

    void VolcanoEditorLayer::OnOverlayRender()
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
            // 方框碰撞体
            {
                auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
                for (auto [entity, tc, bc2d] : view.each())
                {
                    //auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);

                    // 0.001f Z轴偏移量
                    glm::vec3 translation = tc.translation + glm::vec3(bc2d.offset, 0.001f);
                    // bc2d.Size需乘以2，以免缩小一半
                    glm::vec3 scale = tc.scale * glm::vec3(bc2d.size * 2.0f, 1.0f);

                    glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
                        * glm::rotate(glm::mat4(1.0f), glm::radians(tc.inspectorEulerHint.z), glm::vec3(0.0f, 0.0f, 1.0f))
                        * glm::scale(glm::mat4(1.0f), scale);

                    // 绿色的包围盒
                    Renderer2D::DrawRect(transform, glm::vec4(0, 1, 0, 1));
                }
            }

            // 圆碰撞体
            {
                auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
                for (auto [entity, tc, cc2d] : view.each())
                {
                    //auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(entity);

                    glm::vec3 translation = tc.translation + glm::vec3(cc2d.offset, 0.01f);
                    // cc2d.Radius需乘以2，以免缩小一半
                    glm::vec3 scale = tc.scale * glm::vec3(cc2d.radius * 2.0f);

                    glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
                        * glm::scale(glm::mat4(1.0f), scale);

                    Renderer2D::DrawCircle(transform, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.1f);
                }
            }
        }

        // 绘制选定实体的轮廓
        if (Ref<Entity> selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity())
        {
            Renderer2D::SetLineWidth(5.0);
            Renderer2D::DrawRect(selectedEntity->GetWorldTransform(), glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
        }

        Renderer2D::EndScene();
    }

    void VolcanoEditorLayer::ClearConsole()
    {
        Application::GetInstance().ClearConsole();
    }

}