#include "SceneHierarchyPanel.h"

#include "glm/gtc/type_ptr.hpp"

namespace Volcano {

	namespace Utils
	{
		template<typename T, typename UIFunction>
		static void DrawComponent(const std::string& name, Ref<Entity> entity, UIFunction uiFunction)
		{
			const ImGuiTreeNodeFlags treeNodeFlags =
				ImGuiTreeNodeFlags_DefaultOpen
				| ImGuiTreeNodeFlags_Framed
				| ImGuiTreeNodeFlags_SpanAvailWidth
				| ImGuiTreeNodeFlags_AllowOverlap
				| ImGuiTreeNodeFlags_FramePadding;
			if (entity->HasComponent<T>())
			{
				auto& component = entity->GetComponent<T>();
				// 为了定位+按钮在最右边
				ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

				// 将后续所有UI元素的内部填充（内边距）临时设置为 4x4 像素
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				float lineHeight = ImGui::GetFontSize() + ImGui::GetCurrentContext()->Style.FramePadding.y * 2.0f;
				ImGui::Separator();
				// 绘制展开箭头
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

	}

	void SceneHierarchyPanel::DrawComponents(Ref<Entity> entity)
	{
		static bool stopDraw;
		stopDraw = false;

		DrawTagComponent(entity);

		if (stopDraw)
			return;

		Utils::DrawComponent<TransformComponent>("Transform", entity, [this](TransformComponent& component)
			{
				DrawVec3Control("Translation", component.translation);

				DrawVec3Control("Rotation", component.inspectorEulerHint, 0.0f, 100.0f, [&component]() {
					component.rotation = glm::quat(glm::radians(component.inspectorEulerHint));
					});

				DrawVec3Control("Scale", component.scale, 1.0f);
			});

		Utils::DrawComponent<CameraComponent>("Camera", entity, [this](CameraComponent& component) { DrawCameraComponent(component);});

		Utils::DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](CircleRendererComponent& component)
			{
				ImGui::Checkbox("Enabled", &component.enabled);

				ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
				ImGui::DragFloat("Thickness", &component.thickness, 0.025f, 0.0f, 1.0f);
				ImGui::DragFloat("Fade", &component.fade, 0.00025f, 0.0f, 1.0f);

			});

		Utils::DrawComponent<MeshComponent>("Mesh", entity, [entity](MeshComponent& component)
			{
				ImGui::Checkbox("Enabled", &component.enabled);

				const char* items[] = { "None", "Quad", "Circle", "Line", "Plane", "Cube", "Sphere", "Cylinder", "Capsule", "Cone", "Model" };
				int meshType = (int)component.meshType;
				// 下拉选择框（Combo Box / Dropdown）
				if (ImGui::Combo("MeshType", &meshType, items, IM_ARRAYSIZE(items)))
				{
					component.SetMeshType((MeshType)meshType);
				}

			});

		Utils::DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [this](MeshRendererComponent& component) { DrawMeshRendererComponent(component); });

		Utils::DrawComponent<LightComponent>("Light", entity, [this](LightComponent& component) { DrawLightComponent(component);});

		Utils::DrawComponent<SkyboxComponent>("Skybox", entity, [this](SkyboxComponent& component) { DrawSkyboxComponent(component); });

		Utils::DrawComponent<HDRComponent>("HDR", entity, [this](HDRComponent& component) { DrawHDRComponent(component); });

		Utils::DrawComponent<ParticleSystemComponent>("Particle System", entity, [this](ParticleSystemComponent& component) { DrawParticleSystemComponent(component); });

		Utils::DrawComponent<ScriptComponent>("Script", entity, [entity, this](ScriptComponent& component) mutable { DrawScriptComponent(entity, component); });

		Utils::DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component)
			{
				const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
				const char* currentBodyTypeString = bodyTypeStrings[(int)component.type];
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
								component.type = (Rigidbody2DComponent::BodyType)i;
							}
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				ImGui::Checkbox("Fixed Rotation", &component.fixedRotation);
			});

		Utils::DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
			{
				ImGui::Checkbox("Enabled", &component.enabled);

				ImGui::DragFloat2("Offset", glm::value_ptr(component.offset));
				ImGui::DragFloat2("Size", glm::value_ptr(component.size));
				ImGui::DragFloat("Density", &component.density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Friction", &component.friction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution", &component.restitution, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution Threshold", &component.restitutionThreshold, 0.01f, 0.0f);
			});

		Utils::DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component)
			{
				ImGui::Checkbox("Enabled", &component.enabled);

				ImGui::DragFloat2("Offset", glm::value_ptr(component.offset));
				ImGui::DragFloat("Radius", &component.radius, 0.01f);
				ImGui::DragFloat("Density", &component.density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Friction", &component.friction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution", &component.restitution, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution Threshold", &component.restitutionThreshold, 0.01f, 0.0f);
			});

		ImGui::Separator();

		ImGui::PushItemWidth(-1);
		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			DisplayAddComponentEntry<CameraComponent>("Camera");
			DisplayAddComponentEntry<CircleRendererComponent>("Circle Renderer");
			DisplayAddComponentEntry<MeshComponent>("Mesh");
			DisplayAddComponentEntry<MeshRendererComponent>("Mesh Renderer");
			DisplayAddComponentEntry<LightComponent>("Light");
			DisplayAddComponentEntry<SkyboxComponent>("Skybox");
			DisplayAddComponentEntry<HDRComponent>("HDR");
			DisplayAddComponentEntry<ParticleSystemComponent>("Particle System");
			DisplayAddComponentEntry<ScriptComponent>("Script");
			DisplayAddComponentEntry<Rigidbody2DComponent>("Rigidbody 2D");
			DisplayAddComponentEntry<BoxCollider2DComponent>("Box Collider 2D");
			DisplayAddComponentEntry<CircleCollider2DComponent>("Circle Collider 2D");

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