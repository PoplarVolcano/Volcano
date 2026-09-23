#include "SceneHierarchyPanel.h"

#include "glm/gtc/type_ptr.hpp"

namespace Volcano
{
	template<typename Func>
	void DrawItem(const char* tag, Ref<ParticleSystem> particleSystem, float columnWidth, Func&& func)
	{
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);

		bool flag = (tag != nullptr && tag[0] != '\0');
		if (flag)
		{
			ImGui::Text(tag);
			ImGui::NextColumn();
		}

		std::forward<Func>(func)(particleSystem);

		if (flag)
		{
			ImGui::EndColumns();
		}
	}

	void SceneHierarchyPanel::DrawParticleSystemComponent(ParticleSystemComponent& component)
	{

		float columnWidth = 125.0f;

		Ref<ParticleSystem> particleSystem = component.particleSystem;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;

		if (ImGui::TreeNodeEx("##ParticleSystem", flags, "Particle System"))
		{
			DrawItem("Duration", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				ImGui::DragFloat("##duration", &particleSystem->duration, 1.0f, 0.05f, 100000.0f);
				});

			DrawItem("Looping", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				ImGui::Checkbox("##looping", &particleSystem->looping);
				});

			DrawItem("Prewarm", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				ImGui::Checkbox("##prewarm", &particleSystem->prewarm);
				});

			DrawItem("Start Delay", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				if (particleSystem->startDelayType == 0)
				{
					if (ImGui::DragFloat("##startDelay", &particleSystem->startDelay1, 0.1f, 0.0f, FLT_MAX))
						particleSystem->startDelay2 = particleSystem->startDelay1;
				}
				else if (particleSystem->startDelayType == 1)
				{
					ImGui::DragFloat("##startDelay1", &particleSystem->startDelay1, 0.1f, 0.0f, FLT_MAX);
					ImGui::DragFloat("##startDelay2", &particleSystem->startDelay2, 0.1f, 0.0f, FLT_MAX);
				}
				});

			DrawItem("Start Lifetime", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				if (particleSystem->startLifetimeType == 0)
				{
					if (ImGui::DragFloat("##startLifetime", &particleSystem->startLifetime1, 0.1f, 0.0001f, FLT_MAX))
						particleSystem->startLifetime2 = particleSystem->startLifetime1;
				}
				else if (particleSystem->startLifetimeType == 2)
				{
					ImGui::DragFloat("##startLifetime1", &particleSystem->startLifetime1, 0.1f, 0.0001f, FLT_MAX);
					ImGui::DragFloat("##startLifetime2", &particleSystem->startLifetime2, 0.1f, 0.0001f, FLT_MAX);
				}
				});

			DrawItem("Start Speed", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				if (particleSystem->startSpeedType == 0)
				{
					if (ImGui::DragFloat("##startSpeed", &particleSystem->startSpeed1))
						particleSystem->startSpeed2 = particleSystem->startSpeed1;
				}
				else if (particleSystem->startSpeedType == 2)
				{
					ImGui::DragFloat("##startSpeed1", &particleSystem->startSpeed1);
					ImGui::DragFloat("##startSpeed2", &particleSystem->startSpeed2);
				}
				});

			DrawItem("3D Start Size", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				ImGui::Checkbox("##threeDStartSize", &particleSystem->useThreeDStartSize);
				});

			DrawItem("Start Size", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				if (particleSystem->startSizeType == 0)
				{
					if (ImGui::DragFloat("##startSize", &particleSystem->startSize1, 0.1f, 0.0f, FLT_MAX))
						particleSystem->startSize2 = particleSystem->startSize1;
				}
				else if (particleSystem->startSizeType == 2)
				{
					ImGui::DragFloat("##startSize1", &particleSystem->startSize1, 0.1f, 0.0f, FLT_MAX);
					ImGui::DragFloat("##startSize2", &particleSystem->startSize2, 0.1f, 0.0f, FLT_MAX);
				}
				});

			DrawItem("3D Start Rotation", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				ImGui::Checkbox("##threeDStartRotation", &particleSystem->useThreeDStartRotation);
				});

			DrawItem("Start Rotation", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				if (particleSystem->startSizeType == 0)
				{
					if (ImGui::DragFloat("##startRotation", &particleSystem->startRotation1))
						particleSystem->startRotation2 = particleSystem->startRotation1;
				}
				else if (particleSystem->startSizeType == 2)
				{
					ImGui::DragFloat("##startRotation1", &particleSystem->startRotation1);
					ImGui::DragFloat("##startRotation2", &particleSystem->startRotation2);
				}
				});

			DrawItem("Flip Rotation", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				ImGui::DragFloat("##flipRotation", &particleSystem->flipRotation, 0.1f, 0.0f, 1.0f);
				});

			DrawItem("Start Color", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				if (particleSystem->startColorType == 0)
				{
					if (ImGui::ColorEdit4("##startColor", glm::value_ptr(particleSystem->startColor1)))
						particleSystem->startColor2 = particleSystem->startColor1;
				}
				else if (particleSystem->startColorType == 2)
				{
					ImGui::ColorEdit4("##startColor1", glm::value_ptr(particleSystem->startColor1));
					ImGui::ColorEdit4("##startColor2", glm::value_ptr(particleSystem->startColor2));
				}
				});

			DrawItem("Simulation Space", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				const char* simulationSpace[] = { "Local", "World", "Custom" };
				VOL_ASSERT(particleSystem->simulationSpace < 3);
				if (ImGui::BeginCombo("##simulationSpace", simulationSpace[particleSystem->simulationSpace]))
				{
					for (int i = 0; i < 3; i++)
					{
						bool isSelected = particleSystem->simulationSpace == i;
						if (ImGui::Selectable(simulationSpace[i], isSelected))
						{
							if (!isSelected)
							{
								particleSystem->simulationSpace = i;
							}
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				});

			DrawItem("Simulation Speed", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				ImGui::DragFloat("##simulationSpeed", &particleSystem->simulationSpeed, 1.0f, 0.0f, 100.0f);
				});

			DrawItem("Play On Awake", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				ImGui::Checkbox("##playOnAwake", &particleSystem->playOnAwake);
				});

			DrawItem("Max Particles", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				ImGui::DragInt("##maxParticles", &particleSystem->maxParticles, 1, 0, INT_FAST16_MAX);
				});

			ImGui::TreePop();
		}

		ImGui::Separator();

		if (ImGui::TreeNodeEx("##Emission", flags, "Emission"))
		{
			DrawItem("Enable", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				ImGui::Checkbox("##enable", &particleSystem->emission.enabled);
				});

			DrawItem("Rate Over Time", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				if (particleSystem->emission.rateOverTimeType == 0)
				{
					if (ImGui::DragFloat("##rateOverTime", &particleSystem->emission.rateOverTime1))
						particleSystem->emission.rateOverTime2 = particleSystem->emission.rateOverTime1;
				}
				else if (particleSystem->emission.rateOverDistanceType == 2)
				{
					ImGui::DragFloat("##rateOverTime1", &particleSystem->emission.rateOverTime1);
					ImGui::DragFloat("##rateOverTime2", &particleSystem->emission.rateOverTime2);
				}
				});

			DrawItem("Rate Over Distance", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				if (particleSystem->emission.rateOverDistanceType == 0)
				{
					if (ImGui::DragFloat("##rateOverDistance", &particleSystem->emission.rateOverDistance1))
						particleSystem->emission.rateOverDistance2 = particleSystem->emission.rateOverDistance1;
				}
				else if (particleSystem->emission.rateOverDistanceType == 2)
				{
					ImGui::DragFloat("##rateOverDistance1", &particleSystem->emission.rateOverDistance1);
					ImGui::DragFloat("##rateOverDistance2", &particleSystem->emission.rateOverDistance2);
				}
				});

			ImGui::TreePop();
		}

		ImGui::Separator();

		if (ImGui::TreeNodeEx("##Shape", flags, "Shape"))
		{
			DrawItem("Enable", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				ImGui::Checkbox("##enable", &particleSystem->shape.enabled);
				});

			DrawItem("Shape", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				const char* shape[] = {
					"Sphere", "Hemisphere", "Cone", "Donut", "Box", "Mesh", "MeshRenderer",
					"SkinnedMeshRenderer", "Sprite", "SpriteRenderer", "Circle", "Edge", "Rectangle" };
				if (ImGui::BeginCombo("##shape", shape[(int)particleSystem->shape.shape]))
				{
					for (int i = 0; i < 13; i++)
					{
						bool isSelected = (int)particleSystem->shape.shape == i;
						if (ImGui::Selectable(shape[i], isSelected))
						{
							if (!isSelected)
							{
								particleSystem->shape.shape = (ParticleSystem_Shape::Shape)i;
							}
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				});
			
			switch (particleSystem->shape.shape)
			{
			case ParticleSystem_Shape::Shape::Sphere:
			{
				DrawItem("Angle", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
					ImGui::DragFloat("##angle", &particleSystem->shape.angle, 0.1f, 0.0f, 180.0f);
					});

				DrawItem("Radius", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
					ImGui::DragFloat("##radius", &particleSystem->shape.radius, 0.1f, 0.0001f, FLT_MAX);
					});

				DrawItem("RadiusThickness", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
					ImGui::DragFloat("##radiusThickness", &particleSystem->shape.radiusThickness, 0.01f, 0.0f, 1.0f);
					});

				DrawItem("Arc", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
					ImGui::DragFloat("##arc", &particleSystem->shape.arc, 0.1f, 0.0f, 360.0f);
					});

				break;
			}
			case Volcano::ParticleSystem_Shape::Shape::Hemisphere:
			{
				DrawItem("Angle", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
					ImGui::DragFloat("##angle", &particleSystem->shape.angle, 0.1f, 0.0f, 90.0f);
					});

				DrawItem("Radius", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
					ImGui::DragFloat("##radius", &particleSystem->shape.radius, 0.1f, 0.0001f, FLT_MAX);
					});

				DrawItem("RadiusThickness", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
					ImGui::DragFloat("##radiusThickness", &particleSystem->shape.radiusThickness, 0.01f, 0.0f, 1.0f);
					});

				DrawItem("Arc", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
					ImGui::DragFloat("##arc", &particleSystem->shape.arc, 0.1f, 0.0f, 360.0f);
					});

				break;
			}
			case ParticleSystem_Shape::Shape::Cone:
			{
				DrawItem("Angle", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
					ImGui::DragFloat("##angle", &particleSystem->shape.angle, 0.1f, 0.0f, 90.0f);
					});

				DrawItem("Radius", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
					ImGui::DragFloat("##radius", &particleSystem->shape.radius, 0.1f, 0.0001f, FLT_MAX);
					});

				DrawItem("RadiusThickness", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
					ImGui::DragFloat("##radiusThickness", &particleSystem->shape.radiusThickness, 0.01f, 0.0f, 1.0f);
					});

				DrawItem("Arc", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
					ImGui::DragFloat("##arc", &particleSystem->shape.arc, 0.1f, 0.0f, 360.0f);
					});

				DrawItem("Length", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
					ImGui::DragFloat("##length", &particleSystem->shape.length, 0.1f, 0.0f, FLT_MAX);
					});

				DrawItem("Emit From", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
					const char* emitFrom[] = { "Base", "Valume" };
					if (ImGui::BeginCombo("##emitFrom", emitFrom[(uint8_t)particleSystem->shape.emitFrom]))
					{
						for (uint8_t i = 0; i < 2; i++)
						{
							bool isSelected = (uint8_t)particleSystem->shape.emitFrom == i;
							if (ImGui::Selectable(emitFrom[i], isSelected))
							{
								if (!isSelected)
								{
									particleSystem->shape.emitFrom = (ParticleSystem_Shape::EmitFrom)i;
								}
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
					});

				break;
			}
			case ParticleSystem_Shape::Shape::Box:
			{
				DrawItem("Emit From", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
					const char* emitFrom[] = { "Base", "Valume", "Shell", "Edge"};
					if (ImGui::BeginCombo("##emitFrom", emitFrom[(uint8_t)particleSystem->shape.emitFrom]))
					{
						for (uint8_t i = 1; i < 4; i++)
						{
							bool isSelected = (uint8_t)particleSystem->shape.emitFrom == i;
							if (ImGui::Selectable(emitFrom[i], isSelected))
							{
								if (!isSelected)
								{
									particleSystem->shape.emitFrom = (ParticleSystem_Shape::EmitFrom)i;
								}
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
					});

				if (particleSystem->shape.emitFrom != ParticleSystem_Shape::EmitFrom::Valum)
				{

					DrawItem("", particleSystem, columnWidth, [this, columnWidth](Ref<ParticleSystem> particleSystem) {
						DrawVec3Control("Box Thickness", particleSystem->shape.boxThickness, 0.0f, columnWidth);
						});

				}

				break;
			}
			default:
				break;
			}

			DrawItem("Texture", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				ImGui::Button("Texture", ImVec2(100.0f, 100.0f));
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path texturePath(path);

						if (texturePath.extension() == ".png" || texturePath.extension() == ".jpg")
							particleSystem->shape.texture = texturePath.string();
					}
					ImGui::EndDragDropTarget();
				}
				});


			DrawVec3Control("Position", particleSystem->shape.position, 0.0f, columnWidth);

			glm::vec3 degreeEuler = glm::degrees(glm::eulerAngles(particleSystem->shape.rotation));
			DrawVec3Control("Rotation", degreeEuler, 0.0f, columnWidth, [&particleSystem, degreeEuler]() {
				particleSystem->shape.rotation = glm::quat(glm::radians(degreeEuler));
				});

			DrawVec3Control("Scale", particleSystem->shape.scale, 1.0f, columnWidth);

			ImGui::TreePop();
		}

		ImGui::Separator();

		if (ImGui::TreeNodeEx("##Renderer", flags, "Renderer"))
		{
			DrawItem("Enable", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				ImGui::Checkbox("##enable", &particleSystem->renderer.enabled);
				});

			DrawItem("Render Mode", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				
				const char* renderMode[] = {
					"Billboard",
					"StrectchedBillboard",
					"HorizontalBillboard",
					"VerticalBillboard",
					"Mesh",
					"None"
				};

				if (ImGui::BeginCombo("##RenderMode", renderMode[(int)particleSystem->renderer.renderMode]))
				{
					for (int i = 0; i < 6; i++)
					{
						bool isSelected = (int)particleSystem->renderer.renderMode == i;
						if (ImGui::Selectable(renderMode[i], isSelected))
						{
							if (!isSelected)
							{
								particleSystem->renderer.renderMode = (ParticleSystem_Renderer::RenderMode)i;
							}
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				});

			if (particleSystem->renderer.renderMode == ParticleSystem_Renderer::RenderMode::Mesh)
			{
				DrawItem("Meshes", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {

					const char* items[] = { "None", "Quad", "Circle", "Line", "Plane", "Cube", "Sphere", "Cylinder", "Capsule", "Model" };
					auto& meshes = particleSystem->renderer.meshes;

					for (uint32_t i = 0; i < meshes.size(); i++)
					{
						ImGui::PushID(i);
						int meshType = (int)meshes[i];
						if (ImGui::Combo("##Mesh", &meshType, items, IM_ARRAYSIZE(items)))
						{
							meshes[i] = meshType;
						}
						ImGui::SameLine();
						if (ImGui::Button("-"))
							particleSystem->renderer.RemoveMesh(i);

						ImGui::PopID();
					}
					if (ImGui::Button("+"))
						particleSystem->renderer.AddMesh();

					});
			}

			DrawItem("Material", particleSystem, columnWidth, [this](Ref<ParticleSystem> particleSystem) {

				auto* materialLibrary = m_Scene->GetMaterialLibrary();

				int materialLibraryIndex = materialLibrary->GetIndex(particleSystem->renderer.materialLibraryKey);

				if (ImGui::Combo("Material", &materialLibraryIndex, materialLibrary->GetMaterialNamesCStr().data(), (int)materialLibrary->GetMaterialNamesCStr().size()))
				{
					auto* key = materialLibrary->GetKey(materialLibraryIndex);
					if (key != nullptr)
					{
						particleSystem->renderer.materialLibraryKey = *key;
					}
					else
					{
						materialLibraryIndex = materialLibrary->GetIndex(particleSystem->renderer.materialLibraryKey);
					}
				}
				});


			ImGui::TreePop();
		}

		ImGui::Separator();

		if (ImGui::TreeNodeEx("##RotationOverLifetime", flags, "Rotation Over Lifetime"))
		{
			DrawItem("Enable", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				ImGui::Checkbox("##enable", &particleSystem->rotationOverLifetime.enabled);
				});

			DrawItem("Separate Axes", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
				ImGui::Checkbox("##separateAxes", &particleSystem->rotationOverLifetime.separateAxes);
				});


			if (!particleSystem->rotationOverLifetime.separateAxes)
			{
				DrawItem("Angular Velocity", particleSystem, columnWidth, [](Ref<ParticleSystem> particleSystem) {
					if (ImGui::DragFloat("##angularVelocity", &particleSystem->rotationOverLifetime.angularVelocity1, 1.0f))
					{
						if (particleSystem->rotationOverLifetime.angularVelocity1 > particleSystem->rotationOverLifetime.angularVelocity2)
						{
							particleSystem->rotationOverLifetime.angularVelocity2 = particleSystem->rotationOverLifetime.angularVelocity1;
						}
					}
					});
			}
			else
			{
				DrawItem("", particleSystem, columnWidth, [this, columnWidth](Ref<ParticleSystem> particleSystem) {
					DrawVec3Control("Angular Velocity", particleSystem->rotationOverLifetime.angularVelocity3D1, 0.0f, columnWidth, [particleSystem](){
						if (particleSystem->rotationOverLifetime.angularVelocity3D1.x > particleSystem->rotationOverLifetime.angularVelocity3D2.x ||
							particleSystem->rotationOverLifetime.angularVelocity3D1.y > particleSystem->rotationOverLifetime.angularVelocity3D2.y ||
							particleSystem->rotationOverLifetime.angularVelocity3D1.z > particleSystem->rotationOverLifetime.angularVelocity3D2.z)
						{
							particleSystem->rotationOverLifetime.angularVelocity3D2 = particleSystem->rotationOverLifetime.angularVelocity3D1;
						}
						});
					});
			}

			ImGui::TreePop();
		}

	}
}