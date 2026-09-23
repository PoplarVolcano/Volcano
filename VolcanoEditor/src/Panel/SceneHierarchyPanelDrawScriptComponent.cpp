#include "SceneHierarchyPanel.h"

#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/ImGui/UI.h"

namespace Volcano
{

	/*
	获取monoBehaviour脚本实例

	enabled选框，每次点击时判断，脚本实例存在则设置enabled
	注：脚本实例存在与否有两种情况
	还没有选中脚本类，所以没有脚本实例，激活与否没有意义
	选中脚本类或读取脚本类同时实例化脚本，脚本实例存在

	脚本类名下拉框

	*/
	void SceneHierarchyPanel::DrawScriptComponent(Ref<Entity> entity, ScriptComponent& component)
	{
		uint64_t entityID = entity->GetUUID();
		auto scene = entity->GetScene();

		Ref<ScriptInstanceMonoBehaviour> monoBehaviour = ScriptEngine::GetEntityScriptInstance(entity->GetUUID());

		if (ImGui::Checkbox("##ScriptEnabled", &component.enabled) && !component.ClassName.empty())
		{
			if (monoBehaviour != nullptr)
				monoBehaviour->SetEnabled(component.enabled);
		}

		ImGui::SameLine();

		//bool isScriptClassLoaded = ScriptEngine::IsScriptClassLoaded(component.ClassName, false);
		// 如果mono类不存在则红框
		//UI::ScopedStyleColor textColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f), !isScriptClassLoaded && !component.ClassName.empty());

		ImGui::SameLine();

		// 脚本类名下拉框
		if (ImGui::BeginCombo("##ScriptClassName", component.ClassName.c_str()))
		{
			for (auto& [className, scriptClass] : ScriptEngine::GetAppScriptClassMap())
			{
				const bool isSelected = (className == component.ClassName);
				if (ImGui::Selectable(className.c_str(), isSelected))
				{
					if (!isSelected)
					{
						// 选中下拉条目且不是当前脚本类名，则修改当前脚本类名，创建脚本实例
						component.ClassName = className;
						ScriptEngine::CreateMonoBehaviourScriptInstanceByEntity(entity, false);
						monoBehaviour = ScriptEngine::GetEntityScriptInstance(entity->GetUUID());
					}
				}

				// 当前脚本类名对应的下拉条目设置为集中颜色
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}


		// 渲染字段
		bool sceneRunning = scene->IsRunning();

		Ref<ScriptClass> scriptClass = ScriptEngine::GetScriptClass(component.ClassName, false);//monoBehaviour->GetScriptClass();
		if (scriptClass == nullptr)
			return;
		const auto& fields = scriptClass->GetFields();
		auto& entityScriptFieldMap = ScriptEngine::GetEntityScriptFieldMap();

		// 运行时将修改的字段直接返回给C#实例
		// 非运行时将修改的字段返回给实体的字段映射表
		for (const auto& [fieldNameHash, scriptField] : fields)
		{
			ImGui::PushID(&scriptField.name);

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 150.0f);
			ImGui::Text(scriptField.name.c_str());
			ImGui::NextColumn();

			if (scriptField.type == ScriptFieldType::ULong)
			{
				if (sceneRunning)
				{
					uint64_t data = monoBehaviour->GetFieldValue<uint64_t>(fieldNameHash);
					if (ImGui::InputText("##ScriptFieldName", (char*)&data, sizeof(data), ImGuiInputTextFlags_EnterReturnsTrue))
					{
						monoBehaviour->SetFieldValue(fieldNameHash, data);
					}
				}
				else
				{
					ScriptFieldInstance* scriptFieldInstance = entityScriptFieldMap.TryGetField(entityID, scriptClass->GetFullNameHash(), fieldNameHash);
					if (scriptFieldInstance != nullptr)
					{
						uint64_t data = scriptFieldInstance->GetValue<uint64_t>();
						if (ImGui::InputText("##ScriptFieldName", (char*)&data, sizeof(data), ImGuiInputTextFlags_EnterReturnsTrue))
						{
							scriptFieldInstance->SetValue(data);
						}
					}
				}
			}

			if (scriptField.type == ScriptFieldType::Float)
			{
				if (sceneRunning)
				{
					float data = monoBehaviour->GetFieldValue<float>(fieldNameHash);
					if (ImGui::DragFloat("##ScriptFieldName", &data, 0.1f))
					{
						monoBehaviour->SetFieldValue(fieldNameHash, data);
					}
				}
				else
				{
					ScriptFieldInstance* scriptFieldInstance = entityScriptFieldMap.TryGetField(entityID, scriptClass->GetFullNameHash(), fieldNameHash);
					if (scriptFieldInstance != nullptr)
					{
						float data = scriptFieldInstance->GetValue<float>();
						if (ImGui::DragFloat("##ScriptFieldName", &data, 0.1f))
						{
							scriptFieldInstance->SetValue(data);
						}
					}
				}
			}

			if (scriptField.type == ScriptFieldType::Vector3)
			{
				if (sceneRunning)
				{
					glm::vec3 data = monoBehaviour->GetFieldValue<glm::vec3>(fieldNameHash);

					ImGui::DragFloat("##X", &data.x, 0.1f, 0.0f, 0.0f, "%.2f");
					{
						monoBehaviour->SetFieldValue(fieldNameHash, data);
					}

					ImGui::DragFloat("##Y", &data.y, 0.1f, 0.0f, 0.0f, "%.2f");
					{
						monoBehaviour->SetFieldValue(fieldNameHash, data);
					}

					ImGui::DragFloat("##Z", &data.z, 0.1f, 0.0f, 0.0f, "%.2f");
					{
						monoBehaviour->SetFieldValue(fieldNameHash, data);
					}

				}
				else
				{
					ScriptFieldInstance* scriptFieldInstance = entityScriptFieldMap.TryGetField(entityID, scriptClass->GetFullNameHash(), fieldNameHash);
					if (scriptFieldInstance != nullptr)
					{
						glm::vec3 data = scriptFieldInstance->GetValue<glm::vec3>();
						ImGui::DragFloat("##X", &data.x, 0.1f, 0.0f, 0.0f, "%.2f");
						{
							scriptFieldInstance->SetValue<glm::vec3>(data);
						}
						ImGui::DragFloat("##Y", &data.y, 0.1f, 0.0f, 0.0f, "%.2f");
						{
							scriptFieldInstance->SetValue<glm::vec3>(data);
						}
						ImGui::DragFloat("##Z", &data.z, 0.1f, 0.0f, 0.0f, "%.2f");
						{
							scriptFieldInstance->SetValue<glm::vec3>(data);
						}
					}
				}
			}

			if (scriptField.type == ScriptFieldType::Object ||
				scriptField.type == ScriptFieldType::GameObject ||
				scriptField.type == ScriptFieldType::Behaviour ||
				scriptField.type == ScriptFieldType::Component ||
				scriptField.type == ScriptFieldType::Transform ||
				scriptField.type == ScriptFieldType::MonoBehaviour)
			{
				auto& entityIDMap = scene->GetEntityIDMap();
				if (sceneRunning)
				{
					// 获取字段，核心类字段统一为uint64_t entityID
					uint64_t targetEntityID = monoBehaviour->GetFieldValue<uint64_t>(fieldNameHash);

					Ref<Entity> targetEntity;
					if (targetEntityID != 0 && entityIDMap.find(targetEntityID) != entityIDMap.end())
					{
						targetEntity = entityIDMap.at(targetEntityID);
					}

					// entity下拉框
					if (ImGui::BeginCombo("##ScriptFieldName", targetEntity == nullptr ? "" : targetEntity->GetName().c_str()))
					{
						for (auto& [entityID, entityNode] : entityIDMap)
						{
							const bool isSelected = (entityID == targetEntityID);
							if (ImGui::Selectable(entityNode->GetName().c_str(), isSelected))
							{
								if (!isSelected)
								{
									monoBehaviour->SetFieldValue(fieldNameHash, entityID);
								}
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_NODE"))
						{
							const UUID* entityIDPtr = (const UUID*)payload->Data;
							monoBehaviour->SetFieldValue(fieldNameHash, *entityIDPtr);
						}
						ImGui::EndDragDropTarget();
					}
				}
				else
				{
					ScriptFieldInstance* scriptFieldInstance = entityScriptFieldMap.TryGetField(entityID, scriptClass->GetFullNameHash(), fieldNameHash);
					if (scriptFieldInstance != nullptr)
					{
						uint64_t targetEntityID = scriptFieldInstance->GetValue<uint64_t>();
						Ref<Entity> targetEntity;
						if (entityIDMap.find(targetEntityID) != entityIDMap.end())
							targetEntity = entityIDMap[targetEntityID];

						if (ImGui::BeginCombo("##ScriptFieldName", targetEntity == nullptr ? "" : targetEntity->GetName().c_str()))
						{
							for (auto& [ID, entityNode] : entityIDMap)
							{
								const bool isSelected = (entityNode->GetUUID() == (targetEntity == nullptr ? UUID() : targetEntity->GetUUID()));
								if (ImGui::Selectable(entityNode->GetName().c_str(), isSelected))
								{
									if (!isSelected)
										scriptFieldInstance->SetValue((uint64_t)entityNode->GetUUID());
								}
								if (isSelected)
									ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
						}
						if (ImGui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_NODE"))
							{
								const UUID* entityID = (const UUID*)payload->Data;
								scriptFieldInstance->SetValue(*entityID);
							}
							ImGui::EndDragDropTarget();
						}
					}
				}
			}
			ImGui::SameLine();
			ImGui::Text(("(" + std::string(Utils::ScriptFieldTypeToString(scriptField.type)) + ")").c_str());
			ImGui::Columns(1);

			ImGui::PopID();
		}
	}
}