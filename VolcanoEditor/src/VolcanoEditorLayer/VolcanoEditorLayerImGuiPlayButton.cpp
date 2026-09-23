#include "VolcanoEditorLayer.h"

namespace Volcano
{
    void VolcanoEditorLayer::ImGuiPlayButton()
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

        bool hasPlayButton = m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play;
        bool hasSimulateButton = m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate;
        bool hasPauseButton = m_SceneState != SceneState::Edit;

        // 开始暂停按钮
        if (hasPlayButton)
        {
            Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) ? m_IconPlay : m_IconStop;
            if (ImGui::ImageButton("IconPlayOrStop", (ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
            {
                if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate)
                    OnScenePlay();
                else if (m_SceneState == SceneState::Play)
                    OnSceneStop();
            }
        }

        if (hasSimulateButton)
        {
            if (hasPlayButton)
                // 模拟按钮
                ImGui::SameLine();
            Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play) ? m_IconSimulate : m_IconStop;

            if (ImGui::ImageButton("IconSimulateOrStop", (ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
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
                if (ImGui::ImageButton("IconPlayOrPause", (ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
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
                    if (ImGui::ImageButton("IconStop", (ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
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
}
