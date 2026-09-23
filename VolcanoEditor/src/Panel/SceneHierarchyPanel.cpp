#include "SceneHierarchyPanel.h"

namespace Volcano {

	SceneHierarchyPanel::SceneHierarchyPanel(Ref<Scene>& scene)
	{
		SetScene(scene);
	}

	void SceneHierarchyPanel::SetScene(Ref<Scene>& scene)
	{
		m_Scene = scene;
		if (m_SelectionContext)
		{
			UUID SelectedEntityID = m_SelectionContext->GetUUID();
			Ref<Entity> target = m_Scene->GetEntityByUUID(SelectedEntityID);
			if (target)
				m_SelectionContext = target;
			else
				m_SelectionContext = {};
		}
		else
			m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		// 场景层级
		ImGui::Begin("Scene Hierarchy");

		if (m_Scene)
		{
			// BUG记录：窗口菜单放在节点菜单后面会覆盖判定，右键只会出现窗口菜单
			// Right-click on blank space
			if (ImGui::BeginPopupContextWindow(0, 1))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
					m_Scene->CreateEntity("Empty Entity");
				ImGui::EndPopup();
			}

			auto& entityList = m_Scene->GetEntityList();
			for (int i = 0; i < entityList.size(); i++)
			{
				ImGui::PushID(i);
				float availWidth = ImGui::GetContentRegionAvail().x;
				float hitboxHeight = ImGui::GetTextLineHeight() * 0.2f;   // 热区高度，可按需调
				ImGui::InvisibleButton("##separator", ImVec2(availWidth, hitboxHeight));
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_NODE"))
					{
						const UUID* entityID = (const UUID*)payload->Data;
						// 默认拖拽是本场景内部行为，如果目标实体不在本场景内，则无事发生
						auto& entityIDMap = m_Scene->GetEntityIDMap();
						auto idIterator = entityIDMap.find(*entityID);
						if (idIterator != entityIDMap.end())
						{
							size_t oldSize = entityList.size();
							m_Scene->MoveEntity(idIterator->second, i);
						}
					}

					ImGui::EndDragDropTarget();
				}

				DrawEntityNode(entityList[i]);
				ImGui::PopID();
			}

			// 设置一个空节点，用于将实体移动到Scene层级下
			// 若是被点击标记为选中状态|有下一级
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			ImGui::TreeNodeEx((void*)(-1), flags, "");

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_NODE"))
				{
					const UUID* entityID = (const UUID*)payload->Data;
					// 默认拖拽是本场景内部行为，如果目标实体不在本场景内，则无事发生
					auto& entityIDMap = m_Scene->GetEntityIDMap();
					auto it = entityIDMap.find(*entityID);
					if (it != entityIDMap.end())
						m_Scene->MoveEntity(it->second);
				}

				ImGui::EndDragDropTarget();
			}

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				m_SelectionContext = {};
		}

		ImGui::End();

		ImGui::Begin("Properties");
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
		auto& tag = entity->GetComponent<TagComponent>().tag;
		// 若是被点击标记为选中状态|有下一级
		ImGuiTreeNodeFlags flags = (m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = false;

		if (entity->GetEntityChildrenList().empty())
		{
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			ImGui::TreeNodeEx((void*)entity.get(), flags, tag.c_str());
		}
		else
		{
			flags |= ImGuiTreeNodeFlags_OpenOnArrow;
			opened = ImGui::TreeNodeEx((void*)entity.get(), flags, tag.c_str());
		}

		if (ImGui::BeginDragDropSource())
		{
			// 设置数据源
			UUID entityID = entity->GetUUID();
			ImGui::SetDragDropPayload("SCENE_HIERARCHY_NODE", &entityID, sizeof(entityID));
			ImGui::EndDragDropSource();
		}

		// 如果拖拽实体到本节点，将该实体变成本节点的子节点
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_NODE"))
			{
				const UUID* entityID = (const UUID*)payload->Data;
				// 默认拖拽是本场景内部行为，如果目标实体不在本场景内，则无事发生
				auto& entityIDMap = m_Scene->GetEntityIDMap();
				auto it = entityIDMap.find(*entityID);
				if (it != entityIDMap.end())
				    Scene::MoveEntityToEntity(it->second, entity.get());
			}

			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered())//ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		// 如果当前选中实体有粒子系统组件，则显示粒子系统小窗口
		if (m_SelectionContext == entity && entity->HasComponent<ParticleSystemComponent>() && entity->GetComponent<ParticleSystemComponent>().enabled)
		{
			auto& particleSystem = entity->GetComponent<ParticleSystemComponent>().particleSystem;
			ImGui::Begin("Particles");

			if (particleSystem->isPlaying)
			{
				if (ImGui::Button("Pause"))
				{
					particleSystem->Pause();
				}
			}
			else
			{
				if (ImGui::Button("Play"))
				{
					particleSystem->Play();
				}
			}
			ImGui::SameLine();

			if (ImGui::Button("Restart"))
			{
				particleSystem->Restart();
			}

			ImGui::SameLine();

			if (ImGui::Button("Stop"))
			{
				particleSystem->Stop();
			}


			float columnWidth = 175.0f;

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, columnWidth);
			ImGui::Text("Playback Speed");
			ImGui::NextColumn();
			ImGui::InputFloat("##playbackSpeed", &particleSystem->playbackSpeed);
			ImGui::EndColumns();

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, columnWidth);
			ImGui::Text("Playback Time");
			ImGui::NextColumn();
			ImGui::InputFloat("##playbackTime", &particleSystem->playbackTime);
			ImGui::EndColumns();

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, columnWidth);
			ImGui::Text("Particles");
			ImGui::NextColumn();
			ImGui::Text(std::to_string(particleSystem->GetParticleCount()).c_str());
			ImGui::EndColumns();

			ImGui::End();
		}


		bool entityDeleted = false;
		// 本节点右键菜单
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Create Empty Entity"))
			{
				m_Scene->CreateEntity("Empty Entity", entity.get());
			}
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;
			ImGui::EndPopup();
		}

		if (opened)
		{
			auto& entityChildren = entity->GetEntityChildrenList();
			for (int i = 0; i < entityChildren.size(); i++)
			{
				ImGui::PushID(i);
				auto& entityChild = entityChildren[i];

				float availWidth = ImGui::GetContentRegionAvail().x;
				float hitboxHeight = ImGui::GetTextLineHeight() * 0.2f;   // 热区高度，可按需调
				ImGui::InvisibleButton("##separator", ImVec2(availWidth, hitboxHeight));
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_NODE"))
					{
						const UUID* entityID = (const UUID*)payload->Data;
						// 默认拖拽是本场景内部行为，如果目标实体不在本场景内，则无事发生
						auto& entityIDMap = m_Scene->GetEntityIDMap();
						auto idIterator = entityIDMap.find(*entityID);
						if (idIterator != entityIDMap.end())
						{
							Scene::MoveEntityToEntity(idIterator->second, entity.get(), i);
						}
					}

					ImGui::EndDragDropTarget();
				}

				DrawEntityNode(entityChild);
				ImGui::PopID();
			}
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			if (m_SelectionContext == entity)
				m_SelectionContext = {};

			m_Scene->DestroyEntity(entity);
		}

	}

}