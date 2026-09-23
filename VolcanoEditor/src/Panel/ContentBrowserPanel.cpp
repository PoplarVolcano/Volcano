#include "volpch.h"
#include "ContentBrowserPanel.h"
#include <imgui/imgui.h>

#include "VolcanoEditorLayer/VolcanoEditorLayer.h"
#include "Volcano/Core/AppPath.h"

namespace Volcano {

    ContentBrowserPanel::ContentBrowserPanel()
        : m_BaseDirectory(Project::GetAssetsAbsolutePath()), m_CurrentDirectory(m_BaseDirectory)
    {
        auto directoryIconAbsolutePath = AppPath::GetInstance().GetExePath() / "Resources/Icons/ContentBrowser/DirectoryIcon.png";
        auto fileIconAbsolutePath = AppPath::GetInstance().GetExePath() / "Resources/Icons/ContentBrowser/FileIcon.png";
        m_DirectoryIcon = Texture2D::Create(directoryIconAbsolutePath.string());
        m_FileIcon = Texture2D::Create(fileIconAbsolutePath.string());
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Content Browser");

        // 为了返回上一级目录
        // 当前目录 != assets目录
        if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory)) {
            // 如果点击了按钮
            if (ImGui::Button("<-")) {
                // 当前目录 = 当前目录的父目录
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
        }

        static float padding = 8.0f * 1.5f;
        static float thumbnailSize = 64.0f * 1.5f;
        float cellSize = thumbnailSize + padding;

        // 获取窗口宽度
        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1)
            columnCount = 1;

        // 设置列(一行几列）
        ImGui::Columns(columnCount, 0, false);

        // 目录迭代器
        if (std::filesystem::exists(m_CurrentDirectory))
        {
            for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
            {
                // 遍历m_CurrentDirectory文件夹下所有文件
                // directoryEntry为子文件

                // 得到子文件夹或文件path类。					比如：path = assets\cache\shader
                const auto& path = directoryEntry.path();
                // 获取子文件的文件名。						    filenameString = shader
                std::string filenameString = path.filename().string();

                // ImageButton传入了icon的TextureID，这样同一个Scene文件夹下的所有.scene文件都是使用这个TextureID，
                // 外层有for循环遍历，pinkcube.scene在最后，他会在前面的场景最上面，所以每次获取的数据都是pinkcube.scene
                // 用文件名做ID，区分不同按钮
                // 可以用ImageButtonEx，第一个参数是ID
                ImGui::PushID(filenameString.c_str());

                // 设置带图标按钮
                Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
                // 设置button颜色为空
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::ImageButton("", (ImTextureRef)icon->GetRendererID(), {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});

                if (ImGui::BeginDragDropSource())
                {
                    // 得到子文件与assets文件夹的相对位置path。	    relativePath = cache\shader
                    std::filesystem::path relativePath(path);
                    // 设置数据源，c_str返回以空字符结尾的const的宽字符串
                    const wchar_t* itemPath = relativePath.c_str();
                    // wcslen: returns the length of a wide string, that is the number of non-null wide characters that precedethe terminating null wide character
                    // c字符串的最后一个\0标志为不计入，所以要+1
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
                    ImGui::EndDragDropSource();
                }
                ImGui::PopStyleColor();
                // 鼠标在button上且双击左键
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    if (directoryEntry.is_directory())
                        m_CurrentDirectory /= path.filename();
                   
                }
                ImGui::TextWrapped(filenameString.c_str());
                ImGui::NextColumn();

                ImGui::PopID();
            }

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::Button("##EmptyContentBrowserItem", { thumbnailSize, thumbnailSize });
            ImGui::PopStyleColor();
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_NODE"))
                {
                    const Volcano::UUID* entityID = (const Volcano::UUID*)payload->Data;
                    auto& entityIDMap = m_SceneHierarchyPanel->GetScene()->GetEntityIDMap();
                    VOL_CORE_ASSERT(entityIDMap.find(*entityID) != entityIDMap.end());
                    Ref<Entity> entityTemp = entityIDMap.at(*entityID);
                   
                }
                ImGui::EndDragDropTarget();
            }

        }

        ImGui::Columns(1);

        // 设置button尺寸
        ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
        // 设置列间隔
        ImGui::SliderFloat("Padding", &padding, 0, 32);

        ImGui::End();

    }
    void ContentBrowserPanel::SetLayer(Layer* layer) { m_Layer = layer; }
}