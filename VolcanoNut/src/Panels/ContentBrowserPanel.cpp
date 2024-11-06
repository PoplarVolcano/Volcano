#include "volpch.h"
#include "ContentBrowserPanel.h"
#include <imgui/imgui.h>

#include "Volcano/Project/Project.h"
#include "Volcano/Renderer/SceneRenderer.h"
#include "Volcano/Scene/SceneSerializer.h"
#include "Volcano/Scene/Entity.h"
#include "Volcano/Scene/Prefab.h"
#include "ExampleLayer.h"

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
            for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory)) 
            {
                // ����m_CurrentDirectory�ļ����������ļ�
                // directoryEntryΪ���ļ�
                
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
                    {
                        // ˫��.prefab�ļ����л���Prefab������ѡ�ж�ӦprefabEntity
                        if (path.extension() == ".prefab")
                        {
                            Ref<Entity> prefabEntity = Prefab::Get(Project::GetRelativeAssetDirectory(path).string());
                            if (prefabEntity == nullptr)
                            {
                                prefabEntity = Prefab::Load(path);
                            }
                            m_ExampleLayer->SetEditorSceneTemp(Prefab::GetScene());
                            m_SceneHierarchyPanel->SetContext(Prefab::GetScene());
                            m_SceneHierarchyPanel->SetSelectedEntity(prefabEntity);
                        }
                        VOL_CORE_TRACE(path.string());
                    }
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
                    const UUID* entityID = (const UUID*)payload->Data;
                    auto& entityIDMap = m_SceneHierarchyPanel->GetContext()->GetEntityIDMap();
                    VOL_CORE_ASSERT(entityIDMap.find(*entityID) != entityIDMap.end());
                    Ref<Entity> entityTemp = entityIDMap.at(*entityID);
                    if (entityTemp != nullptr)
                    {
                        // �����ӦPrefab�Ѿ��������Ѿ���ȡ����ȡ�Ѷ�ȡprefab��ID���浽�µ�prefab������ʹ����ID
                        std::string prefabName = entityTemp->GetName() + ".prefab";
                        std::filesystem::path prefabPath = Project::GetRelativeAssetDirectory(m_CurrentDirectory / prefabName);
                        Ref<Entity> prefabEntity = Prefab::Get(prefabPath.string());
                        UUID newID;
                        if (prefabEntity != nullptr)
                            newID = prefabEntity->GetUUID();
                        else
                            newID = UUID();
                        SceneSerializer serializer(SceneRenderer::GetActiveScene());
                        serializer.SerializePrefab((m_CurrentDirectory / (prefabName)).string(), *entityTemp.get(), &newID);
                    }
                }
                ImGui::EndDragDropTarget();
            }

        }

        ImGui::Columns(1);

        // ����button�ߴ�
        ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
        // �����м��
        ImGui::SliderFloat("Padding", &padding, 0, 32);

        ImGui::End();

	}
    void ContentBrowserPanel::SetExampleLayer(ExampleLayer* exampleLayer) { m_ExampleLayer = exampleLayer; }
}