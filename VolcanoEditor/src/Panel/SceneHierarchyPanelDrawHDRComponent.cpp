#include "SceneHierarchyPanel.h"

#include "glm/gtc/type_ptr.hpp"

namespace Volcano
{
	void SceneHierarchyPanel::DrawHDRComponent(HDRComponent& component)
	{
		ImGui::Checkbox("Enabled", &component.enabled);

		ImGui::Checkbox("Primary", &component.primary);

		ImGui::Separator();

		auto& equirectangularMapKey = component.equirectangularMapKey;

		if (equirectangularMapKey.key != std::string())
		{
			uint64_t keyHash = equirectangularMapKey.keyHash;
			Ref<Texture2D> texture = Texture::GetTextureLibrary()->Get(equirectangularMapKey.key, keyHash, equirectangularMapKey.flip, equirectangularMapKey.internalFormat);
			float aspect = (float)texture->GetWidth() / (float)texture->GetHeight();
			if (ImGui::ImageButton("EquirectangularMap", (void*)(intptr_t)texture->GetRendererID(), ImVec2(100.0f * aspect, 100.0f)))
			{
				equirectangularMapKey = TextureLibraryKey();
			}
		}
		else
		{
			if (ImGui::Button("EquirectangularMap"))
			{
				equirectangularMapKey = TextureLibraryKey();
			}
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path filePath = path;

				if (filePath.extension() == ".hdr")
				{
					equirectangularMapKey = { filePath.string(), 0, true, TextureInternalFormat::RGB16F };
					component.UpdateEnvCubeMap();
				}
			}
			ImGui::EndDragDropTarget();
		}

	}
}