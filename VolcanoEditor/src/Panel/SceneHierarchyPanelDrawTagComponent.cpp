#include "SceneHierarchyPanel.h"

#include "Volcano/Scripting/ScriptEngine.h"

namespace Volcano
{
	void SceneHierarchyPanel::DrawTagComponent(Ref<Entity> entity)
	{

		if (entity->HasComponent<TagComponent>())
		{
			if (ImGui::Checkbox("##EntityActive", &entity->GetActive()))
			{
				if (entity->HasComponent<ScriptComponent>())
				{
					auto& scriptComponent = entity->GetComponent<ScriptComponent>();
					if (!scriptComponent.ClassName.empty() && scriptComponent.enabled)
					{
						auto scriptInstance = ScriptEngine::GetEntityScriptInstance(entity->GetUUID());
						if (scriptInstance != nullptr)
						{
							scriptInstance->SetEnabled(entity->GetActive() && scriptComponent.enabled);

							if (entity->GetActive() == true && m_Scene->IsRunning())
							{
								ScriptEngine::EntityStart(entity->GetUUID());
							}
						}
					}
				}
			}

			ImGui::SameLine();

			auto& tag = entity->GetComponent<TagComponent>().tag;
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));

			ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

			// 重命名
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer), flags))
			{
				Entity* entityParent = entity->GetEntityParent();
				if (entityParent == nullptr)
					tag = Scene::NewName(entity->GetScene()->GetEntityList(), buffer);
				else
					tag = Scene::NewName(entityParent->GetEntityChildrenList(), buffer);
			}
		}
	}
}