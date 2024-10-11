#include "SceneHierarchyPanel.h"
#include "Volcano/Scene/Components.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/UI/UI.h"

#include "Volcano/Renderer/RendererItem/ModelTemp.h"
#include "Volcano/Math/Math.h"

#include "imgui/imgui.h"
#include <imgui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

namespace Volcano {

	SceneHierarchyPanel::SceneHierarchyPanel(Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
		if (m_SelectionContext)
		{
			UUID SelectedEntityID = m_SelectionContext->GetUUID();
			Ref<Entity> target = m_Context->GetEntityByUUID(SelectedEntityID);
			if(target)
				m_SelectionContext = target;
			else
				m_SelectionContext = {};
		}
		else
			m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		if (m_Context)
		{
			// BUG记录：窗口菜单放在节点菜单后面会覆盖判定，右键只会出现窗口菜单
			// Right-click on blank space
			if (ImGui::BeginPopupContextWindow(0, 1))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
					m_Context->CreateEntity("Empty Entity");
				ImGui::EndPopup();
			}

			auto entityNameMap = m_Context->GetEntityNameMap();
			for (auto itr = entityNameMap.begin(); itr != entityNameMap.end(); itr++)
			{
				DrawEntityNode(itr->second);
			}

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				m_SelectionContext = {};
		}

		ImGui::End();
		
		ImGui::Begin("Prorerties");
		if (m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::SetSelectedEntity(Ref<Entity> entity)
	{
		m_SelectionContext = entity;
	}

	void SceneHierarchyPanel::DrawEntityNode(Ref<Entity> entity)
	{
		auto& tag = entity->GetComponent<TagComponent>().Tag;
		// 若是被点击标记为选中状态|有下一级
		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		// 第一个参数是唯一ID 64的，
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity.get(), flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		bool entityDeleted = false;
		//if (ImGui::BeginPopupContextWindow(0, 1))
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Create Empty Entity"))
			{
				m_Context->CreateEntity("Empty Entity", entity);
			}
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;
			ImGui::EndPopup();
		}

		if (opened)
		{
			auto entityChildren = entity->GetEntityChildren();
			for (auto& [name, entityChild] : entityChildren)
			{
				DrawEntityNode(entityChild);
				//ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
				//bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity.m_Children[i], flags, tag.c_str());
				//if (opened)
				//	ImGui::TreePop();
			}
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
			if(entity->GetEntityParent())
			    m_Context->DestroyEntityChild(entity);
			else
				m_Context->DestroyEntity(entity);
		}
	}

	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		//计算行高
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();

	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Ref<Entity> entity, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = 
			ImGuiTreeNodeFlags_DefaultOpen 
			| ImGuiTreeNodeFlags_Framed 
			| ImGuiTreeNodeFlags_SpanAvailWidth 
			| ImGuiTreeNodeFlags_AllowItemOverlap 
			| ImGuiTreeNodeFlags_FramePadding;
		if (entity->HasComponent<T>())
		{
			auto& component = entity->GetComponent<T>();
			// 为了定位+按钮在最右边
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			// 先绘制展开箭头
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();

			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);

