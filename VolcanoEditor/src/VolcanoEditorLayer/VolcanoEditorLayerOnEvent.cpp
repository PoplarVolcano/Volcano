#include "VolcanoEditorLayer.h"

#include "ImGuizmo.h"

namespace Volcano
{

    void VolcanoEditorLayer::OnEvent(Event& event)
    {
        if (m_SceneState == SceneState::Edit)
        {
            m_EditorCamera.OnEvent(event);
        }

        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(VOL_BIND_EVENT_FN(VolcanoEditorLayer::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(VOL_BIND_EVENT_FN(VolcanoEditorLayer::OnMouseButtonPressed));

    }

    bool VolcanoEditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        Input::Click(e.GetKeyCode());

        if (e.IsRepeat())
            return false;

        bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
        bool alt = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);

        switch (e.GetKeyCode())
        {
        case Key::Home: // 重新启动游戏
            if (m_SceneState != SceneState::Edit)
                OnSceneStop();
            if (!Project::GetActive()->GetPlayGame())
                OnScenePlay();
            Project::GetActive()->SetPlayGame(!Project::GetActive()->GetPlayGame());
            break;
        }

        // 编辑或模拟模式
        if (!Project::GetActive()->GetPlayGame())
        {
            switch (e.GetKeyCode())
            {
            case Key::C: // alt + C：清理控制台
                if (alt)
                    ClearConsole();
                break;
            case Key::N: // ctrl + N：新建场景
                if (control)
                    NewScene();
                break;
            case Key::O: // ctrl + O：打开项目
                if (control)
                    OpenProject();
                break;
            case Key::S:// ctrl + shift + S：另存场景  ctrl + S：保存场景
                if (control)
                    if (shift)
                        SaveSceneAs();
                    else
                        SaveScene();
                break;
            case Key::D: // ctrl + D：在场景目录下复制实体
                if (control)
                    OnDuplicateEntity();
                break;

                //Gizmos
            case Key::Q: // Q：关闭Gizmo
                if (!ImGuizmo::IsUsing())
                    m_GizmoType = -1;
                break;
            case Key::W: // W：移动Gizmo
                if (!ImGuizmo::IsUsing())
                    m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                break;
            case Key::E: // E：旋转Gizmo
                if (!ImGuizmo::IsUsing())
                    m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                break;
            case Key::R: // R：缩放Gizmo
                if (!ImGuizmo::IsUsing())  // ImGuizmo::IsUsing()用户当前是否正在和 Gizmo 控件进行交互
                    m_GizmoType = ImGuizmo::OPERATION::SCALE;
                break;
            case Key::T: // Ctrl + T：重载脚本
                if (control)
                       ScriptEngine::ReloadAssembly();
                    break;

                // TODO: 只有鼠标点击viewport的实体会被选中删除，SceneHierarchyPanel选中实体delete不能删除
            case Key::Delete:
                if (Application::GetInstance().GetImGuiLayer()->GetActiveWidgetID() == 0)
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
        }

        return false;
    }

    bool VolcanoEditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        Input::Click(e.GetMouseButton());

        if (e.GetMouseButton() == Mouse::ButtonLeft)
        {
            // 鼠标悬浮，没按Alt
            if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
            {

                auto [mx, my] = ImGui::GetMousePos();
                // 鼠标绝对位置减去窗口的左上角绝对位置=鼠标相对于窗口左上角的位置
                mx -= m_ViewportBounds[0].x;
                my -= m_ViewportBounds[0].y;
                // 窗口的右下角绝对位置-左上角的绝对位置=窗口的大小
                glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
                // 翻转y,使其左下角开始才是(0,0)
                my = viewportSize.y - my;
                int mouseX = (int)mx;
                int mouseY = (int)my;

                // 判定鼠标悬浮的实体entity，用于设置点击鼠标时选中的实体entity
                if (mouseX >= 0 && mouseY >= 0 && mouseX < viewportSize.x && mouseY < viewportSize.y)
                {
                    // 读取帧缓冲第二个缓冲区的数据
                    int pixelData = SceneRenderer::GetPostProcessingFramebuffer()->ReadPixelInt(1, mouseX, mouseY);
                    // TODO：若shader中没有输出EntityID，则这里会设置一个有错误ID的Entity并且无法读取会报错
                    //VOL_TRACE(pixelData);
                    auto entityTemp = m_ActiveScene->GetEntityEnttMap().find((entt::entity)pixelData);
                    if (entityTemp != m_ActiveScene->GetEntityEnttMap().end())
                        m_SceneHierarchyPanel.SetSelectedEntity(entityTemp->second);
                    else
                        m_SceneHierarchyPanel.SetSelectedEntity(nullptr);
                }


            }
        }
        return false;
    }
}