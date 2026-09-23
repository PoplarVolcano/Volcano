#include "SceneHierarchyPanel.h"

namespace Volcano
{
	void SceneHierarchyPanel::DrawSkyboxComponent(SkyboxComponent& component)
	{
		const char* items[] = { "CubeMap", "6 Sided" };
		// 下拉选择框（Combo Box / Dropdown）
		ImGui::Combo("TextureType", &component.textureType, items, IM_ARRAYSIZE(items));

		ImGui::Checkbox("Enabled", &component.enabled);

		ImGui::Checkbox("Primary", &component.primary);

		if (component.textureType == 0)
		{
			if (component.textureCubeMap != nullptr)
			{
				float aspect = (float)component.textureCubeMap->GetWidth() / (float)component.textureCubeMap->GetHeight();
				if (ImGui::ImageButton("TextureCube", (void*)(intptr_t)component.textureCubeMap->GetRendererID(), ImVec2(100.0f * aspect, 100.0f)))
					component.ResetTextureCubeMap();;
			}
			else
			{
				if (ImGui::Button("TextureCube"))
					component.ResetTextureCubeMap();
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texturePath = path;

					if (texturePath.extension() == ".png" || texturePath.extension() == ".jpg")
						component.textureCubeMap = TextureCube::Create(texturePath.string());
				}
				ImGui::EndDragDropTarget();
			}
		}
		else if (component.textureType == 1)
		{
			const char* buttons[] = { "TextureRight", "TextureLeft", "TextureTop", "TextureBottom", "TextureFront", "TextureBack" };
			for (uint32_t i = 0; i != 6; i++)
			{
				ImGui::PushID(i);

				ImGui::Columns(2);
				ImGui::SetColumnWidth(0, 150.0f);

				auto& texParam = component.textures[i];
				if (texParam.key != std::string())
				{
					Ref<Texture2D> texture = Texture::GetTextureLibrary()->Get(texParam.key, texParam.keyHash, texParam.flip);
					float aspect = (float)texture->GetWidth() / (float)texture->GetHeight();
					if (ImGui::ImageButton(buttons[i], (void*)(intptr_t)texture->GetRendererID(), ImVec2(100.0f * aspect, 100.0f)))
					{
						texParam.key = std::string();
						texParam.keyHash = 0;
						texParam.flip = true;
						component.ResetTextureCubeSixSided();
					}
				}
				else
				{
					if (ImGui::Button(buttons[i]))
					{
						texParam.key = std::string();
						texParam.keyHash = 0;
						texParam.flip = true;
						component.ResetTextureCubeSixSided();
					}
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path filePath = path;

						if (filePath.extension() == ".png" || filePath.extension() == ".jpg")
						{
							texParam.key = filePath.string();
							texParam.keyHash = 0;
							texParam.flip = false;
							component.ResetTextureCubeSixSided();
						}
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::NextColumn();

				if (ImGui::Button("flip"))
				{
					texParam.flip = !texParam.flip;
					component.ResetTextureCubeSixSided();
				}

				ImGui::EndColumns();
				ImGui::PopID();

				ImGui::Separator();
			}
		}
	}

}