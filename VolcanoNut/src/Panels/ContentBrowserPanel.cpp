#include "volpch.h"
#include "ContentBrowserPanel.h"
#include <imgui/imgui.h>

#include "Volcano/Project/Project.h"

namespace Volcano {

    ContentBrowserPanel::ContentBrowserPanel() 
        : m_BaseDirectory(Project::GetAssetDirectory()), m_CurrentDirectory(m_BaseDirectory)
    {
        m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
        m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");
    }

	void ContentBrowserPanel::OnImGuiRender()
	{
        ImGui::Begin("Content Browser");

        // Ϊ�˷�����һ��Ŀ¼
        // ��ǰĿ¼ != assetsĿ¼
        if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory)) {
            // �������˰�ť
            if (ImGui::Button("<-")) {
                // ��ǰĿ¼ = ��ǰĿ¼�ĸ�Ŀ¼
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
        }

        static float padding = 8.0f * 1.5f;
        static float thumbnailSize = 64.0f * 1.5f;
        float cellSize = thumbnailSize + padding;

        // ��ȡ���ڿ��
        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1)
            columnCount = 1;

        // ������(һ�м��У�
        ImGui::Columns(columnCount, 0, false);

		// Ŀ¼������
        if (std::filesystem::exists(m_CurrentDirectory))
        {
            for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
                // �õ����ļ��л��ļ�path�ࡣ					���磺path = assets\cache\shader
                const auto& path = directoryEntry.path();
                // ��ȡ���ļ����ļ�����						    filenameString = shader
                std::string filenameString = path.filename().string();

                // ImageButton������icon��TextureID������ͬһ��Scene�ļ����µ�����.scene�ļ�����ʹ�����TextureID��
                // �����forѭ��������pinkcube.scene�����������ǰ��ĳ��������棬����ÿ�λ�ȡ�����ݶ���pinkcube.scene
                // ���ļ�����ID�����ֲ�ͬ��ť
                // ������ImageButtonEx����һ��������ID
                ImGui::PushID(filenameString.c_str());

                // ���ô�ͼ�갴ť
                Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
                // ����button��ɫΪ��
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

                if (ImGui::BeginDragDropSource())
                {
                    // �õ����ļ���assets�ļ��е����λ��path��	    relativePath = cache\shader
                    std::filesystem::path relativePath(path);
                    // ��������Դ��c_str�����Կ��ַ���β��const�Ŀ��ַ���
                    const wchar_t* itemPath = relativePath.c_str();
                    // wcslen: returns the length of a wide string, that is the number of non-null wide characters that precedethe terminating null wide character
                    // c�ַ��������һ��\0��־Ϊ�����룬����Ҫ+1
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
                    ImGui::EndDragDropSource();
                }
                ImGui::PopStyleColor();
                // �����button����˫�����
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    if (directoryEntry.is_directory())
                        m_CurrentDirectory /= path.filename();
                    else
                        VOL_CORE_TRACE((m_CurrentDirectory / path.filename()).string());
                }
                ImGui::TextWrapped(filenameString.c_str());
                ImGui::NextColumn();

                ImGui::PopID();
            }
        }

        ImGui::Columns(1);

        // ����button�ߴ�
        ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
        // �����м��
        ImGui::SliderFloat("Padding", &padding, 0, 32);

        ImGui::End();
	}
}