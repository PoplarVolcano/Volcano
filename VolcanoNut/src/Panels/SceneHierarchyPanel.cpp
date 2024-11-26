#include "SceneHierarchyPanel.h"
#include "Volcano/Scene/Components.h"
#include "Volcano/Scene/SceneSerializer.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/UI/UI.h"
#include "Volcano/Utils/PlatformUtils.h"
#include "Volcano/Project/Project.h"
#include "Volcano/Scene/Prefab.h"

#include "Volcano/Renderer/RendererItem/Model.h"
#include "Volcano/Math/Math.h"

#include "imgui/imgui.h"
#include <imgui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

namespace Volcano {

	SceneHierarchyPanel::SceneHierarchyPanel(Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(Ref<Scene>& context)
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

			auto entityList = m_Context->GetEntityList();
			for (auto it = entityList.begin(); it != entityList.end(); it++)
			{
				DrawEntityNode(*it);
			}

			// 若是被点击标记为选中状态|有下一级
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			ImGui::TreeNodeEx((void*)(-1), flags, "");

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_NODE"))
				{
					const UUID* entityID = (const UUID*)payload->Data;
					auto& entityIDMap = m_Context->GetEntityIDMap();
					VOL_CORE_ASSERT(entityIDMap.find(*entityID) != entityIDMap.end());
					Ref<Entity> srcEntity = entityIDMap.at(*entityID);
					Scene::UpdateEntityParent(srcEntity, nullptr);
					/*
					Entity* entityParent = srcEntity->GetEntityParent();
					if (entityParent != nullptr)
					{
						entityParent->GetEntityChildren().erase(std::find(entityParent->GetEntityChildren().begin(), entityParent->GetEntityChildren().end(), srcEntity));

						std::string newName = Scene::NewName(m_Context->GetEntityList(), srcEntity->GetName());
						srcEntity->SetName(newName);
						srcEntity->SetEntityParent(nullptr);
						m_Context->GetEntityList().push_back(srcEntity);
					}
					*/
				}

				// 在Scene目录下新建Prefan实体
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path prefabPath(path);

					if (prefabPath.extension() == ".prefab" && m_Context->GetName() != "Prefab")
					{
						auto& prefabScene = Prefab::GetScene();
						std::string prefabPathTemp = Project::GetRelativeAssetDirectory(prefabPath).string();
						Ref<Entity> targetEntity = Prefab::Get(prefabPathTemp);
						if (targetEntity == nullptr)
						{
							targetEntity = Prefab::Load(prefabPath);
						}
						Ref<Entity> resultEntity = m_Context->DuplicateEntity(targetEntity);
						Prefab::GetEntityPrefabMap()[resultEntity->GetUUID()] = prefabPathTemp;
					}
				}