			if (ImGui::Button("...", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;
				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}
			if (removeComponent)
				entity->RemoveComponent<T>();
		}

	}

	void ModelLoading(Ref<MeshData> mesh, Ref<Entity> entity)
	{
		auto entityNew = entity->GetScene()->CreateEntity(mesh->name, entity);
		auto transform = entityNew->GetComponent<TransformComponent>();
		Math::DecomposeTransform(mesh->transform, transform.Translation, transform.Rotation, transform.Scale);
		if (mesh->mesh != nullptr)
		{
			auto& mc = entityNew->AddComponent<MeshComponent>();
			mc.SetMesh(MeshType::Model, entityNew.get(), std::make_shared<MeshTemp>(*mesh->mesh.get()));
		}
		if (!mesh->textures.empty())
		{
			auto& mrc = entityNew->AddComponent<MeshRendererComponent>();
			for (auto& [type, texture] : mesh->textures)
				mrc.AddTexture(type, texture);
		}
		if (!mesh->children.empty())
		{
			for(auto& meshChild : mesh->children)
			ModelLoading(meshChild, entityNew);
		}
	}

	void SceneHierarchyPanel::DrawComponents(Ref<Entity> entity)
	{
		if (entity->HasComponent<TagComponent>())
		{
			auto& tag = entity->GetComponent<TagComponent>().Tag;
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));

			ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

			if (ImGui::InputText("##Tag", buffer, sizeof(buffer), flags))
			{
				Entity* entityParent = entity->GetEntityParent();
				if (entityParent)
				{
					auto& entityMap = entityParent->GetEntityChildren();
					Ref<Entity> entityTemp = entityMap[tag];
					entityMap.erase(tag);
					std::string newName = Scene::NewName(entityMap, buffer);
					entityMap[newName] = entityTemp;
					tag = std::string(newName);

				}
				else
				{
					auto& entityMap = m_Context->GetEntityNameMap();
					Ref<Entity> entityTemp = entityMap[tag];
					entityMap.erase(tag);
					std::string newName = Scene::NewName(entityMap, buffer);
					entityMap[newName] = entityTemp;
					tag = std::string(newName);
				}
			}
		}


		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
			{
				DrawVec3Control("Translation", component.Translation);

				glm::vec3 rotation = glm::degrees(component.Rotation);
				DrawVec3Control("Rotation", rotation);
				component.Rotation = glm::radians(rotation);
				//ImGui::DragFloat3("Position", glm::value_ptr(tc.Translation), 0.1f);

				DrawVec3Control("Scale", component.Scale, 1.0f);
			});

		DrawComponent<LightComponent>("Light", entity, [](auto& component)
			{
				const char* lightTypeStrings[] = { "DirectionalLight", "PointLight", "SpotLight"};
				const char* currentLightTypeString = lightTypeStrings[(int)component.Type];
				if (ImGui::BeginCombo("LightType", currentLightTypeString))
				{
					for (int i = 0; i < 3; i++)
					{
						bool isSelected = currentLightTypeString == lightTypeStrings[i];
						if (ImGui::Selectable(lightTypeStrings[i], isSelected))
						{
							currentLightTypeString = lightTypeStrings[i];
							component.Type = (LightComponent::LightType)i;
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (component.Type == LightComponent::LightType::DirectionalLight)
				{
					DrawVec3Control("Ambient",  component.Ambient, 0.1f);
					DrawVec3Control("Diffuse",  component.Diffuse, 1.0f);
					DrawVec3Control("Specular", component.Specular, 1.0f);
				}

				if (component.Type == LightComponent::LightType::PointLight)
				{
					DrawVec3Control("Ambient",    component.Ambient,   0.1f);
					DrawVec3Control("Diffuse",    component.Diffuse,   1.0f);
					DrawVec3Control("Specular",   component.Specular,  1.0f);
					ImGui::DragFloat("Constant",  &component.Constant,  0.1f, 0.0f, 0.0f, "%.3f");
					ImGui::DragFloat("Linear",    &component.Linear,    0.1f, 0.0f, 0.0f, "%.3f");
					ImGui::DragFloat("Quadratic", &component.Quadratic, 0.1f, 0.0f, 0.0f, "%.3f");
				}

				if (component.Type == LightComponent::LightType::SpotLight)
				{
					DrawVec3Control("Ambient",    component.Ambient, 0.1f);
					DrawVec3Control("Diffuse",    component.Diffuse, 1.0f);
					DrawVec3Control("Specular",   component.Specular, 1.0f);
					ImGui::DragFloat("Constant",  &component.Constant, 0.1f, 0.0f, 0.0f, "%.3f");
					ImGui::DragFloat("Linear",    &component.Linear, 0.1f, 0.0f, 0.0f, "%.3f");
					ImGui::DragFloat("Quadratic", &component.Quadratic, 0.1f, 0.0f, 0.0f, "%.3f");
					float cutOff = glm::degrees(glm::acos(component.CutOff));
					float outerCutOff = glm::degrees(glm::acos(component.OuterCutOff));
					ImGui::DragFloat("CutOff", &cutOff, 0.1f, 0.0f, 0.0f, "%.3f");
					ImGui::DragFloat("OuterCutOff", &outerCutOff, 0.1f, 0.0f, 0.0f, "%.3f");
					component.CutOff = glm::cos(glm::radians(cutOff));
					component.OuterCutOff = glm::cos(glm::radians(outerCutOff));
				}

			});

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
			{
				auto& camera = component.Camera;

				ImGui::Checkbox("Primary", &component.Primary);

				const char* projectionTypeStrings[] = { "Prespective", "Orthographic" };
				const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
				if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
				{
					for (int i = 0; i < 2; i++)
					{
						bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
						if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
						{
							currentProjectionTypeString = projectionTypeStrings[i];
							camera.SetProjectionType((SceneCamera::ProjectionType)i);
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Prespective)
				{
					float verticalFOV = glm::degrees(camera.GetPerspectiveVerticalFOV());
					if (ImGui::DragFloat("Vertical FOV", &verticalFOV))
						camera.SetPerspectiveVerticalFOV(glm::radians(verticalFOV));

					float perspNear = camera.GetPerspectiveNearClip();
					if (ImGui::DragFloat("Near", &perspNear))
						camera.SetPerspectiveNearClip(perspNear);

					float perspFar = camera.GetPerspectiveFarClip();
					if (ImGui::DragFloat("Far", &perspFar))
						camera.SetPerspectiveFarClip(perspFar);
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float orthoSize = camera.GetOrthographicSize();
					if (ImGui::DragFloat("Size", &orthoSize))
						camera.SetOrthographicSize(orthoSize);

					float orthoNear = camera.GetOrthographicNearClip();
					if (ImGui::DragFloat("Near", &orthoNear))
						camera.SetOrthographicNearClip(orthoNear);

					float orthoFar = camera.GetOrthographicFarClip();
					if (ImGui::DragFloat("Far", &orthoFar))
						camera.SetOrthographicFarClip(orthoFar);

					ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
				}
			});

		DrawComponent<ScriptComponent>("Script", entity, [entity, scene = m_Context](auto& component) mutable
			{
				bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);

				static char buffer[64];
				// 把组件的ClassName写入buffer
				strcpy_s(buffer, sizeof(buffer), component.ClassName.c_str());

				// 如果mono类不存在则红框
				UI::ScopedStyleColor textColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f), !scriptClassExists);

				// 把buffer写入组件的ClassName
				if (ImGui::InputText("Class", buffer, sizeof(buffer)))
				{
					component.ClassName = buffer;
					return;
				}

				// Fields
				bool sceneRunning = scene->IsRunning();
				if (sceneRunning)
				{
					// 运行状态下将mono类获取的字段实时返回editor
					Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity->GetUUID());
					if (scriptInstance)
					{
						const auto& fields = scriptInstance->GetScriptClass()->GetFields();
						for (const auto& [name, field] : fields)
						{
							if (field.Type == ScriptFieldType::Float)
							{
								float data = scriptInstance->GetFieldValue<float>(name);
								if (ImGui::DragFloat(name.c_str(), &data, 0.1f))
								{
									scriptInstance->SetFieldValue(name, data);
								}
							}
						}
					}
				}
				else
				{
					// Editor模式下，从mono类中获取字段数据做拖动条，将拖动条数据注入EntityScriptFields
					// 注：mono类实例化会从EntityScriptFields读取字段数据并注入mono类实例，
					//   即读取的字段数据会应用到所有的相同entity子类脚本实例的字段
					// TODO：将entity的mono类脚本实例与EntityScriptFields对应上
					if (scriptClassExists)
					{
						Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(component.ClassName);
						// 获取mono类中的字段field
						const auto& fields = entityClass->GetFields();

						// 获取字段field的map
						auto& entityFields = ScriptEngine::GetScriptFieldMap(*entity.get());
						for (const auto& [name, field] : fields)
						{
							// Field has been set in editor
							if (entityFields.find(name) != entityFields.end())
							{
								ScriptFieldInstance& scriptField = entityFields.at(name);

								// Display control to set it maybe
								if (field.Type == ScriptFieldType::Float)
								{
									float data = scriptField.GetValue<float>();
									if (ImGui::DragFloat(name.c_str(), &data, 0.1f))
									{
										scriptField.SetValue(data);
									}
								}
							}
							else
							{
								if (field.Type == ScriptFieldType::Float)
								{
									float data = 0.0f;
									if (ImGui::DragFloat(name.c_str(), &data, 0.1f))
									{
										ScriptFieldInstance& fieldInstance = entityFields[name];
										fieldInstance.Field = field;
										fieldInstance.SetValue(data);
									}
								}
							}
						}
					}
				}
			});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
			{
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));

				ImGui::Button("Texture", ImVec2(100.0f, 100.0f));
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path texturePath(path);
						component.Texture = Texture2D::Create(texturePath.string());
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
			});

		DrawComponent<MeshComponent>("Mesh", entity, [entity](MeshComponent& component)
			{
				const char* items[] = { "None", "Quad", "Circle", "Line", "Cube", "Sphere", "Model"};
				int meshType = (int)component.meshType;
				if (ImGui::Combo("MeshType", &meshType, items, IM_ARRAYSIZE(items)))
				{
					component.SetMesh((MeshType)meshType, entity.get());
				}
			});

		DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [](MeshRendererComponent& component)
			{
				const char* items[] = { "Diffuse", "Specular", "Normal", "Height" };
				auto& textures = component.Textures;

				for (uint32_t i = 0; i < textures.size(); i++)
				{
					ImGui::PushID(i);
					int imageType = (int)textures[i].first;
					if (ImGui::Combo("ImageType", &imageType, items, IM_ARRAYSIZE(items)))
					{
						component.Textures[i].first = (ImageType)imageType;
					}

					//ImGui::SameLine();

					auto& texture = textures[i].second;
					if (texture != nullptr)
					{
						float aspect = texture->GetWidth() / texture->GetHeight();
						if (ImGui::ImageButton((void*)(intptr_t)texture->GetRendererID(), ImVec2(100.0f * aspect, 100.0f)))
							texture = nullptr;
					}
					else
					{
						if (ImGui::Button(items[(int)textures[i].first]))
							texture = nullptr;
					}
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
						{
							const wchar_t* path = (const wchar_t*)payload->Data;
							std::filesystem::path filePath = path;
							texture = Texture2D::Create(filePath.string());
						}
						ImGui::EndDragDropTarget();
					}
					ImGui::PopID();
				}

				if(ImGui::Button("+"))
					component.AddTexture();

				ImGui::SameLine();

				if (ImGui::Button("-"))
					if (textures.size() > 0)
						component.DeleteTexture(textures.size() - 1);
			});

		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto& component)
			{
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));

				if (ImGui::Button("Texture", ImVec2(100.0f, 100.0f)))
					component.Texture = nullptr;
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path texturePath = path;
						component.Texture = Texture2D::Create(texturePath.string());
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
				ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
			});

		DrawComponent<SphereRendererComponent>("Sphere Renderer", entity, [](auto& component)
			{
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));

				if (ImGui::Button("Albedo", ImVec2(100.0f, 100.0f)))
					component.Albedo = nullptr;
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path albedoPath = path;
						component.Albedo = Texture2D::Create(albedoPath.string());
					}
					ImGui::EndDragDropTarget();
				}

				if (ImGui::Button("Normal", ImVec2(100.0f, 100.0f)))
					component.Normal = nullptr;
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path normalPath = path;
						component.Normal = Texture2D::Create(normalPath.string());
					}
					ImGui::EndDragDropTarget();
				}

				if (ImGui::Button("Metallic", ImVec2(100.0f, 100.0f)))
					component.Metallic = nullptr;
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path metallicPath = path;
						component.Metallic = Texture2D::Create(metallicPath.string());
					}
					ImGui::EndDragDropTarget();
				}

				if (ImGui::Button("Roughness", ImVec2(100.0f, 100.0f)))
					component.Roughness = nullptr;
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path roughnesslPath = path;
						component.Roughness = Texture2D::Create(roughnesslPath.string());
					}
					ImGui::EndDragDropTarget();
				}

				if (ImGui::Button("AO", ImVec2(100.0f, 100.0f)))
					component.AO = nullptr;
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path AOPath = path;
						component.AO = Texture2D::Create(AOPath.string());
					}
					ImGui::EndDragDropTarget();
				}
			});

		DrawComponent<ModelRendererComponent>("Model Renderer", entity, [](auto& component)
			{
				if (ImGui::Button("ModelPath", ImVec2(100.0f, 100.0f)))
					component.ModelPath = nullptr;
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path modelPath = path;
						component.ModelPath = modelPath.string();
					}
					ImGui::EndDragDropTarget();
				}
			});

		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component)
			{
				const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
				const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];
				if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
				{
					for (int i = 0; i < 2; i++)
					{
						bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
						if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
						{
							currentBodyTypeString = bodyTypeStrings[i];
							component.Type = (Rigidbody2DComponent::BodyType)i;
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
			});

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
			{
				ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
				ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
				ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
			});

		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component)
			{
				ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
				ImGui::DragFloat("Radius", &component.Radius, 0.01f);
				ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
			});


		ImGui::PushItemWidth(-1);
		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			DisplayAddComponentEntry<LightComponent>("Light");
			DisplayAddComponentEntry<CameraComponent>("Camera");
			DisplayAddComponentEntry<ScriptComponent>("Script");
			DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
			DisplayAddComponentEntry<MeshComponent>("Mesh");
			DisplayAddComponentEntry<MeshRendererComponent>("Mesh Renderer");
			DisplayAddComponentEntry<CircleRendererComponent>("Circle Renderer");
			DisplayAddComponentEntry<SphereRendererComponent>("Sphere Renderer");
			DisplayAddComponentEntry<ModelRendererComponent>("Model Renderer");
			DisplayAddComponentEntry<Rigidbody2DComponent>("Rigidbody 2D");
			DisplayAddComponentEntry<BoxCollider2DComponent>("Box Collider 2D");
			DisplayAddComponentEntry<CircleCollider2DComponent>("Circle Collider 2D");

			ImGui::EndPopup();
		}
		ImGui::PopItemWidth();


		ImGui::Button("LoadModel");
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path modelPath = path;
				if(!ModelTemp::GetModelLibrary()->Exists(modelPath.string()))
				    ModelTemp::GetModelLibrary()->Load(modelPath.string());
				auto model = ModelTemp::GetModelLibrary()->Get(modelPath.string());
				ModelLoading(model->GetMeshRoot(), entity);
			}
			ImGui::EndDragDropTarget();
		}
	}

	template<typename T>
	inline void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName)
	{
		if (!m_SelectionContext->HasComponent<T>())
		{
			if (ImGui::MenuItem(entryName.c_str()))
			{
				m_SelectionContext->AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}
}