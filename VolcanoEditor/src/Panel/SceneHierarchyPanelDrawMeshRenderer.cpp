#include "SceneHierarchyPanel.h"

#include "glm/gtc/type_ptr.hpp"

namespace Volcano
{
	void SceneHierarchyPanel::DrawMeshRendererComponent(MeshRendererComponent& component)
	{
		ImGui::Checkbox("Enabled", &component.enabled);

		ImGui::Separator();

		const char* materialTypeItems[] = { "Diffuse", "Specular", "Normal", "Parallax", "Roughness", "AO", "Emission"};
		const char* lightingModeItems[] = { "Unlit", "BlinnPhong", "PBR"};

		auto* materialLibrary = m_Scene->GetMaterialLibrary();

		int materialLibraryIndex = materialLibrary->GetIndex(component.materialLibraryKey);

		if (ImGui::Combo("Material", &materialLibraryIndex, materialLibrary->GetMaterialNamesCStr().data(), (int)materialLibrary->GetMaterialNamesCStr().size()))
		{
			auto* key = materialLibrary->GetKey(materialLibraryIndex);
			if (key != nullptr)
			{
				component.materialLibraryKey = *key;
			}
			else
			{
				// combo选中的索引无效，回滚到上一个索引。
				materialLibraryIndex = materialLibrary->GetIndex(component.materialLibraryKey);
			}
		}


		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strncpy_s(buffer, sizeof(buffer), component.materialLibraryKey.key.c_str(), sizeof(buffer));

		ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_EnterReturnsTrue;

		if (ImGui::InputText("##MaterialName", buffer, sizeof(buffer), inputTextFlags))
		{
			MaterialLibraryKey newKey = { buffer, std::hash<std::string>{}(buffer) };
			materialLibrary->UpdateMaterialName(component.materialLibraryKey, newKey);
			component.materialLibraryKey = newKey;
		}

		ImGui::SameLine();

		if (ImGui::Button("+"))
		{
			component.materialLibraryKey = materialLibrary->AddMaterial();
		}

		ImGui::SameLine();

		if (ImGui::Button("-"))
		{
			materialLibrary->RemoveMaterial(component.materialLibraryKey);
			component.materialLibraryKey = *materialLibrary->GetKey(0);
		}

		const Material* materialPtr = materialLibrary->GetMaterial(component.materialLibraryKey);
		if (materialPtr == nullptr)
		{
			component.materialLibraryKey = *materialLibrary->GetKey(0);
			materialPtr = materialLibrary->GetMaterial(component.materialLibraryKey);
		}

		int lightingMode = materialPtr->lightingMode;
		if (ImGui::Combo("Lighting Mode", &lightingMode, lightingModeItems, IM_ARRAYSIZE(lightingModeItems)))
		{
			materialLibrary->UpdateMaterial(component.materialLibraryKey, lightingMode);
		}

		std::array<const TextureLibraryKey*, s_MaterialTypeCount> materialTemp = {
			&materialPtr->diffuse,
			&materialPtr->specular,
			&materialPtr->normal,
			&materialPtr->parallax,
			&materialPtr->roughness,
			&materialPtr->ao,
			&materialPtr->emission
		};

		for (uint8_t i = 0; i != s_MaterialTypeCount; i++)
		{
			ImGui::PushID(i);

			auto& textureLibraryKey = *materialTemp[i];


			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.0f);
			ImGui::Text(materialTypeItems[i]);
			ImGui::NextColumn();

			if (ImGui::Button("flip"))
			{
				auto textureLibraryKeyTemp = textureLibraryKey;
				textureLibraryKeyTemp.flip = !textureLibraryKeyTemp.flip;
				materialLibrary->UpdateMaterial(
					component.materialLibraryKey,
					(MaterialType)i,
					textureLibraryKeyTemp
				);
			}

			ImGui::SameLine();
			if (ImGui::Button("white"))
				m_Scene->SetTextureWhite(component.materialLibraryKey, (MaterialType)i);

			ImGui::SameLine();
			if (ImGui::Button("black"))
				m_Scene->SetTextureBlack(component.materialLibraryKey, (MaterialType)i);

			ImGui::EndColumns();

			if (textureLibraryKey.key != std::string())
			{
				uint64_t keyHash = textureLibraryKey.keyHash;
				Ref<Texture2D> texture = Texture::GetTextureLibrary()->Get(textureLibraryKey.key, keyHash, textureLibraryKey.flip);
				float aspect = (float)texture->GetWidth() / (float)texture->GetHeight();
				if (ImGui::ImageButton(materialTypeItems[i], (void*)(intptr_t)texture->GetRendererID(), ImVec2(100.0f * aspect, 100.0f)))
				{
					materialLibrary->RemoveMaterial(component.materialLibraryKey, (MaterialType)i);
				}
			}
			else
			{
				if (ImGui::Button(materialTypeItems[i]))
				{
					materialLibrary->RemoveMaterial(component.materialLibraryKey, (MaterialType)i);
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
						TextureLibraryKey textureLibraryKey = { filePath.string(), 0, true, TextureInternalFormat::NONE };
						materialLibrary->UpdateMaterial(
							component.materialLibraryKey,
							(MaterialType)i,
							textureLibraryKey
						);
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::PopID();
		}

		ImGui::Separator();

		ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
		ImGui::DragFloat("Parallax Scale", &component.parallaxScale, 0.01f, 0.0f, 100.0f);
		ImGui::DragFloat("Tiling Factor", &component.tilingFactor, 0.1f, 0.0f, 100.0f);
		ImGui::DragFloat("OffsetX", &component.uvRect.x, 0.00025f, 0.0f, 1.0f);
		ImGui::DragFloat("OffsetY", &component.uvRect.y, 0.00025f, 0.0f, 1.0f);
		ImGui::DragFloat("ScaleX", &component.uvRect.z, 0.00025f, 0.0f, 1.0f);
		ImGui::DragFloat("ScaleY", &component.uvRect.w, 0.00025f, 0.0f, 1.0f);

		ImGui::DragFloat("Thickness", &component.thickness, 0.025f, 0.0f, 1.0f);
		ImGui::DragFloat("Fade", &component.fade, 0.00025f, 0.0f, 1.0f);

		// 显示的文本，flags 变量的指针，要控制的那个位（例如 1 << 0）
		ImGui::CheckboxFlags("Outline", &component.flags, (uint32_t)MeshRendererComponent::MeshRendererFlags::Outline);
		
		if (component.HasFlag(component.flags, MeshRendererComponent::MeshRendererFlags::Outline))
		{
			ImGui::ColorEdit4("Outline Color", glm::value_ptr(component.outlineColor));
			DrawVec3Control("Outline Scale", component.outlineScale, 1.0f);
		}

		ImGui::CheckboxFlags("Explosion", &component.flags, (uint32_t)MeshRendererComponent::MeshRendererFlags::Explosion);
		if (component.HasFlag(component.flags, MeshRendererComponent::MeshRendererFlags::Explosion))
		{
			ImGui::DragFloat("Explosion Offset", &component.explosionOffset, 0.025f, 0.0f, 100.0f);
		}

		ImGui::CheckboxFlags("Normal Visualization", &component.flags, (uint32_t)MeshRendererComponent::MeshRendererFlags::NormalVisualization);
		if (component.HasFlag(component.flags, MeshRendererComponent::MeshRendererFlags::NormalVisualization))
		{
			ImGui::ColorEdit4("Normal Visualization Color", glm::value_ptr(component.normalVisualizationColor));
			ImGui::DragFloat("Normal Visualization Length", &component.normalVisualizationLength, 0.025f, 0.0f, 100.0f);
			ImGui::DragInt("Normal Visualization Index1", &component.normalVisualizationIndex1, 1, -1);
			ImGui::DragInt("Normal Visualization Index2", &component.normalVisualizationIndex2, 1, -1);
		}

	}
}