				ImGui::EndDragDropTarget();
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
		ImGuiTreeNodeFlags flags = (m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = false;

		if (!Prefab::GetPrefabPath(entity).empty())
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.84f, 0.99f, 1.0f));//ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(127.0f / 255.0f, 214.0f / 255.0f, 252.0f / 255.0f, 1.0f);
		
		if (entity->GetEntityChildren().empty())
		{
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity.get(), flags, tag.c_str());
		}
		else
		{
			flags |= ImGuiTreeNodeFlags_OpenOnArrow;
			opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity.get(), flags, tag.c_str());
		}

		if (!Prefab::GetPrefabPath(entity).empty())
			ImGui::PopStyleColor();

		if (ImGui::BeginDragDropSource())
		{
			// 设置数据源
			UUID entityID = entity->GetUUID();
			ImGui::SetDragDropPayload("SCENE_HIERARCHY_NODE", &entityID, sizeof(entityID));
			ImGui::EndDragDropSource();
		}

		// 拖拽实体改变路径
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_NODE"))
			{
				const UUID* entityID = (const UUID*)payload->Data;
				Scene* scene = entity->GetScene();
				auto& entityIDMap = m_Context->GetEntityIDMap();
				VOL_CORE_ASSERT(entityIDMap.find(*entityID) != entityIDMap.end());
				Ref<Entity> srcEntity = entityIDMap.at(*entityID);
				Scene::UpdateEntityParent(srcEntity, entity.get());
				/*
				Scene* scene = entity->GetScene();
				auto& entityIDMap = m_Context->GetEntityIDMap();
				VOL_CORE_ASSERT(entityIDMap.find(*entityID) != entityIDMap.end());
				Ref<Entity> srcEntity = entityIDMap.at(*entityID);
				Entity* entityParent = srcEntity->GetEntityParent();
				if(entityParent != nullptr)
					entityParent->GetEntityChildren().erase(std::find(entityParent->GetEntityChildren().begin(), entityParent->GetEntityChildren().end(), srcEntity));
				else
					scene->GetEntityList().erase(std::find(scene->GetEntityList().begin(), scene->GetEntityList().end(), srcEntity));
				entity->AddEntityChild(srcEntity);
				*/
			}

			// 在结点下创建一个Prefab实体
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path prefabPath(path);

				if (prefabPath.extension() == ".prefab" && m_Context->GetName() != "Prefab")
				{
					auto& prefabScene = Prefab::GetScene();
					std::string prefabPathTemp = Project::GetRelativeAssetDirectory(prefabPath).string();
					auto targetEntity = Prefab::Get(prefabPathTemp);
					if (targetEntity == nullptr)
					{
						targetEntity = Prefab::Load(prefabPath);
					}
					Ref<Entity> resultEntity = m_Context->DuplicateEntity(targetEntity, entity.get());
					Prefab::GetEntityPrefabMap()[resultEntity->GetUUID()] = prefabPathTemp;
				}
			}

			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered())//ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		bool entityDeleted = false;
		//if (ImGui::BeginPopupContextWindow(0, 1))
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Create Empty Entity"))
			{
				m_Context->CreateEntity("Empty Entity", entity.get());
			}
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;
			ImGui::EndPopup();
		}

		if (opened)
		{
			auto entityChildren = entity->GetEntityChildren();
			for (auto& entityChild : entityChildren)
			{
				DrawEntityNode(entityChild);
			}
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			if (m_SelectionContext == entity)
				m_SelectionContext = {};

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

	void ModelLoading(Ref<ModelNode> modelNode, Ref<Model> model, Ref<Entity> entity)
	{
		auto entityNode = entity->GetScene()->CreateEntity(modelNode->name, entity.get());
		auto transform = entityNode->GetComponent<TransformComponent>();
		Math::DecomposeTransform(modelNode->transform, transform.Translation, transform.Rotation, transform.Scale);
		for (uint32_t i = 0; i < modelNode->numMeshes; i++)
		{
		    auto entityMesh = entity->GetScene()->CreateEntity(modelNode->name, entityNode.get());
			auto& mesh = model->GetMeshes()[i];//modelNode->meshes[i]];
			auto& mc = entityMesh->AddComponent<MeshComponent>();
			mc.SetMesh(MeshType::Model, entityMesh.get(), std::make_shared<Mesh>(*mesh->mesh.get()));

			if (!mesh->textures.empty())
			{
				auto& mrc = entityMesh->AddComponent<MeshRendererComponent>();
				for (auto& [type, texture] : mesh->textures)
					mrc.AddTexture(type, texture);
			}
		}

		for(auto& modelNodeChild : modelNode->children)
			ModelLoading(modelNodeChild, model, entityNode);
	}

	void DestroyAssimpNode(AssimpNodeData& node, AnimationComponent& ac)
	{
		for (uint32_t i = 0; i < node.children.size(); i++)
			DestroyAssimpNode(node.children[i], ac);
		
		// 删除所有Children节点以及对应BoneInfo和Bone
		auto& bones = ac.animation->GetBones();
		for (uint32_t i = 0; i < node.children.size(); i++)
		{
			bones.erase(std::find_if(bones.begin(), bones.end(), [&](Bone& bone){ return bone.GetBoneName() == node.children.begin()->name; } ));
			ac.animation->GetBoneIDMap().erase(node.children.begin()->name);
			node.children.erase(node.children.begin());
		}
	}

	void DrawAssimpNode(AssimpNodeData& node, AnimationComponent& ac)
	{
		auto& boneMap = ac.animation->GetBoneIDMap();
		int boneID = boneMap[node.name].id;

		ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		ImGui::Separator();

		bool open = ImGui::TreeNodeEx((void*)boneID, treeNodeFlags, node.name.c_str());

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Create Empty Bone"))
			{
				ac.newBoneNodeParent = &node;
			}
			if (ImGui::MenuItem("Delete Bone"))
				entityDeleted = true;
			ImGui::EndPopup();
		}


		if (open)
		{
			// BoneInfo.id
			ImGui::PushID(boneID);
			if (ImGui::TreeNodeEx((void*)boneID, treeNodeFlags, "Bone Information"))
			{
				char buffer[64];
				memset(buffer, 0, sizeof(buffer));
				strncpy_s(buffer, sizeof(buffer), node.name.c_str(), sizeof(buffer));

				ImGui::Text(("ID: " + std::to_string(boneID)).c_str());

				ImGui::PushItemWidth(200);
				ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
				if (ImGui::InputText("##Name", buffer, sizeof(buffer), flags))
				{
					auto boneInfo = boneMap[node.name];
					boneMap.erase(node.name);
					std::string name = buffer;
					node.name = name;
					for (uint32_t i = 0; boneMap.find(node.name) != boneMap.end(); i++)
					{
						node.name = name + "(" + std::to_string(i) + ")";
					}
					boneMap[node.name] = boneInfo;
					ac.animation->GetBones()[boneInfo.id].SetBoneName(node.name);
				}
				ImGui::PopItemWidth();

				ImGui::SameLine();

				if (ImGui::Button("Reset Transform"))
				{
					boneMap[node.name].offset = glm::mat4(1.0f);
				}

				ImGui::PushID("node.transformation");
				// 设置node.transformation
				if (node.transformation != glm::mat4(0.0f))
				{
					ImGui::Text("Node.transformation");
					glm::vec3 Translation;
					glm::vec3 Rotation;
					glm::vec3 Scale;
					Math::DecomposeTransform(node.transformation, Translation, Rotation, Scale);
					DrawVec3Control("Translation", Translation);

					glm::vec3 rotation = glm::degrees(Rotation);
					DrawVec3Control("Rotation", rotation);
					Rotation = glm::radians(rotation);
					//ImGui::DragFloat3("Position", glm::value_ptr(tc.Translation), 0.1f);
					DrawVec3Control("Scale", Scale, 1.0f);

					node.transformation =
						glm::translate(glm::mat4(1.0f), Translation)
						* glm::toMat4(glm::quat(Rotation))
						* glm::scale(glm::mat4(1.0f), Scale);
				}
				ImGui::PopID();

				ImGui::PushID("BoneInfo.offset");
				if (boneMap[node.name].offset != glm::mat4(0.0f))
				{
					ImGui::Text("BoneInfo.offset");
					glm::vec3 Translation;
					glm::vec3 Rotation;
					glm::vec3 Scale;
					Math::DecomposeTransform(boneMap[node.name].offset, Translation, Rotation, Scale);
					DrawVec3Control("Translation", Translation);

					glm::vec3 rotation = glm::degrees(Rotation);
					DrawVec3Control("Rotation", rotation);
					Rotation = glm::radians(rotation);
					//ImGui::DragFloat3("Position", glm::value_ptr(tc.Translation), 0.1f);
					DrawVec3Control("Scale", Scale, 1.0f);

					boneMap[node.name].offset =
						glm::translate(glm::mat4(1.0f), Translation)
						* glm::toMat4(glm::quat(Rotation))
						* glm::scale(glm::mat4(1.0f), Scale);
				}
				ImGui::PopID();

				ImGui::TreePop();
			}
			ImGui::PopID();

			for (auto& child : node.children)
			{
				DrawAssimpNode(child, ac);
			}
			ImGui::TreePop();
		}


		if (entityDeleted)
		{
			if (node.parent != nullptr)
			{
				// 删除node所有子节点
				DestroyAssimpNode(node, ac);

				// 删除node对应的BoneInfo和Bone
				auto& bones = ac.animation->GetBones();
				bones.erase(std::find_if(bones.begin(), bones.end(), [&](Bone& bone) { return bone.GetBoneName() == node.name; }));
				ac.animation->GetBoneIDMap().erase(node.name);

				// 从父节点的children中删除node
				for (auto itr = node.parent->children.begin(); itr != node.parent->children.end(); itr++)
					if (itr->name == node.name)
					{
						node.parent->children.erase(itr);
						break;
					}

			}
		}

	}

	void SceneHierarchyPanel::DrawPrefabCombo(std::filesystem::path folder_path, Ref<Entity> entity, bool& stopDraw)
	{
		if (std::filesystem::exists(folder_path) && std::filesystem::is_directory(folder_path))
		{
			for (const auto& entry : std::filesystem::directory_iterator(folder_path))
			{
				if (std::filesystem::is_regular_file(entry.status()) && entry.path().extension() == ".prefab") {
					std::string prefabPath = Project::GetRelativeAssetDirectory(entry.path()).string();
					bool isSelected = Prefab::GetPrefabPath(entity) == prefabPath;
					if (ImGui::Selectable(prefabPath.c_str(), isSelected))
					{
						if (!isSelected)
						{
							auto& prefabScene = Prefab::GetScene();

							Ref<Entity> prefabEntity = Prefab::Get(prefabPath);
							if (prefabEntity == nullptr)
								prefabEntity = Prefab::Load(entry.path());

							Ref<Entity> parent;
							if (entity->GetEntityParent() != nullptr)
								parent = m_Context->GetEntityByUUID(entity->GetEntityParent()->GetUUID());

							UUID id = entity->GetUUID();

							m_Context->DestroyEntity(entity);

							// 修改Entity的Prefab目标不需要修改ID
							m_Context->DuplicateEntity(prefabEntity, parent.get(), id);
							Prefab::GetEntityPrefabMap()[id] = prefabPath;

							stopDraw = true;
						}
						//currentLightTypeString = lightTypeStrings[i];
						//component.Type = (LightComponent::LightType)i;
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				if (std::filesystem::is_directory(entry.status()))
				{
					DrawPrefabCombo(entry.path(), entity, stopDraw);
				}
			}
		}
		else {
			VOL_ERROR("The specified path does not exist or is not a directory.");
		}

	}

	void SceneHierarchyPanel::DrawComponents(Ref<Entity> entity)
	{
		static bool stopDraw;
		stopDraw = false;

		if (entity->HasComponent<TagComponent>())
		{
			if (ImGui::Checkbox("##EntityActive", &entity->GetActive()))
			{
				if (m_Context->IsRunning())
				{
					if (entity->GetActive() == true)
					{
						if (entity->HasComponent<ScriptComponent>())
						{
							auto& sc = entity->GetComponent<ScriptComponent>();
							if (!sc.ClassName.empty())
							{
								auto instance = ScriptEngine::GetEntityScriptInstance(entity->GetUUID());
								if (instance == nullptr)
								{
									ScriptEngine::CreateEntity(*entity.get());
									ScriptEngine::AwakeEntity(*entity.get());
								}
								else
									instance->SetEnabled(sc.enabled);
								ScriptEngine::OnEnableEntity(*entity.get());
								ScriptEngine::StartEntity(*entity.get());
							}
						}
					}
					else
					{
						if (entity->HasComponent<ScriptComponent>())
						{
							auto& sc = entity->GetComponent<ScriptComponent>();
							if (!sc.ClassName.empty())
							{
								auto instance = ScriptEngine::GetEntityScriptInstance(entity->GetUUID());
								if (instance == nullptr)
								{
									ScriptEngine::CreateEntity(*entity.get());
									ScriptEngine::AwakeEntity(*entity.get());
								}
								else
									instance->SetEnabled(sc.enabled);
								ScriptEngine::OnDisableEntity(*entity.get());
							}
						}
					}
				}
			}

			ImGui::SameLine();

			auto& tag = entity->GetComponent<TagComponent>().Tag;
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));

			ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

			// 重命名
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer), flags))
			{
				Entity* entityParent = entity->GetEntityParent();
				if(entityParent == nullptr)
					tag = Scene::NewName(entity->GetScene()->GetEntityList(), buffer);
				else
					tag = Scene::NewName(entityParent->GetEntityChildren(), buffer);
			}

			// PrefabList
			if (!Prefab::GetPrefabPath(entity).empty())
			{
				if (m_Context->GetName() != "Prefab")
				{
					if (ImGui::BeginCombo("Prefab", Prefab::GetPrefabPath(entity).c_str()))
					{
						try {
							DrawPrefabCombo(Project::GetAssetDirectory(), entity, stopDraw);
						}
						catch (const std::filesystem::filesystem_error& e) {
							VOL_ERROR(e.what());
						}

						ImGui::EndCombo();
					}

					ImGui::SameLine();

					// 重置Entity
					if (ImGui::Button("Reset"))
					{
						Ref<Entity> prefabEntity = Prefab::Get(Prefab::GetPrefabPath(entity));
						Prefab::ReloadPrefabTarget(prefabEntity, entity->GetScene(), entity->GetUUID());
					}
					ImGui::SameLine();

					// 断开Prefab和Entity联系
					if (ImGui::Button("Unpack"))
					{
						Prefab::RemovePrefabTarget(entity);
					}
				}
				else
				{
					if (ImGui::Button("SavePrefab"))
					{
						// 保存Prefab
						SceneSerializer serializer(m_Context);
						serializer.SerializePrefab(Project::GetAssetFileSystemPath(Prefab::GetPrefabPath(entity)).string(), *entity.get());
						
						// 将链接Prefab的所有Entity重新复制一遍
						Prefab::ReloadPrefabTarget(entity, m_Context.get());
					}
				}
			}

		}

		if (stopDraw)
			return;

		DrawComponent<TransformComponent>("Transform", entity, [](TransformComponent& component)
			{
				DrawVec3Control("Translation", component.Translation);

				glm::vec3 rotation = glm::degrees(component.Rotation);
				DrawVec3Control("Rotation", rotation);
				component.Rotation = glm::radians(rotation);
				//ImGui::DragFloat3("Position", glm::value_ptr(tc.Translation), 0.1f);

				DrawVec3Control("Scale", component.Scale, 1.0f);
			});

		DrawComponent<LightComponent>("Light", entity, [](LightComponent& component)
			{

				ImGui::Checkbox("##LightEnable", &component.enabled);

				ImGui::SameLine();

				const char* lightTypeStrings[] = { "DirectionalLight", "PointLight", "SpotLight"};
				const char* currentLightTypeString = lightTypeStrings[(int)component.Type];
				if (ImGui::BeginCombo("LightType", currentLightTypeString))
				{
					for (int i = 0; i < 3; i++)
					{
						bool isSelected = currentLightTypeString == lightTypeStrings[i];
						if (ImGui::Selectable(lightTypeStrings[i], isSelected))
						{
							if (!isSelected)
							{
								currentLightTypeString = lightTypeStrings[i];
								component.Type = (LightComponent::LightType)i;
							}
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

		DrawComponent<CameraComponent>("Camera", entity, [](CameraComponent& component)
			{
				ImGui::Checkbox("Enable", &component.enabled);


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
							if (!isSelected)
							{
								currentProjectionTypeString = projectionTypeStrings[i];
								camera.SetProjectionType((SceneCamera::ProjectionType)i);
							}
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

		DrawComponent<ScriptComponent>("Script", entity, [entity, scene = m_Context](ScriptComponent& component) mutable
			{
				Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity->GetUUID());
				if (ImGui::Checkbox("##ScriptEnabled", &component.enabled))
				{
					if (!component.ClassName.empty())
					{
						if (component.enabled)
						{
							if (scriptInstance == nullptr)
							{
								ScriptEngine::CreateEntity(*entity.get());
								ScriptEngine::AwakeEntity(*entity.get());
							}
							else
								scriptInstance->SetEnabled(component.enabled);

							ScriptEngine::OnEnableEntity(*entity.get());
							ScriptEngine::StartEntity(*entity.get());
						}
						else
						{
							if (scriptInstance == nullptr)
							{
								ScriptEngine::CreateEntity(*entity.get());
								ScriptEngine::AwakeEntity(*entity.get());
							}
							else
								scriptInstance->SetEnabled(component.enabled);

							ScriptEngine::OnDisableEntity(*entity.get());
						}
					}
				}

				ImGui::SameLine();

				bool scriptClassExists = ScriptEngine::ClassExists(component.ClassName);

				// 如果mono类不存在则红框
				//UI::ScopedStyleColor textColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f), !scriptClassExists);

				ImGui::SameLine();


				if (ImGui::BeginCombo("##ScriptClassName", component.ClassName.c_str()))
				{
					for (auto& [className, scriptClass] : ScriptEngine::GetClasses())
					{
						if (!scriptClass->IsCore())
						{
							const bool isSelected = (className == component.ClassName);
							if (ImGui::Selectable(className.c_str(), isSelected))
							{
								if (!isSelected)
									component.ClassName = className;
							}

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				// Fields
				bool sceneRunning = scene->IsRunning();
				if (sceneRunning)
				{
					//Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity->GetUUID());
					// 运行状态下将mono类获取的字段实时返回editor
					if (scriptInstance)
					{
						const auto& fields = scriptInstance->GetScriptClass()->GetFields();
						for (const auto& [name, field] : fields)
						{
							ImGui::PushID(&name);

							ImGui::Columns(2);
							ImGui::SetColumnWidth(0, 150.0f);
							ImGui::Text(name.c_str());
							ImGui::NextColumn();

							if (field.Type == ScriptFieldType::ULong)
							{
								uint64_t data = scriptInstance->GetFieldValue<uint64_t>(name);
								if (name == "ID")
								{
									ImGui::Text(std::to_string(data).c_str());
								}
								else
								{
									if (ImGui::InputText("##ScriptFieldName", (char*)&data, sizeof(data), ImGuiInputTextFlags_EnterReturnsTrue))
									{
										scriptInstance->SetFieldValue(name, data);
									}
								}
							}

							if (field.Type == ScriptFieldType::Float)
							{
								float data = scriptInstance->GetFieldValue<float>(name);
								if (ImGui::DragFloat("##ScriptFieldName", &data, 0.1f))
								{
									scriptInstance->SetFieldValue(name, data);
								}
							}
							if (field.Type == ScriptFieldType::Entity || field.Type == ScriptFieldType::GameObject || field.Type == ScriptFieldType::Behaviour || field.Type == ScriptFieldType::Component || field.Type == ScriptFieldType::Transform || field.Type == ScriptFieldType::MonoBehaviour)
							{
								uint64_t data = scriptInstance->GetFieldValue<uint64_t>(name);

								auto& entityIDMap = scene->GetEntityIDMap();
								Ref<Entity> targetEntity;

								if (data != 0)
								{
									if (entityIDMap.find(data) != entityIDMap.end())
										targetEntity = entityIDMap[data];

									// 如果当前scene没有该entity，在PrefabScene中找
									if (targetEntity == nullptr)
									{
										auto& prefabEntityIDMap = Prefab::GetScene()->GetEntityIDMap();
										if (prefabEntityIDMap.find(data) != prefabEntityIDMap.end())
											targetEntity = prefabEntityIDMap[data];
									}
								}

								if (ImGui::BeginCombo("##ScriptFieldName", targetEntity == nullptr ? "" : targetEntity->GetName().c_str()))
								{
									for (auto& [ID, entityNode] : entityIDMap)
									{
										const bool isSelected = (entityNode->GetUUID() == (targetEntity == nullptr ? UUID() : targetEntity->GetUUID()));
										if (ImGui::Selectable(entityNode->GetName().c_str(), isSelected))
										{
											if (!isSelected)
											{
												uint64_t id = entityNode->GetUUID();
												scriptInstance->SetFieldValue(name, id);
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
										const UUID* entityID = (const UUID*)payload->Data;
										scriptInstance->SetFieldValue(name, *entityID);
									}

									if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
									{
										const wchar_t* path = (const wchar_t*)payload->Data;
										std::filesystem::path prefabPath(path);

										if (prefabPath.extension() == ".prefab")
										{
											auto& prefabScene = Prefab::GetScene();
											Ref<Entity> targetEntity = Prefab::Get(Project::GetRelativeAssetDirectory(prefabPath).string());
											if (targetEntity == nullptr)
											{
												targetEntity = Prefab::Load(prefabPath);
											}
											scriptInstance->SetFieldValue(name, targetEntity->GetUUID());
										}
									}
									ImGui::EndDragDropTarget();
								}
							}
							ImGui::SameLine();
							ImGui::Text(("(" + std::string(Utils::ScriptFieldTypeToString(field.Type)) + ")").c_str());
							ImGui::EndColumns();

							ImGui::PopID();
						}
					}
				}
				else
				{
					// Editor模式下，从mono类中获取字段数据做拖动条，将拖动条数据注入EntityScriptFields
					// 注：mono类实例化会从EntityScriptFields读取字段数据并注入mono类实例，
					//   即读取的字段数据会应用到所有的相同entity子类脚本实例的字段
					// TODO：将entity的mono类脚本实例与EntityScriptFields对应上
					
					// 脚本类存在，渲染脚本类字段
					if (scriptClassExists)
					{
						Ref<ScriptClass> entityClass = ScriptEngine::GetClass(component.ClassName);
						// 获取mono类中的字段field
						const auto& fields = entityClass->GetFields();

						// 获取字段field的map
						auto& entityFields = ScriptEngine::GetScriptFieldMap(*entity.get());
						for (const auto& [name, field] : fields)
						{
							ImGui::PushID(&name);

							ImGui::Columns(2);
							ImGui::SetColumnWidth(0, 150.0f);
							ImGui::Text(name.c_str());
							ImGui::NextColumn();

							// Field has been set in editor
							if (entityFields.find(name) != entityFields.end())
							{

								VOL_CORE_ASSERT(entityFields.find(name) != entityFields.end());
								ScriptFieldInstance& scriptField = entityFields.at(name);

								if (field.Type == ScriptFieldType::ULong)
								{
									uint64_t data = scriptField.GetValue<uint64_t>();
									if (name == "ID")
									{
										ImGui::Text(std::to_string(data).c_str());
									}
									else
									{
										if (ImGui::InputText("##ScriptFieldName", (char*)&data, sizeof(data), ImGuiInputTextFlags_EnterReturnsTrue))
										{
											scriptField.SetValue(data);
										}
									}
								}

								// Display control to set it maybe
								if (field.Type == ScriptFieldType::Float)
								{
									float data = scriptField.GetValue<float>();
									if (ImGui::DragFloat("##ScriptFieldName", &data, 0.1f))
									{
										scriptField.SetValue(data);
									}
								}

								if (field.Type == ScriptFieldType::Entity || field.Type == ScriptFieldType::GameObject || field.Type == ScriptFieldType::Behaviour || field.Type == ScriptFieldType::Component || field.Type == ScriptFieldType::Transform || field.Type == ScriptFieldType::MonoBehaviour)
								{
									uint64_t data = scriptField.GetValue<uint64_t>();
									auto& entityIDMap = scene->GetEntityIDMap();
									Ref<Entity> targetEntity;
									if (entityIDMap.find(data) != entityIDMap.end())
										targetEntity = entityIDMap[data];

									// 如果当前scene没有该entity，在PrefabScene中找
									if (targetEntity == nullptr)
									{
										auto& prefabEntityIDMap = Prefab::GetScene()->GetEntityIDMap();
										if (prefabEntityIDMap.find(data) != prefabEntityIDMap.end())
											targetEntity = prefabEntityIDMap[data];
									}

									if (ImGui::BeginCombo("##ScriptFieldName", targetEntity == nullptr ? "" : targetEntity->GetName().c_str()))
									{
										for (auto& [ID, entityNode] : entityIDMap)
										{
											const bool isSelected = (entityNode->GetUUID() == (targetEntity == nullptr ? UUID() : targetEntity->GetUUID()));
											if (ImGui::Selectable(entityNode->GetName().c_str(), isSelected))
											{
												if(!isSelected)
												    scriptField.SetValue((uint64_t)entityNode->GetUUID());
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
											scriptField.SetValue(*entityID);
										}

										if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
										{
											const wchar_t* path = (const wchar_t*)payload->Data;
											std::filesystem::path prefabPath(path);

											if (prefabPath.extension() == ".prefab")
											{
												auto& prefabScene = Prefab::GetScene();
												Ref<Entity> targetEntity = Prefab::Get(Project::GetRelativeAssetDirectory(prefabPath).string());
												if (targetEntity == nullptr)
												{
													targetEntity = Prefab::Load(prefabPath);
												}
												scriptField.SetValue(targetEntity->GetUUID());
											}
										}
										ImGui::EndDragDropTarget();
									}
								}
							}
							else
							{
								if (field.Type == ScriptFieldType::ULong)
								{
									uint64_t data = 0;
									if (name != "ID")
									{
										if (ImGui::InputText("##ScriptFieldName", (char*)&data, sizeof(data), ImGuiInputTextFlags_EnterReturnsTrue))
										{
											ScriptFieldInstance& fieldInstance = entityFields[name];
											fieldInstance.Field = field;
											fieldInstance.SetValue(data);
										}
									}
								}

								// 如果name字段不在ScriptFieldMap中,设置渲染空字段，修改后在ScriptFieldMap中创建name字段
								if (field.Type == ScriptFieldType::Float)
								{
									float data = 0.0f;
									if (ImGui::DragFloat("##ScriptFieldName", &data, 0.1f))
									{
										ScriptFieldInstance& fieldInstance = entityFields[name];
										fieldInstance.Field = field;
										fieldInstance.SetValue(data);
									}
								}
								if (field.Type == ScriptFieldType::Entity || field.Type == ScriptFieldType::GameObject || field.Type == ScriptFieldType::Behaviour || field.Type == ScriptFieldType::Component || field.Type == ScriptFieldType::Transform || field.Type == ScriptFieldType::MonoBehaviour)
								{
									auto& entityIDMap = scene->GetEntityIDMap();
									if (ImGui::BeginCombo("##ScriptFieldName", ""))
									{
										for (auto& [ID, entityNode] : entityIDMap)
										{
											if (ImGui::Selectable(entityNode->GetName().c_str(), false))
											{
												ScriptFieldInstance& fieldInstance = entityFields[name];
												fieldInstance.Field = field;
												fieldInstance.SetValue((uint64_t)entityNode->GetUUID());
											}
										}
										ImGui::EndCombo();
									}
									if (ImGui::BeginDragDropTarget())
									{
										if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_NODE"))
										{
											const UUID* entityID = (const UUID*)payload->Data;
											ScriptFieldInstance& fieldInstance = entityFields[name];
											fieldInstance.Field = field;
											fieldInstance.SetValue(*entityID);
										}

										if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
										{
											const wchar_t* path = (const wchar_t*)payload->Data;
											std::filesystem::path prefabPath(path);

											if (prefabPath.extension() == ".prefab")
											{
												auto& prefabScene = Prefab::GetScene();
												Ref<Entity> targetEntity = Prefab::Get(Project::GetRelativeAssetDirectory(prefabPath).string());
												if (targetEntity == nullptr)
												{
													targetEntity = Prefab::Load(prefabPath);
												}
												ScriptFieldInstance& fieldInstance = entityFields[name];
												fieldInstance.Field = field;
												fieldInstance.SetValue(targetEntity->GetUUID());
											}
										}
										ImGui::EndDragDropTarget();
									}
								}
							}

							ImGui::SameLine();
							ImGui::Text(("(" + std::string(Utils::ScriptFieldTypeToString(field.Type)) + ")").c_str());
							ImGui::EndColumns();
							ImGui::PopID();
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

						if(texturePath.extension() == ".png")
						    component.Texture = Texture2D::Create(texturePath.string());
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
			});

		DrawComponent<MeshComponent>("Mesh", entity, [entity](MeshComponent& component)
			{
				const char* items[] = { "None", "Quad", "Circle", "Line", "Plane", "Cube", "Sphere", "Cylinder", "Capsule", "Model"};
				int meshType = (int)component.meshType;
				if (ImGui::Combo("MeshType", &meshType, items, IM_ARRAYSIZE(items)))
				{
					component.SetMesh((MeshType)meshType, entity.get());
				}

				if (component.meshType != MeshType::None && component.meshType != MeshType::Model && component.mesh != nullptr)
				{
					ImGui::Text(("VertexSize: " + std::to_string(component.mesh->GetVertexSize())).c_str());

					auto flags = ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue;
					char buffer[32];
					auto& vertexBone = component.vertexBone;
					if (vertexBone.size())
					{
						Entity* entityTemp = entity.get();
						do
						{
							if (entityTemp->HasComponent<AnimationComponent>())
							{
								auto& ac = entityTemp->GetComponent<AnimationComponent>();
								auto& bones = ac.animation->GetBones();

								static int deleteIndex = -1;

								ImGui::PushItemWidth(80);
								for (uint32_t i = 0; i < vertexBone.size(); i++)
								{
									ImGui::PushItemWidth(80);
									ImGui::PushID(i);
									memset(buffer, 0, sizeof(buffer));
									sprintf_s(buffer, "%d", vertexBone[i].vertexIndex1);
									if (ImGui::InputTextWithHint("##vi1", "vi1:", buffer, sizeof(buffer), flags))
										vertexBone[i].vertexIndex1 = atoi(buffer);

									ImGui::SameLine();

									memset(buffer, 0, sizeof(buffer));
									sprintf_s(buffer, "%d", vertexBone[i].vertexIndex2);

									if (ImGui::InputTextWithHint("##vi2", "vi2:", buffer, sizeof(buffer), flags))
										vertexBone[i].vertexIndex2 = atoi(buffer);

									ImGui::SameLine();

									ImGui::PushItemWidth(200);
									if (bones.size())
									{
										auto* bone = ac.animation->FindBone(vertexBone[i].boneIndex);
										std::string boneName = bone == nullptr ? std::string() : bone->GetBoneName();
										if (ImGui::BeginCombo("##combo", boneName.c_str()))
										{
											for (uint32_t comboIndex = 0; comboIndex < bones.size(); comboIndex++)
											{
												const bool isSelected = (vertexBone[i].boneIndex == bones[comboIndex].GetBoneID());
												if (ImGui::Selectable(bones[comboIndex].GetBoneName().c_str(), isSelected))
												{
													if (!isSelected)
													    vertexBone[i].boneIndex = bones[comboIndex].GetBoneID();
												}

												if (isSelected)
													ImGui::SetItemDefaultFocus();

											}
											ImGui::EndCombo();
										}

										ImGui::SameLine();
										ImGui::PushItemWidth(80);
										memset(buffer, 0, sizeof(buffer));
										sprintf_s(buffer, "%.4f", vertexBone[i].weight);
										if (ImGui::InputText("weight", buffer, sizeof(buffer), flags))
											vertexBone[i].weight = std::stof(buffer);
									}
									else
										ImGui::Text("No Bone");
									ImGui::PopItemWidth();

									ImGui::SameLine();

									if (ImGui::Button("-"))
										deleteIndex = i;

									ImGui::PopID();
								}

								if (deleteIndex != -1)
								{
									vertexBone.erase(vertexBone.begin() + deleteIndex);
									deleteIndex = -1;
								}

								break;
							}
							else
								entityTemp = entityTemp->GetEntityParent();
						} while (entityTemp != nullptr);


						if (entityTemp == nullptr)
						{
							ImGui::Text("No Animation");
						}
					}
					if (ImGui::Button("+"))
					{
						MeshComponent::VertexBone vertexBone;
						component.vertexBone.push_back(vertexBone);
					}
				}

				if (component.meshType == MeshType::Model)
				{

					ImGui::PushItemWidth(200);
					auto& meshes = Mesh::GetMeshLibrary()->GetMeshes();
					if (ImGui::BeginCombo("##Meshes", component.modelPath.c_str()))
					{
						for (auto& mesh : meshes)
						{
							bool isSelected = component.modelPath == mesh.first;
							if (ImGui::Selectable(mesh.first.c_str(), isSelected))
							{
								if (!isSelected)
								{
									component.SetMesh(MeshType::Model, entity.get(), mesh.second->mesh);
									component.modelPath = mesh.first;
									if (entity->HasComponent<MeshRendererComponent>())
										if (mesh.second->textures.empty())
											entity->RemoveComponent<MeshRendererComponent>();
										else
											entity->GetComponent<MeshRendererComponent>().Textures = mesh.second->textures;
									else
										if (!mesh.second->textures.empty())
											entity->AddComponent<MeshRendererComponent>().Textures = mesh.second->textures;
								}
							}

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
					ImGui::PopItemWidth();

					if (ImGui::Button("Load"))
					{
						std::string filePath = FileDialogs::OpenFile("Model Mesh (*.mms)\0*.mms\0 ", entity->GetScene()->GetPath());

						if (!filePath.empty())
						{
							std::string relativePath = Project::GetRelativeAssetDirectory(filePath).string();
							Ref<MeshNode> meshNode = Mesh::GetMeshLibrary()->Load(relativePath);
							component.SetMesh(MeshType::Model, entity.get(), meshNode->mesh);
							component.modelPath = relativePath;
							if (entity->HasComponent<MeshRendererComponent>())
								if (meshNode->textures.empty())
									entity->RemoveComponent<MeshRendererComponent>();
								else
									entity->GetComponent<MeshRendererComponent>().Textures = meshNode->textures;
							else
								if (!meshNode->textures.empty())
									entity->AddComponent<MeshRendererComponent>().Textures = meshNode->textures;
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Save..."))
					{
						if (component.mesh != nullptr)
						{
							if (component.modelPath.empty())
							{
								std::string filePath = FileDialogs::SaveFile("Model Mesh (*.mms)\0*.mms\0 ", entity->GetScene()->GetPath());
								if (!filePath.empty())
								{
									Ref<MeshNode> meshNode = std::make_shared<MeshNode>();
									meshNode->name = Project::GetRelativeAssetDirectory(filePath).string();
									meshNode->mesh = component.mesh;
									if (entity->HasComponent<MeshRendererComponent>())
										meshNode->textures = entity->GetComponent<MeshRendererComponent>().Textures;
									MeshSerializer serializer(meshNode);
									serializer.Serialize(filePath);
									Mesh::GetMeshLibrary()->Add(meshNode);
									component.modelPath = meshNode->name;
								}
							}
							else
							{
								Ref<MeshNode> meshNode = std::make_shared<MeshNode>();
								meshNode->name = component.modelPath;
								meshNode->mesh = component.mesh;
								if (entity->HasComponent<MeshRendererComponent>())
									meshNode->textures = entity->GetComponent<MeshRendererComponent>().Textures;
								MeshSerializer serializer(meshNode);
								serializer.Serialize(Project::GetAssetFileSystemPath(meshNode->name));
							}
						}
					}
					if (component.mesh == nullptr)
					{
						if (ImGui::BeginItemTooltip())
						{
							ImGui::Text("Please load model.");
							ImGui::EndTooltip();
						}

					}
					ImGui::SameLine();
					if (ImGui::Button("Save as..."))
					{
						if (component.mesh != nullptr)
						{
							std::string filePath = FileDialogs::SaveFile("Model Mesh (*.mms)\0*.mms\0 ", entity->GetScene()->GetPath());

							if (!filePath.empty())
							{
								Ref<MeshNode> meshNode = std::make_shared<MeshNode>();
								meshNode->name = Project::GetRelativeAssetDirectory(filePath).string();
								meshNode->mesh = component.mesh;
								if (entity->HasComponent<MeshRendererComponent>())
									meshNode->textures = entity->GetComponent<MeshRendererComponent>().Textures;
								MeshSerializer serializer(meshNode);
								serializer.Serialize(filePath);
								Mesh::GetMeshLibrary()->Add(meshNode);
								component.modelPath = meshNode->name;
							}
						}
					}
					if (component.mesh == nullptr)
					{
						if (ImGui::BeginItemTooltip())
						{
							ImGui::Text("Please load model.");
							ImGui::EndTooltip();
						}
					}

					ImGui::SameLine();
					if (ImGui::Button("Drop"))
					{
						if (component.modelPath != std::string())
						{
							component.modelPath = std::string();
							component.mesh = nullptr;
							Mesh::GetMeshLibrary()->Remove(component.modelPath);
						}
					}
					if (component.modelPath == std::string())
					{
						if (ImGui::BeginItemTooltip())
						{
							ImGui::Text("Please load model.");
							ImGui::EndTooltip();
						}
					}

				}
			});

		DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [](MeshRendererComponent& component)
			{
				ImGui::Checkbox("##MeshRendererEnabled", &component.enabled);

				ImGui::SameLine();

				const char* items[] = { "Diffuse", "Specular", "Normal", "Height", "Roughness", "AO" };
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
						float aspect = (float)texture->GetWidth() / (float)texture->GetHeight();
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

							if (filePath.extension() == ".png")
							    texture = Texture2D::Create(filePath.string());
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();
					if (ImGui::Button("filp"))
					{
						if(!texture->GetPath().empty())
							texture = Texture2D::Create(Project::GetAssetFileSystemPath(texture->GetPath()).string(), !texture->GetFilp());
					}

					ImGui::SameLine();
					if (ImGui::Button("white"))
						component.SetTextureWhite(i);

					ImGui::SameLine();
					if (ImGui::Button("black"))
						component.SetTextureBlack(i);

					ImGui::PopID();
				}


				if(ImGui::Button("+"))
					component.AddTexture();

				ImGui::SameLine();

				if (ImGui::Button("-"))
					if (textures.size() > 0)
						component.DeleteTexture(textures.size() - 1);
			});

		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](CircleRendererComponent& component)
			{
				ImGui::Checkbox("##CircleRendererEnabled", &component.enabled);

				ImGui::SameLine();

				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));

				if (ImGui::Button("Texture", ImVec2(100.0f, 100.0f)))
					component.Texture = nullptr;
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path texturePath = path;
						if(texturePath.extension() == ".png")
						    component.Texture = Texture2D::Create(texturePath.string());
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
				ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
			});

		DrawComponent<AnimatorComponent>("Animator", entity, [](AnimatorComponent& component) 
			{
				ImGui::Checkbox("##AnimatorEnabled", &component.enabled);

				ImGui::SameLine();

			    if (ImGui::Button("Animator"))
					component.Reset();
				ImGui::SameLine();
				if (component.animator->GetPlay())
				{
					if (ImGui::Button("Pause"))
						component.animator->SetPlay(false);
				}
				else
				{
					if (ImGui::Button("Play"))
						component.animator->SetPlay(true);
				}
				ImGui::SameLine();
				if (ImGui::Button("Stop"))
				{
					component.animator->SetPlay(true);
					component.animator->SetCurrentTime(0.0f);
					component.animator->UpdateAnimation(0.0f);
					component.animator->SetPlay(false);
				}
			});

		DrawComponent<AnimationComponent>("Animation", entity, [](AnimationComponent& component)
			{
				ImGui::Checkbox("##AnimationEnabled", &component.enabled);

				ImGui::SameLine();

				if (ImGui::Button("Animation"))
				{
					component.animation = std::make_shared<Animation>();
				}
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path animationPath = path;

						if (animationPath.extension() == ".obj" || animationPath.extension() == ".fbx" || animationPath.extension() == ".dae")
						{
							Ref<Model> model = Model::Create(animationPath.string().c_str());
							component.LoadAnimation(animationPath.string(), model.get());
						}
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::Separator();
				static bool animationManager = false;
				if (ImGui::Button("AnimationManager"))
					animationManager = !animationManager;
				if (animationManager)
				{
					ImGui::PushItemWidth(200);
					auto& animations = Animation::GetAnimationLibrary()->GetAnimations();
					if (ImGui::BeginCombo("##Animations", component.animation->GetName().c_str()))
					{
						for (auto& animation : animations)
						{
							bool isSelected = component.animation->GetName() == animation.first;
							if (ImGui::Selectable(animation.first.c_str(), isSelected))
							{
								if (!isSelected)
								    component.animation = animation.second;
							}

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
					ImGui::PopItemWidth();


					if (ImGui::Button("Save"))
					{
						std::string filePath;
						if (component.animation->GetName().empty())
						{
							filePath = FileDialogs::SaveFile("Animation (*.anm)\0*.anm\0 ", Project::GetAssetDirectory().string());
							if (!filePath.empty())
							{
								AnimationSerializer serializer(component.animation);
								serializer.Serialize(filePath);
								// 删除原动画的记录
								Animation::GetAnimationLibrary()->Remove(component.animation->GetName());
								std::string animationName = Project::GetRelativeAssetDirectory(filePath).string();
								component.animation->SetPath(filePath);
								component.animation->SetName(animationName);
								// 如果目标动画已导入过，删除目标动画记录，重新设置添加记录
								if (Animation::GetAnimationLibrary()->Exists(component.animation->GetName()))
									Animation::GetAnimationLibrary()->Remove(component.animation->GetName());
								Animation::GetAnimationLibrary()->Add(component.animation);
							}
						}
						else
						{
							filePath = component.animation->GetPath();
							AnimationSerializer serializer(component.animation);
							serializer.Serialize(filePath);
						}

					}

					ImGui::SameLine();
					if (ImGui::Button("Save as..."))
					{
						std::string filePath = FileDialogs::SaveFile("Animation (*.anm)\0*.anm\0 ", Project::GetAssetDirectory().string());

						if (!filePath.empty())
						{
							AnimationSerializer serializer(component.animation);
							serializer.Serialize(filePath);

							Animation::GetAnimationLibrary()->Remove(component.animation->GetName());
							std::string animationName = Project::GetRelativeAssetDirectory(filePath).string();
							component.animation->SetPath(filePath);
							component.animation->SetName(animationName);
							if (Animation::GetAnimationLibrary()->Exists(component.animation->GetName()))
								Animation::GetAnimationLibrary()->Remove(component.animation->GetName());
							Animation::GetAnimationLibrary()->Add(component.animation);
						}
					}

					ImGui::SameLine();
					if (ImGui::Button("Load"))
					{
						std::string filePath = FileDialogs::OpenFile("Animation (*.anm)\0*.anm\0 ", Project::GetAssetDirectory().string());
						if (!filePath.empty())
						{
							std::string filePathTemp = Project::GetRelativeAssetDirectory(filePath).string();
							// 如果已读取动画，删除动画重新读取
							if (Animation::GetAnimationLibrary()->Exists(filePathTemp))
								Animation::GetAnimationLibrary()->Remove(filePathTemp);
							component.animation = Animation::GetAnimationLibrary()->LoadAnm(filePathTemp);
							component.animation->SetPath(filePath);
							component.animation->SetName(filePathTemp);
						}
					}

					ImGui::SameLine();
					if (ImGui::Button("Drop"))
					{
						Animation::GetAnimationLibrary()->Remove(component.animation->GetName());
						component.animation = std::make_shared<Animation>();
					}
				}

				ImGui::Separator();
				ImGui::DragFloat("Duration", &component.animation->GetDuration());
				ImGui::DragFloat("TicksPerSecond", &component.animation->GetTicksPerSecond(), 1.0f);

				ImGui::Separator();
				auto& node = component.animation->GetRootNode();
				DrawAssimpNode(node, component);

				if (component.newBoneNodeParent != nullptr)
				{
					ImGui::Begin("New Bone");
					{
						ImGui::InputInt("BoneID", &component.boneIDBuffer);
						char buffer[64];
						memset(buffer, 0, sizeof(buffer));
						strncpy_s(buffer, sizeof(buffer), component.boneNameBuffer.c_str(), sizeof(buffer));
						if (ImGui::InputText("BoneName", buffer, sizeof(buffer)))
							component.boneNameBuffer = buffer;
						bool boneIDExist = component.animation->FindBone(component.boneIDBuffer);
						bool boneNameExist = component.animation->FindBone(component.boneNameBuffer);

						if (ImGui::Button("Create Bone"))
						{
							if (!boneIDExist && !boneNameExist)
							{
								auto& boneMap = component.animation->GetBoneIDMap();
								AssimpNodeData child;
								child.transformation = glm::mat4(1.0f);
								child.parent = component.newBoneNodeParent;
								child.name = component.boneNameBuffer;
								auto& bones = component.animation->GetBones();

								// 添加AssimpNodeData
								component.newBoneNodeParent->childrenCount++;
								component.newBoneNodeParent->children.push_back(child);

								boneMap[child.name].id = component.boneIDBuffer;
								boneMap[child.name].offset = glm::mat4(1.0f);

								// 添加Bone
								bones.push_back(Bone(child.name, component.boneIDBuffer));

								component.boneIDBuffer = 0;
								component.boneNameBuffer = std::string();
								component.newBoneNodeParent = nullptr;
							}
						}
						if (boneIDExist || boneNameExist)
						{
							if (ImGui::BeginItemTooltip())
							{
								ImGui::Text("Bone already exist.");
								ImGui::EndTooltip();
							}

						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel"))
						{
							component.boneIDBuffer = 0;
							component.boneNameBuffer = std::string();
							component.newBoneNodeParent = nullptr;
						}
					}
					ImGui::End();
				}

				//ImGui::Separator();
				//for (auto& boneInfo : component.animation->GetBoneIDMap())
				//	ImGui::Text(boneInfo.first.c_str());

				ImGui::Separator();
				ImGuiSliderFlags flag = ImGuiActivateFlags_None;

				ImGui::SliderInt("Key", &component.key, 0, 10);
				if (ImGui::TreeNodeEx("Bones", ImGuiTreeNodeFlags_SpanAvailWidth, "Bones: "))
				{
				    auto& bones = component.animation->GetBones();
					for (uint32_t i = 0; i < bones.size(); i++)
					{
						ImGui::PushID(i);

						if (ImGui::TreeNodeEx((void*)i, ImGuiTreeNodeFlags_SpanAvailWidth, (bones[i].GetBoneName() + "[" + std::to_string(bones[i].GetBoneID()) + "]").c_str()))
						{
							//ImGui::Text(bones[i].GetBoneName().c_str());
							auto& positions = bones[i].GetPositions();
								auto& rotations = bones[i].GetRotations();
							auto& scales = bones[i].GetScales();
							int size = glm::min(bones[i].GetNumPosition(), bones[i].GetNumRotation());
							size = glm::min(size, bones[i].GetNumScale());
							if (component.key < size)
							{
								DrawVec3Control("Translation", positions[component.key].position);

								glm::vec3 rotation = glm::degrees(glm::eulerAngles(rotations[component.key].orientation)); // 弧度
								DrawVec3Control("Rotation", rotation);
								rotations[component.key].orientation = glm::quat(glm::radians(rotation));

								DrawVec3Control("Scale", scales[component.key].scale, 1.0f);

								float timeStamp = positions[component.key].timeStamp;
								if (ImGui::InputFloat("TimeStamp", &timeStamp))
								{
									positions[component.key].timeStamp = timeStamp;
									rotations[component.key].timeStamp = timeStamp;
									scales[component.key].timeStamp = timeStamp;
								}
							}

							if (ImGui::Button("+"))
							{
								bones[i].AddKeyPosition(glm::vec3(0.0f));
								bones[i].AddKeyRotation(glm::quat(glm::vec3(0.0f)));
								bones[i].AddKeyScale(glm::vec3(1.0f));
							}
							ImGui::SameLine();
							if (ImGui::Button("-"))
							{
								bones[i].RemoveKeyPosition(bones[i].GetNumPosition() - 1);
								bones[i].RemoveKeyRotation(bones[i].GetNumRotation() - 1);
								bones[i].RemoveKeyScale(bones[i].GetNumScale() - 1);
							}
							ImGui::SameLine();
							ImGui::Text(("KeySize: " + std::to_string(size)).c_str());

							ImGui::TreePop();
						}
						ImGui::PopID();
					}
					ImGui::TreePop();
				}

			});

		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component)
			{
				const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
				const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];
				if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
				{
					for (int i = 0; i < 3; i++)
					{
						bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
						if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
						{
							if (!isSelected)
							{
								currentBodyTypeString = bodyTypeStrings[i];
								component.Type = (Rigidbody2DComponent::BodyType)i;
							}
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

		DrawComponent<RigidbodyComponent>("Rigidbody", entity, [](RigidbodyComponent& component)
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
							if (!isSelected)
							{
								currentBodyTypeString = bodyTypeStrings[i];
								component.Type = (RigidbodyComponent::BodyType)i;
							}
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
			});

		DrawComponent<BoxColliderComponent>("Box Collider", entity, [](BoxColliderComponent& component)
			{
				ImGui::Checkbox("Show Collider", &component.showCollider);
				DrawVec3Control("Size", component.size);
				ImGui::DragFloat("Density", &component.material.density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("StaticFriction", &component.material.staticFriction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("DynamicFriction", &component.material.dynamicFriction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("bounciness", &component.material.bounciness, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution Threshold", &component.material.restitutionThreshold, 0.01f, 0.0f);
			});

		DrawComponent<SphereColliderComponent>("Sphere Collider", entity, [](SphereColliderComponent& component)
			{
				ImGui::Checkbox("Show Collider", &component.showCollider);
				ImGui::DragFloat("Radius", &component.radius);
				ImGui::DragFloat("Density", &component.material.density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("StaticFriction", &component.material.staticFriction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("DynamicFriction", &component.material.dynamicFriction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("bounciness", &component.material.bounciness, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution Threshold", &component.material.restitutionThreshold, 0.01f, 0.0f);
			});

		DrawComponent<SkyboxComponent>("Skybox", entity, [](SkyboxComponent& component)
			{
				ImGui::Checkbox("##SkyboxEnabled", &component.enabled);

				ImGui::SameLine();

				ImGui::Checkbox("Primary", &component.Primary);

				if (ImGui::Button("Skybox"))
				{
					component.texture = TextureCube::Create(TextureFormat::RGB16F, 512, 512);
				}
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path texturePath = path;

						if (texturePath.extension() == ".png" || texturePath.extension() == ".jpg")
						    component.texture = TextureCube::Create(texturePath.string());
					}
					ImGui::EndDragDropTarget();
				}

			});

		ImGui::Separator();

		ImGui::Button("LoadModel");
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path modelPath = path;;

				if (modelPath.extension() == ".obj" || modelPath.extension() == ".fbx" || modelPath.extension() == ".dae")
				{
					auto model = Model::Create(modelPath.string().c_str());
					ModelLoading(model->GetModelNodeRoot(), model, entity);

					if (!entity->HasComponent<AnimatorComponent>())
					{
						auto& ac = entity->AddComponent<AnimatorComponent>();
					}

					if (!entity->HasComponent<AnimationComponent>())
					{
						auto& ac = entity->AddComponent<AnimationComponent>();
						ac.animation = model->GetAnimation();
					}
					else
					{
						auto& ac = entity->GetComponent<AnimationComponent>();
						ac.animation = model->GetAnimation();
					}
				}
			}
			ImGui::EndDragDropTarget();
		}


		ImGui::Separator();

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
			DisplayAddComponentEntry<AnimatorComponent>("Animator");
			DisplayAddComponentEntry<AnimationComponent>("Animation");
			DisplayAddComponentEntry<Rigidbody2DComponent>("Rigidbody 2D");
			DisplayAddComponentEntry<BoxCollider2DComponent>("Box Collider 2D");
			DisplayAddComponentEntry<CircleCollider2DComponent>("Circle Collider 2D");
			DisplayAddComponentEntry<RigidbodyComponent>("Rigidbody");
			DisplayAddComponentEntry<BoxColliderComponent>("Box Collider");
			DisplayAddComponentEntry<SphereColliderComponent>("Sphere Collider");
			DisplayAddComponentEntry<SkyboxComponent>("Skybox");

			ImGui::EndPopup();
		}
		ImGui::PopItemWidth();

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